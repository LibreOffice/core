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


#include <svx/svdmrkv.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoedge.hxx>
#include "svdglob.hxx"
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
#include <sdr/overlay/overlayrollingrectangle.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdograf.hxx>

#include <editeng/editdata.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

using namespace com::sun::star;

// Migrate Marking of Objects, Points and GluePoints

class ImplMarkingOverlay
{
    // The OverlayObjects
    sdr::overlay::OverlayObjectList               maObjects;

    // The remembered second position in logical coordinates
    basegfx::B2DPoint                               maSecondPosition;

    // bitfield
    // A flag to remember if the action is for unmarking.
    bool                                            mbUnmarking : 1;

public:
    ImplMarkingOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos, bool bUnmarking = false);
    ~ImplMarkingOverlay();

    void SetSecondPosition(const basegfx::B2DPoint& rNewPosition);
    bool IsUnmarking() const { return mbUnmarking; }
};

ImplMarkingOverlay::ImplMarkingOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos, bool bUnmarking)
:   maSecondPosition(rStartPos),
    mbUnmarking(bUnmarking)
{
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        rtl::Reference< sdr::overlay::OverlayManager > xTargetOverlay = pCandidate->GetOverlayManager();

        if (xTargetOverlay.is())
        {
            sdr::overlay::OverlayRollingRectangleStriped* pNew = new sdr::overlay::OverlayRollingRectangleStriped(
                rStartPos, rStartPos, false);
            xTargetOverlay->add(*pNew);
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
            sdr::overlay::OverlayRollingRectangleStriped& rCandidate = static_cast< sdr::overlay::OverlayRollingRectangleStriped&>(maObjects.getOverlayObject(a));
            rCandidate.setSecondPosition(rNewPosition);
        }

        // remember new position
        maSecondPosition = rNewPosition;
    }
}



// MarkView



void SdrMarkView::ImpClearVars()
{
    eDragMode=SDRDRAG_MOVE;
    bRefHdlShownOnly=false;
    eEditMode=SDREDITMODE_EDIT;
    eEditMode0=SDREDITMODE_EDIT;
    bDesignMode=false;
    pMarkedObj=NULL;
    pMarkedPV=NULL;
    bForceFrameHandles=false;
    bPlusHdlAlways=false;
    nFrameHandlesLimit=50;
    bInsPolyPoint=false;
    mnInsPointNum = 0L;
    bMarkedObjRectDirty=false;
    bMarkedPointsRectsDirty=false;
    mbMarkHandlesHidden = false;
    bMrkPntDirty=false;
    bMarkHdlWhenTextEdit=false;
    bMarkableObjCountDirty=false; // not yet implemented
    nMarkableObjCount=0;          // not yet implemented

    // Migrate selections
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

SdrMarkView::SdrMarkView(SdrModel* pModel1, OutputDevice* pOut)
:   SdrSnapView(pModel1,pOut),
    mpMarkObjOverlay(0L),
    mpMarkPointsOverlay(0L),
    mpMarkGluePointsOverlay(0L),
    maHdlList(this),
    mpSdrViewSelection(new sdr::ViewSelection())
{
    ImpClearVars();
    StartListening(*pModel1);
}

SdrMarkView::~SdrMarkView()
{
    // Migrate selections
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
    delete mpSdrViewSelection;
}

void SdrMarkView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
    if (pSdrHint)
    {
        SdrHintKind eKind=pSdrHint->GetKind();

        if (eKind==HINT_OBJCHG || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED)
        {
            bMarkedObjRectDirty=true;
            bMarkedPointsRectsDirty=true;
        }
    }
    SdrSnapView::Notify(rBC,rHint);
}

void SdrMarkView::ModelHasChanged()
{
    SdrPaintView::ModelHasChanged();
    GetMarkedObjectListWriteAccess().SetNameDirty();
    bMarkedObjRectDirty=true;
    bMarkedPointsRectsDirty=true;
    // Example: Obj is selected and maMarkedObjectList is sorted.
    // In another View 2, the ObjOrder is changed (e. g. MovToTop())
    // Then we need to re-sort MarkList.
    GetMarkedObjectListWriteAccess().SetUnsorted();
    SortMarkedObjects();
    bMrkPntDirty=true;
    UndirtyMrkPnt();
    SdrView* pV=static_cast<SdrView*>(this);
    if (pV!=NULL && !pV->IsDragObj() && !pV->IsInsObjPoint()) {
        AdjustMarkHdl();
    }

    if (comphelper::LibreOfficeKit::isActive())
    {
        //TODO: Is MarkedObjRect valid at this point?
        Rectangle aSelection(GetMarkedObjRect());
        OString sSelection;
        if (aSelection.IsEmpty())
            sSelection = "EMPTY";
        else
        {
            // In case the map mode is in 100th MM, then need to convert the coordinates over to twips for LOK.
            if (pMarkedPV)
            {
                if (OutputDevice* pOutputDevice = pMarkedPV->GetView().GetFirstOutputDevice())
                {
                    if (pOutputDevice->GetMapMode().GetMapUnit() == MAP_100TH_MM)
                        aSelection = OutputDevice::LogicToLogic(aSelection, MAP_100TH_MM, MAP_TWIP);
                }
            }

            sSelection = aSelection.toString();
        }

        GetModel()->libreOfficeKitCallback(LOK_CALLBACK_INVALIDATE_TILES, sSelection.getStr());
    }
}



bool SdrMarkView::IsAction() const
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



void SdrMarkView::ClearPageView()
{
    UnmarkAllObj();
    SdrSnapView::ClearPageView();
}

void SdrMarkView::HideSdrPage()
{
    bool bMrkChg(false);

    SdrPageView* pPageView = GetSdrPageView();
    if (pPageView)
    {
        // break all creation actions when hiding page (#75081#)
        BrkAction();

        // Discard all selections on this page
        bMrkChg = GetMarkedObjectListWriteAccess().DeletePageView(*pPageView);
    }

    SdrSnapView::HideSdrPage();

    if(bMrkChg)
    {
        MarkListHasChanged();
        AdjustMarkHdl();
    }
}



bool SdrMarkView::BegMarkObj(const Point& rPnt, bool bUnmark)
{
    BrkAction();

    DBG_ASSERT(0L == mpMarkObjOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkObjOverlay (!)");
    basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
    mpMarkObjOverlay = new ImplMarkingOverlay(*this, aStartPos, bUnmark);

    aDragStat.Reset(rPnt);
    aDragStat.NextPoint();
    aDragStat.SetMinMove(nMinMovLog);

    return true;
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

bool SdrMarkView::EndMarkObj()
{
    bool bRetval(false);

    if(IsMarkObj())
    {
        if(aDragStat.IsMinMoved())
        {
            Rectangle aRect(aDragStat.GetStart(), aDragStat.GetNow());
            aRect.Justify();
            MarkObj(aRect, mpMarkObjOverlay->IsUnmarking());
            bRetval = true;
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



bool SdrMarkView::BegMarkPoints(const Point& rPnt, bool bUnmark)
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

        return true;
    }

    return false;
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

bool SdrMarkView::EndMarkPoints()
{
    bool bRetval(false);

    if(IsMarkPoints())
    {
        if(aDragStat.IsMinMoved())
        {
            Rectangle aRect(aDragStat.GetStart(), aDragStat.GetNow());
            aRect.Justify();
            MarkPoints(aRect, mpMarkPointsOverlay->IsUnmarking());

            bRetval = true;
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



bool SdrMarkView::BegMarkGluePoints(const Point& rPnt, bool bUnmark)
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

        return true;
    }

    return false;
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

bool SdrMarkView::EndMarkGluePoints()
{
    bool bRetval(false);

    if(IsMarkGluePoints())
    {
        if(aDragStat.IsMinMoved())
        {
            Rectangle aRect(aDragStat.GetStart(),aDragStat.GetNow());
            aRect.Justify();
            MarkGluePoints(&aRect, mpMarkGluePointsOverlay->IsUnmarking());

            bRetval = true;
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

sal_uIntPtr SdrMarkView::GetMarkableObjCount() const
{
    sal_uIntPtr nCount=0;
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        SdrObjList* pOL=pPV->GetObjList();
        const size_t nObjAnz = pOL->GetObjCount();
        for (size_t nObjNum=0; nObjNum<nObjAnz; ++nObjNum) {
            SdrObject* pObj=pOL->GetObj(nObjNum);
            if (IsObjMarkable(pObj,pPV)) {
                nCount++;
            }
        }
    }
    return nCount;
}

bool SdrMarkView::HasMarkableObj() const
{
    SdrPageView* pPV = GetSdrPageView();
    if(pPV)
    {
        SdrObjList* pOL=pPV->GetObjList();
        const size_t nObjAnz = pOL->GetObjCount();
        for (size_t nObjNum = 0; nObjNum<nObjAnz; ++nObjNum) {
            SdrObject* pObj=pOL->GetObj(nObjNum);
            if (IsObjMarkable(pObj,pPV)) {
                return true;
            }
        }
    }
    return false;
}

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

bool SdrMarkView::ImpIsFrameHandles() const
{
    const size_t nMarkCount=GetMarkedObjectCount();
    bool bFrmHdl=nMarkCount>static_cast<size_t>(nFrameHandlesLimit) || bForceFrameHandles;
    bool bStdDrag=eDragMode==SDRDRAG_MOVE;
    if (nMarkCount==1 && bStdDrag && bFrmHdl)
    {
        const SdrObject* pObj=GetMarkedObjectByIndex(0);
        if (pObj->GetObjInventor()==SdrInventor)
        {
            sal_uInt16 nIdent=pObj->GetObjIdentifier();
            if (nIdent==OBJ_LINE || nIdent==OBJ_EDGE || nIdent==OBJ_CAPTION || nIdent==OBJ_MEASURE || nIdent==OBJ_CUSTOMSHAPE || nIdent==OBJ_TABLE )
            {
                bFrmHdl=false;
            }
        }
    }
    if (!bStdDrag && !bFrmHdl) {
        // all other drag modes only with FrameHandles
        bFrmHdl=true;
        if (eDragMode==SDRDRAG_ROTATE) {
            // when rotating, use ObjOwn drag, if there's at least 1 PolyObj
            for (size_t nMarkNum=0; nMarkNum<nMarkCount && bFrmHdl; ++nMarkNum) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrObject* pObj=pM->GetMarkedSdrObj();
                bFrmHdl=!pObj->IsPolyObj();
            }
        }
    }
    if (!bFrmHdl) {
        // FrameHandles, if at least 1 Obj can't do SpecialDrag
        for (size_t nMarkNum=0; nMarkNum<nMarkCount && !bFrmHdl; ++nMarkNum) {
            const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            const SdrObject* pObj=pM->GetMarkedSdrObj();
            bFrmHdl=!pObj->hasSpecialDrag();
        }
    }

    // no FrameHdl for crop
    if(bFrmHdl && SDRDRAG_CROP == eDragMode)
    {
        bFrmHdl = false;
    }

    return bFrmHdl;
}

void SdrMarkView::SetMarkHandles()
{
    // remember old focus handle values to search for it again
    const SdrHdl* pSaveOldFocusHdl = maHdlList.GetFocusHdl();
    bool bSaveOldFocus(false);
    sal_uInt32 nSavePolyNum(0L), nSavePointNum(0L);
    SdrHdlKind eSaveKind(HDL_MOVE);
    SdrObject* pSaveObj = NULL;

    if(pSaveOldFocusHdl
        && pSaveOldFocusHdl->GetObj()
        && pSaveOldFocusHdl->GetObj()->ISA(SdrPathObj)
        && (pSaveOldFocusHdl->GetKind() == HDL_POLY || pSaveOldFocusHdl->GetKind() == HDL_BWGT))
    {
        bSaveOldFocus = true;
        nSavePolyNum = pSaveOldFocusHdl->GetPolyNum();
        nSavePointNum = pSaveOldFocusHdl->GetPointNum();
        pSaveObj = pSaveOldFocusHdl->GetObj();
        eSaveKind = pSaveOldFocusHdl->GetKind();
    }

    // delete/clear all handles. This will always be done, even with areMarkHandlesHidden()
    maHdlList.Clear();
    maHdlList.SetRotateShear(eDragMode==SDRDRAG_ROTATE);
    maHdlList.SetDistortShear(eDragMode==SDRDRAG_SHEAR);
    pMarkedObj=NULL;
    pMarkedPV=NULL;

    // are handles enabled at all? Create only then
    if(!areMarkHandlesHidden())
    {
        const size_t nMarkCount=GetMarkedObjectCount();
        bool bStdDrag=eDragMode==SDRDRAG_MOVE;
        bool bSingleTextObjMark=false;

        if (nMarkCount==1)
        {
            pMarkedObj=GetMarkedObjectByIndex(0);
            bSingleTextObjMark =
                pMarkedObj &&
                pMarkedObj->ISA(SdrTextObj) &&
                static_cast<SdrTextObj*>(pMarkedObj)->IsTextFrame();
        }

        bool bFrmHdl=ImpIsFrameHandles();

        if (nMarkCount>0)
        {
            pMarkedPV=GetSdrPageViewOfMarkedByIndex(0);

            for (size_t nMarkNum=0; nMarkNum<nMarkCount && (pMarkedPV!=NULL || !bFrmHdl); ++nMarkNum)
            {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);

                if (pMarkedPV!=pM->GetPageView())
                {
                    pMarkedPV=NULL;
                }
            }
        }

        // apply calc offset to marked object rect
        // ( necessary for handles to be displayed in
        // correct position )
        Point aGridOff = GetGridOffset();

        // There can be multiple mark views, but we're only interested in the one that has a window associated.
        const bool bTiledRendering = GetModel()->isTiledRendering() && GetFirstOutputDevice() && GetFirstOutputDevice()->GetOutDevType() == OUTDEV_WINDOW;

        // check if text edit or ole is active and handles need to be suppressed. This may be the case
        // when a single object is selected
        // Using a strict return statement is okay here; no handles means *no* handles.
        if(pMarkedObj)
        {
            // formally #i33755#: If TextEdit is active the EditEngine will directly paint
            // to the window, so suppress Overlay and handles completely; a text frame for
            // the active text edit will be painted by the repaitnt mechanism in
            // SdrObjEditView::ImpPaintOutlinerView in this case. This needs to be reworked
            // in the future
            // Also formally #122142#: Pretty much the same for SdrCaptionObj's in calc.
            if(static_cast<SdrView*>(this)->IsTextEdit())
            {
                const SdrTextObj* pSdrTextObj = dynamic_cast< const SdrTextObj* >(pMarkedObj);

                if(pSdrTextObj && pSdrTextObj->IsInEditMode())
                {
                    if (bTiledRendering)
                        // Suppress handles -> empty graphic selection.
                        GetModel()->libreOfficeKitCallback(LOK_CALLBACK_GRAPHIC_SELECTION, "EMPTY");
                    return;
                }
            }

            // formally #i118524#: if inplace activated OLE is selected, suppress handles
            const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pMarkedObj);

            if(pSdrOle2Obj && (pSdrOle2Obj->isInplaceActive() || pSdrOle2Obj->isUiActive()))
            {
                return;
            }

            if (bTiledRendering && pMarkedObj->GetObjIdentifier() == OBJ_TABLE)
            {
                rtl::Reference<sdr::SelectionController> xController = static_cast<SdrView*>(this)->getSelectionController();
                if (xController.is() && xController->hasSelectedCells())
                {
                    // The table shape has selected cells, which provide text selection already -> no graphic selection.
                    GetModel()->libreOfficeKitCallback(LOK_CALLBACK_GRAPHIC_SELECTION, "EMPTY");
                    return;
                }
            }
        }

        Rectangle aRect(GetMarkedObjRect());

        if (bTiledRendering)
        {
            Rectangle aSelection(aRect);
            OString sSelection;
            if (aSelection.IsEmpty())
                sSelection = "EMPTY";
            else
            {
                // In case the map mode is in 100th MM, then need to convert the coordinates over to twips for LOK.
                if (pMarkedPV)
                {
                    if (OutputDevice* pOutputDevice = pMarkedPV->GetView().GetFirstOutputDevice())
                    {
                        if (pOutputDevice->GetMapMode().GetMapUnit() == MAP_100TH_MM)
                            aSelection = OutputDevice::LogicToLogic(aSelection, MAP_100TH_MM, MAP_TWIP);
                    }
                }

                sSelection = aSelection.toString();

                // hide the text selection too
                GetModel()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION, "");
            }
            GetModel()->libreOfficeKitCallback(LOK_CALLBACK_GRAPHIC_SELECTION, sSelection.getStr());
        }

        if (bFrmHdl)
        {
            if(!aRect.IsEmpty())
            { // otherwise nothing is found
                if( bSingleTextObjMark )
                {
                    const size_t nSiz0=maHdlList.GetHdlCount();
                    pMarkedObj->AddToHdlList(maHdlList);
                    const size_t nSiz1=maHdlList.GetHdlCount();
                    for (size_t i=nSiz0; i<nSiz1; ++i)
                    {
                        SdrHdl* pHdl=maHdlList.GetHdl(i);
                        pHdl->SetObj(pMarkedObj);
                        pHdl->SetPos( pHdl->GetPos() + aGridOff );
                        pHdl->SetPageView(pMarkedPV);
                        pHdl->SetObjHdlNum(sal_uInt16(i-nSiz0));
                    }
                }
                else
                {
                    bool bWdt0=aRect.Left()==aRect.Right();
                    bool bHgt0=aRect.Top()==aRect.Bottom();
                    if (bWdt0 && bHgt0)
                    {
                        maHdlList.AddHdl(new SdrHdl(aRect.TopLeft(),HDL_UPLFT));
                    }
                    else if (!bStdDrag && (bWdt0 || bHgt0))
                    {
                        maHdlList.AddHdl(new SdrHdl(aRect.TopLeft()    ,HDL_UPLFT));
                        maHdlList.AddHdl(new SdrHdl(aRect.BottomRight(),HDL_LWRGT));
                    }
                    else
                    {
                        if (!bWdt0 && !bHgt0) maHdlList.AddHdl(new SdrHdl(aRect.TopLeft()     ,HDL_UPLFT));
                        if (          !bHgt0) maHdlList.AddHdl(new SdrHdl(aRect.TopCenter()   ,HDL_UPPER));
                        if (!bWdt0 && !bHgt0) maHdlList.AddHdl(new SdrHdl(aRect.TopRight()    ,HDL_UPRGT));
                        if (!bWdt0          ) maHdlList.AddHdl(new SdrHdl(aRect.LeftCenter()  ,HDL_LEFT ));
                        if (!bWdt0          ) maHdlList.AddHdl(new SdrHdl(aRect.RightCenter() ,HDL_RIGHT));
                        if (!bWdt0 && !bHgt0) maHdlList.AddHdl(new SdrHdl(aRect.BottomLeft()  ,HDL_LWLFT));
                        if (          !bHgt0) maHdlList.AddHdl(new SdrHdl(aRect.BottomCenter(),HDL_LOWER));
                        if (!bWdt0 && !bHgt0) maHdlList.AddHdl(new SdrHdl(aRect.BottomRight() ,HDL_LWRGT));
                    }
                }
            }
        }
        else
        {
            bool bDone(false);

            // moved crop handling to non-frame part and the handle creation to SdrGrafObj
            if(1 == nMarkCount && pMarkedObj && SDRDRAG_CROP == eDragMode)
            {
                // Default addCropHandles from SdrObject does nothing. When pMarkedObj is SdrGrafObj, previous
                // behaviour occurs (code in svx/source/svdraw/svdograf.cxx). When pMarkedObj is SwVirtFlyDrawObj
                // writer takes the responsibility of adding handles (code in sw/source/core/draw/dflyobj.cxx)
                pMarkedObj->addCropHandles(maHdlList);
                bDone = true;
            }

            if(!bDone)
            {
                for (size_t nMarkNum=0; nMarkNum<nMarkCount; ++nMarkNum)
                {
                    const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                    SdrObject* pObj=pM->GetMarkedSdrObj();
                    SdrPageView* pPV=pM->GetPageView();
                    const size_t nSiz0=maHdlList.GetHdlCount();
                    pObj->AddToHdlList(maHdlList);
                    const size_t nSiz1=maHdlList.GetHdlCount();
                    bool bPoly=pObj->IsPolyObj();
                    const SdrUShortCont* pMrkPnts=pM->GetMarkedPoints();
                    for (size_t i=nSiz0; i<nSiz1; ++i)
                    {
                        SdrHdl* pHdl=maHdlList.GetHdl(i);
                        pHdl->SetPos( pHdl->GetPos() + aGridOff );
                        pHdl->SetObj(pObj);
                        pHdl->SetPageView(pPV);
                        pHdl->SetObjHdlNum(sal_uInt16(i-nSiz0));

                        if (bPoly)
                        {
                            bool bSelected=pMrkPnts!=NULL
                                      && pMrkPnts->find( sal_uInt16(i-nSiz0) ) != pMrkPnts->end();
                            pHdl->SetSelected(bSelected);
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
                                        pPlusHdl->SetPlusHdl(true);
                                        maHdlList.AddHdl(pPlusHdl);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // GluePoint handles
        for (size_t nMarkNum=0; nMarkNum<nMarkCount; ++nMarkNum)
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
                    for(SdrUShortCont::const_iterator it = pMrkGlue->begin(); it != pMrkGlue->end(); ++it)
                    {
                        sal_uInt16 nId=*it;
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
                            maHdlList.AddHdl(pGlueHdl);
                        }
                    }
                }
            }
        }

        // rotation point/axis of reflection
        AddDragModeHdl(eDragMode);

        // sort handles
        maHdlList.Sort();

        // add custom handles (used by other apps, e.g. AnchorPos)
        AddCustomHdl();

        // try to restore focus handle index from remembered values
        if(bSaveOldFocus)
        {
            for(size_t a = 0; a < maHdlList.GetHdlCount(); ++a)
            {
                SdrHdl* pCandidate = maHdlList.GetHdl(a);

                if(pCandidate->GetObj()
                    && pCandidate->GetObj() == pSaveObj
                    && pCandidate->GetKind() == eSaveKind
                    && pCandidate->GetPolyNum() == nSavePolyNum
                    && pCandidate->GetPointNum() == nSavePointNum)
                {
                    maHdlList.SetFocusHdl(pCandidate);
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
        ForceRefToMarked();
        SetMarkHandles();
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

            maHdlList.AddHdl(pHdl);

            break;
        }
        case SDRDRAG_MIRROR:
        {
            // add axis of reflection
            SdrHdl* pHdl3 = new SdrHdl(aRef2, HDL_REF2);
            SdrHdl* pHdl2 = new SdrHdl(aRef1, HDL_REF1);
            SdrHdl* pHdl1 = new SdrHdlLine(*pHdl2, *pHdl3, HDL_MIRX);

            pHdl1->SetObjHdlNum(1); // for sorting
            pHdl2->SetObjHdlNum(2); // for sorting
            pHdl3->SetObjHdlNum(3); // for sorting

            maHdlList.AddHdl(pHdl1); // line comes first, so it is the last in HitTest
            maHdlList.AddHdl(pHdl2);
            maHdlList.AddHdl(pHdl3);

            break;
        }
        case SDRDRAG_TRANSPARENCE:
        {
            // add interactive transparency handle
            const size_t nMarkCount = GetMarkedObjectCount();
            if(nMarkCount == 1)
            {
                SdrObject* pObj = GetMarkedObjectByIndex(0);
                SdrModel* pModel = GetModel();
                const SfxItemSet& rSet = pObj->GetMergedItemSet();

                if(SfxItemState::SET != rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, false))
                {
                    // add this item, it's not yet there
                    XFillFloatTransparenceItem aNewItem(
                        static_cast<const XFillFloatTransparenceItem&>(rSet.Get(XATTR_FILLFLOATTRANSPARENCE)));
                    XGradient aGrad = aNewItem.GetGradientValue();

                    aNewItem.SetEnabled(true);
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

                    SfxItemSet aNewSet(pModel->GetItemPool());
                    aNewSet.Put(aNewItem);
                    pObj->SetMergedItemSetAndBroadcast(aNewSet);
                }

                // set values and transform to vector set
                GradTransformer aGradTransformer;
                GradTransVector aGradTransVector;
                GradTransGradient aGradTransGradient;

                aGradTransGradient.aGradient = static_cast<const XFillFloatTransparenceItem&>(rSet.Get(XATTR_FILLFLOATTRANSPARENCE)).GetGradientValue();
                GradTransformer::GradToVec(aGradTransGradient, aGradTransVector, pObj);

                // build handles
                const Point aTmpPos1(basegfx::fround(aGradTransVector.maPositionA.getX()), basegfx::fround(aGradTransVector.maPositionA.getY()));
                const Point aTmpPos2(basegfx::fround(aGradTransVector.maPositionB.getX()), basegfx::fround(aGradTransVector.maPositionB.getY()));
                SdrHdlColor* pColHdl1 = new SdrHdlColor(aTmpPos1, aGradTransVector.aCol1, SDR_HANDLE_COLOR_SIZE_NORMAL, true);
                SdrHdlColor* pColHdl2 = new SdrHdlColor(aTmpPos2, aGradTransVector.aCol2, SDR_HANDLE_COLOR_SIZE_NORMAL, true);
                SdrHdlGradient* pGradHdl = new SdrHdlGradient(aTmpPos1, aTmpPos2, false);
                DBG_ASSERT(pColHdl1 && pColHdl2 && pGradHdl, "Could not get all necessary handles!");

                // link them
                pGradHdl->SetColorHandles(pColHdl1, pColHdl2);
                pGradHdl->SetObj(pObj);
                pColHdl1->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));
                pColHdl2->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));

                // insert them
                maHdlList.AddHdl(pColHdl1);
                maHdlList.AddHdl(pColHdl2);
                maHdlList.AddHdl(pGradHdl);
            }
            break;
        }
        case SDRDRAG_GRADIENT:
        {
            // add interactive gradient handle
            const size_t nMarkCount = GetMarkedObjectCount();
            if(nMarkCount == 1)
            {
                SdrObject* pObj = GetMarkedObjectByIndex(0);
                const SfxItemSet& rSet = pObj->GetMergedItemSet();
                drawing::FillStyle eFillStyle = static_cast<const XFillStyleItem&>(rSet.Get(XATTR_FILLSTYLE)).GetValue();

                if(eFillStyle == drawing::FillStyle_GRADIENT)
                {
                    // set values and transform to vector set
                    GradTransformer aGradTransformer;
                    GradTransVector aGradTransVector;
                    GradTransGradient aGradTransGradient;
                    Size aHdlSize(15, 15);

                    aGradTransGradient.aGradient = static_cast<const XFillGradientItem&>(rSet.Get(XATTR_FILLGRADIENT)).GetGradientValue();
                    GradTransformer::GradToVec(aGradTransGradient, aGradTransVector, pObj);

                    // build handles
                    const Point aTmpPos1(basegfx::fround(aGradTransVector.maPositionA.getX()), basegfx::fround(aGradTransVector.maPositionA.getY()));
                    const Point aTmpPos2(basegfx::fround(aGradTransVector.maPositionB.getX()), basegfx::fround(aGradTransVector.maPositionB.getY()));
                    SdrHdlColor* pColHdl1 = new SdrHdlColor(aTmpPos1, aGradTransVector.aCol1, aHdlSize, false);
                    SdrHdlColor* pColHdl2 = new SdrHdlColor(aTmpPos2, aGradTransVector.aCol2, aHdlSize, false);
                    SdrHdlGradient* pGradHdl = new SdrHdlGradient(aTmpPos1, aTmpPos2, true);
                    DBG_ASSERT(pColHdl1 && pColHdl2 && pGradHdl, "Could not get all necessary handles!");

                    // link them
                    pGradHdl->SetColorHandles(pColHdl1, pColHdl2);
                    pGradHdl->SetObj(pObj);
                    pColHdl1->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));
                    pColHdl2->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));

                    // insert them
                    maHdlList.AddHdl(pColHdl1);
                    maHdlList.AddHdl(pColHdl2);
                    maHdlList.AddHdl(pGradHdl);
                }
            }
            break;
        }
        case SDRDRAG_CROP:
        {
            // TODO
            break;
        }
        default: break;
    }
}

/** handle mouse over effects for handles */
bool SdrMarkView::MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin)
{
    if(maHdlList.GetHdlCount())
    {
        SdrHdl* pMouseOverHdl = 0;
        if( !rMEvt.IsLeaveWindow() && pWin )
        {
            Point aMDPos( pWin->PixelToLogic( rMEvt.GetPosPixel() ) );
            pMouseOverHdl = PickHandle(aMDPos);
        }

        // notify last mouse over handle that he lost the mouse
        const size_t nHdlCount = maHdlList.GetHdlCount();

        for(size_t nHdl = 0; nHdl < nHdlCount; ++nHdl)
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
        if( pMouseOverHdl )
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
            // first calculate the length of the axis of reflection
            long nOutMin=0;
            long nOutMax=0;
            long nMinLen=0;
            long nObjDst=0;
            long nOutHgt=0;
            OutputDevice* pOut=GetFirstOutputDevice();
            if (pOut!=NULL) {
                // minimum length: 50 pixels
                nMinLen=pOut->PixelToLogic(Size(0,50)).Height();
                // 20 pixels distance to the Obj for the reference point
                nObjDst=pOut->PixelToLogic(Size(0,20)).Height();
                // MinY/MaxY
                // margin = minimum length = 10 pixels
                long nDst=pOut->PixelToLogic(Size(0,10)).Height();
                nOutMin=-pOut->GetMapMode().GetOrigin().Y();
                nOutMax=pOut->GetOutputSize().Height()-1+nOutMin;
                nOutMin+=nDst;
                nOutMax-=nDst;
                // absolute minimum length, however, is 10 pixels
                if (nOutMax-nOutMin<nDst) {
                    nOutMin+=nOutMax+1;
                    nOutMin/=2;
                    nOutMin-=(nDst+1)/2;
                    nOutMax=nOutMin+nDst;
                }
                nOutHgt=nOutMax-nOutMin;
                // otherwise minimum length = 1/4 OutHgt
                long nTemp=nOutHgt/4;
                if (nTemp>nMinLen) nMinLen=nTemp;
            }

            Rectangle aR(GetMarkedObjBoundRect());
            Point aCenter(aR.Center());
            long nMarkHgt=aR.GetHeight()-1;
            long nHgt=nMarkHgt+nObjDst*2;       // 20 pixels overlapping above and below
            if (nHgt<nMinLen) nHgt=nMinLen;     // minimum length 50 pixels or 1/4 OutHgt, respectively

            long nY1=aCenter.Y()-(nHgt+1)/2;
            long nY2=nY1+nHgt;

            if (pOut!=NULL && nMinLen>nOutHgt) nMinLen=nOutHgt; // TODO: maybe shorten this a little

            if (pOut!=NULL) { // now move completely into the visible area
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
        SdrHdl* pH = maHdlList.GetHdl(HDL_REF1);
        if(pH)
            pH->SetPos(rPt);
    }
}

void SdrMarkView::SetRef2(const Point& rPt)
{
    if(eDragMode == SDRDRAG_MIRROR)
    {
        aRef2 = rPt;
        SdrHdl* pH = maHdlList.GetHdl(HDL_REF2);
        if(pH)
            pH->SetPos(rPt);
    }
}

bool SdrPageView::IsObjSelectable(SdrObject *pObj) const
{
    SdrLayerID nLay=pObj->GetLayer();
    bool bRaus=!pObj->IsInserted(); // Obj deleted?
    if (!pObj->Is3DObj()) {
        bRaus=bRaus || pObj->GetPage()!=GetPage();   // Obj suddenly in different Page or Group
    }
    bRaus=bRaus || GetLockedLayers().IsSet(nLay) ||  // Layer locked?
                   !GetVisibleLayers().IsSet(nLay);  // Layer invisible?

    if( !bRaus )
        bRaus = !pObj->IsVisible(); // invisible objects can not be selected

    if (!bRaus) {
        // Grouped objects can now be selected.
        // After EnterGroup the higher-level objects,
        // have to be deselected, though.
        const SdrObjList* pOOL=pObj->GetObjList();
        const SdrObjList* pVOL=GetObjList();
        while (pOOL!=NULL && pOOL!=pVOL) {
            pOOL=pOOL->GetUpList();
        }
        bRaus=pOOL!=pVOL;
    }
    return !bRaus;
}

void SdrMarkView::CheckMarked()
{
    for (size_t nm=GetMarkedObjectCount(); nm>0;) {
        --nm;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        bool bRaus=!pPV->IsObjSelectable(pObj);
        if (bRaus)
        {
            GetMarkedObjectListWriteAccess().DeleteMark(nm);
        }
        else
        {
            if (!IsGluePointEditMode()) { // selected glue points only in GlueEditMode
                SdrUShortCont* pPts=pM->GetMarkedGluePoints();
                if (pPts!=NULL) {
                    pPts->clear();
                }
            }
        }
    }

    // at least reset the remembered BoundRect to prevent handle
    // generation if bForceFrameHandles is TRUE.
    bMarkedObjRectDirty = true;
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

void SdrMarkView::SetFrameHandles(bool bOn)
{
    if (bOn!=bForceFrameHandles) {
        bool bOld=ImpIsFrameHandles();
        bForceFrameHandles=bOn;
        bool bNew=ImpIsFrameHandles();
        if (bNew!=bOld) {
            AdjustMarkHdl();
            MarkListHasChanged();
        }
    }
}

void SdrMarkView::SetEditMode(SdrViewEditMode eMode)
{
    if (eMode!=eEditMode) {
        bool bGlue0=eEditMode==SDREDITMODE_GLUEPOINTEDIT;
        bool bEdge0=static_cast<SdrCreateView*>(this)->IsEdgeTool();
        eEditMode0=eEditMode;
        eEditMode=eMode;
        bool bGlue1=eEditMode==SDREDITMODE_GLUEPOINTEDIT;
        bool bEdge1=static_cast<SdrCreateView*>(this)->IsEdgeTool();
        // avoid flickering when switching between GlueEdit and EdgeTool
        if (bGlue1 && !bGlue0) ImpSetGlueVisible2(bGlue1);
        if (bEdge1!=bEdge0) ImpSetGlueVisible3(bEdge1);
        if (!bGlue1 && bGlue0) ImpSetGlueVisible2(bGlue1);
        if (bGlue0 && !bGlue1) UnmarkAllGluePoints();
    }
}



bool SdrMarkView::IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const
{
    if (pObj)
    {
        if (pObj->IsMarkProtect() ||
            (!bDesignMode && pObj->IsUnoObj()))
        {
            // object not selectable or
            // SdrUnoObj not in DesignMode
            return false;
        }
    }
    return pPV==NULL || pPV->IsObjMarkable(pObj);
}

bool SdrMarkView::IsMarkedObjHit(const Point& rPnt, short nTol) const
{
    bool bRet=false;
    nTol=ImpGetHitTolLogic(nTol,NULL);
    Point aPt(rPnt);
    for (size_t nm=0; nm<GetMarkedObjectCount() && !bRet; ++nm) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        bRet = 0 != CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pM->GetPageView(),SdrSearchOptions::NONE,0);
    }
    return bRet;
}

SdrHdl* SdrMarkView::PickHandle(const Point& rPnt, SdrSearchOptions nOptions, SdrHdl* pHdl0) const
{
    if (bSomeObjChgdFlag) { // recalculate handles, if necessary
        FlushComeBackTimer();
    }
    bool bBack(nOptions & SdrSearchOptions::BACKWARD);
    bool bNext(nOptions & SdrSearchOptions::NEXT);
    Point aPt(rPnt);
    return maHdlList.IsHdlListHit(aPt,bBack,bNext,pHdl0);
}

bool SdrMarkView::MarkObj(const Point& rPnt, short nTol, bool bToggle, bool bDeep)
{
    SdrObject* pObj;
    SdrPageView* pPV;
    nTol=ImpGetHitTolLogic(nTol,NULL);
    SdrSearchOptions nOptions=SdrSearchOptions::PICKMARKABLE;
    if (bDeep) nOptions=nOptions|SdrSearchOptions::DEEP;
    bool bRet=PickObj(rPnt,(sal_uInt16)nTol,pObj,pPV,nOptions);
    if (bRet) {
        bool bUnmark=bToggle && IsObjMarked(pObj);
        MarkObj(pObj,pPV,bUnmark);
    }
    return bRet;
}

bool SdrMarkView::MarkNextObj(bool bPrev)
{
    SdrPageView* pPageView = GetSdrPageView();

    if(!pPageView)
    {
        return false;
    }

    SortMarkedObjects();
    const size_t nMarkCount=GetMarkedObjectCount();
    size_t nChgMarkNum = SAL_MAX_SIZE; // number of the MarkEntry we want to replace
    size_t nSearchObjNum = bPrev ? 0 : SAL_MAX_SIZE;
    if (nMarkCount!=0) {
        nChgMarkNum=bPrev ? 0 : nMarkCount-1;
        SdrMark* pM=GetSdrMarkByIndex(nChgMarkNum);
        OSL_ASSERT(pM!=NULL);
        if (pM->GetMarkedSdrObj() != NULL)
            nSearchObjNum = pM->GetMarkedSdrObj()->GetNavigationPosition();
    }

    SdrObject* pMarkObj=NULL;
    SdrObjList* pSearchObjList=pPageView->GetObjList();
    const size_t nObjAnz = pSearchObjList->GetObjCount();
    if (nObjAnz!=0) {
        if (nSearchObjNum>nObjAnz) nSearchObjNum=nObjAnz;
        while (pMarkObj==NULL && ((!bPrev && nSearchObjNum>0) || (bPrev && nSearchObjNum<nObjAnz)))
        {
            if (!bPrev)
                nSearchObjNum--;
            SdrObject* pSearchObj = pSearchObjList->GetObjectForNavigationPosition(nSearchObjNum);
            if (IsObjMarkable(pSearchObj,pPageView))
            {
                if (TryToFindMarkedObject(pSearchObj)==SAL_MAX_SIZE)
                {
                    pMarkObj=pSearchObj;
                }
            }
            if (bPrev) nSearchObjNum++;
        }
    }

    if(!pMarkObj)
    {
        return false;
    }

    if (nChgMarkNum!=SAL_MAX_SIZE)
    {
        GetMarkedObjectListWriteAccess().DeleteMark(nChgMarkNum);
    }
    MarkObj(pMarkObj,pPageView); // also calls MarkListHasChanged(), AdjustMarkHdl()
    return true;
}

bool SdrMarkView::MarkNextObj(const Point& rPnt, short nTol, bool bPrev)
{
    SortMarkedObjects();
    nTol=ImpGetHitTolLogic(nTol,NULL);
    Point aPt(rPnt);
    SdrMark* pTopMarkHit=NULL;
    SdrMark* pBtmMarkHit=NULL;
    size_t nTopMarkHit=0;
    size_t nBtmMarkHit=0;
    // find topmost of the selected objects that is hit by rPnt
    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nm=nMarkCount; nm>0 && pTopMarkHit==NULL;) {
        --nm;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        if(CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pM->GetPageView(),SdrSearchOptions::NONE,0))
        {
            pTopMarkHit=pM;
            nTopMarkHit=nm;
        }
    }
    // nothing found, in this case, just select an object
    if (pTopMarkHit==NULL) return MarkObj(rPnt,sal_uInt16(nTol),false);

    SdrObject* pTopObjHit=pTopMarkHit->GetMarkedSdrObj();
    SdrObjList* pObjList=pTopObjHit->GetObjList();
    SdrPageView* pPV=pTopMarkHit->GetPageView();
    // find lowermost of the selected objects that is hit by rPnt
    // and is placed on the same PageView as pTopMarkHit
    for (size_t nm=0; nm<nMarkCount && pBtmMarkHit==NULL; ++nm) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrPageView* pPV2=pM->GetPageView();
        if (pPV2==pPV && CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pPV2,SdrSearchOptions::NONE,0))
        {
            pBtmMarkHit=pM;
            nBtmMarkHit=nm;
        }
    }
    if (pBtmMarkHit==NULL) { pBtmMarkHit=pTopMarkHit; nBtmMarkHit=nTopMarkHit; }
    SdrObject* pBtmObjHit=pBtmMarkHit->GetMarkedSdrObj();
    const size_t nObjAnz = pObjList->GetObjCount();

    size_t nSearchBeg = 0;
    E3dScene* pScene = NULL;
    SdrObject* pObjHit = (bPrev) ? pBtmObjHit : pTopObjHit;
    bool bRemap = pObjHit->ISA(E3dCompoundObject)
        && static_cast<E3dCompoundObject*>(pObjHit)->IsAOrdNumRemapCandidate(pScene);

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

    size_t no=nSearchBeg;
    SdrObject* pFndObj=NULL;
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

        if (CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pObj,pPV,SdrSearchOptions::TESTMARKABLE,0))
        {
            if (TryToFindMarkedObject(pObj)==SAL_MAX_SIZE) {
                pFndObj=pObj;
            } else {
                // TODO: for performance reasons set on to Top or Btm, if necessary
            }
        }
        if (bPrev) no++;
    }
    if (pFndObj!=NULL)
    {
        GetMarkedObjectListWriteAccess().DeleteMark(bPrev?nBtmMarkHit:nTopMarkHit);
        GetMarkedObjectListWriteAccess().InsertEntry(SdrMark(pFndObj,pPV));
        MarkListHasChanged();
        AdjustMarkHdl();
    }
    return pFndObj!=NULL;
}

bool SdrMarkView::MarkObj(const Rectangle& rRect, bool bUnmark)
{
    bool bFnd=false;
    Rectangle aR(rRect);
    SdrObjList* pObjList;
    BrkAction();
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        pObjList=pPV->GetObjList();
        Rectangle aFrm1(aR);
        const size_t nObjAnz = pObjList->GetObjCount();
        for (size_t nO=0; nO<nObjAnz; ++nO) {
            SdrObject* pObj=pObjList->GetObj(nO);
            Rectangle aRect(pObj->GetCurrentBoundRect());
            if (aFrm1.IsInside(aRect)) {
                if (!bUnmark) {
                    if (IsObjMarkable(pObj,pPV))
                    {
                        GetMarkedObjectListWriteAccess().InsertEntry(SdrMark(pObj,pPV));
                        bFnd=true;
                    }
                } else {
                    const size_t nPos=TryToFindMarkedObject(pObj);
                    if (nPos!=SAL_MAX_SIZE)
                    {
                        GetMarkedObjectListWriteAccess().DeleteMark(nPos);
                        bFnd=true;
                    }
                }
            }
        }
    }
    if (bFnd) {
        SortMarkedObjects();
        MarkListHasChanged();
        AdjustMarkHdl();
    }
    return bFnd;
}

void SdrMarkView::MarkObj(SdrObject* pObj, SdrPageView* pPV, bool bUnmark, bool bImpNoSetMarkHdl)
{
    if (pObj!=NULL && pPV!=NULL && IsObjMarkable(pObj, pPV)) {
        BrkAction();
        if (!bUnmark)
        {
            GetMarkedObjectListWriteAccess().InsertEntry(SdrMark(pObj,pPV));
        }
        else
        {
            const size_t nPos=TryToFindMarkedObject(pObj);
            if (nPos!=SAL_MAX_SIZE)
            {
                GetMarkedObjectListWriteAccess().DeleteMark(nPos);
            }
        }
        if (!bImpNoSetMarkHdl) {
            MarkListHasChanged();
            AdjustMarkHdl();
        }
    }
}

bool SdrMarkView::IsObjMarked(SdrObject* pObj) const
{
    return TryToFindMarkedObject(pObj)!=SAL_MAX_SIZE;
}

sal_uInt16 SdrMarkView::GetMarkHdlSizePixel() const
{
    return maHdlList.GetHdlSize()*2+1;
}

void SdrMarkView::SetMarkHdlSizePixel(sal_uInt16 nSiz)
{
    if (nSiz<3) nSiz=3;
    nSiz/=2;
    if (nSiz!=maHdlList.GetHdlSize()) {
        maHdlList.SetHdlSize(nSiz);
    }
}

SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, SdrSearchOptions nOptions, const SetOfByte* pMVisLay) const
{
    if(((nOptions & SdrSearchOptions::IMPISMASTER) && pObj->IsNotVisibleAsMaster()) || (!pObj->IsVisible()))
    {
        return NULL;
    }

    const bool bCheckIfMarkable(nOptions & SdrSearchOptions::TESTMARKABLE);
    const bool bDeep(nOptions & SdrSearchOptions::DEEP);
    const bool bOLE(pObj->ISA(SdrOle2Obj));
    const bool bTXT(pObj->ISA(SdrTextObj) && static_cast<SdrTextObj*>(pObj)->IsTextFrame());
    SdrObject* pRet=NULL;
    Rectangle aRect(pObj->GetCurrentBoundRect());
    // hack for calc grid sync
    aRect += pObj->GetGridOffset();
    sal_uInt16 nTol2(nTol);

    // double tolerance for OLE, text frames and objects in
    // active text edit
    if(bOLE || bTXT || pObj==static_cast<const SdrObjEditView*>(this)->GetTextEditObject())
    {
        nTol2*=2;
    }

    aRect.Left  ()-=nTol2; // add 1 tolerance for all objects
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
                // adjustment hit point for virtual objects
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

SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList* pOL, SdrPageView* pPV, SdrSearchOptions nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj) const
{
    return (*this).CheckSingleSdrObjectHit(rPnt,nTol,pOL,pPV,nOptions,pMVisLay,rpRootObj,NULL);
}
SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList* pOL, SdrPageView* pPV, SdrSearchOptions nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj,const SdrMarkList * pMarkList) const
{
    bool bBack(nOptions & SdrSearchOptions::BACKWARD);
    bool bBefMrk(nOptions & SdrSearchOptions::BEFOREMARK);
    SdrObject* pRet=NULL;
    rpRootObj=NULL;
    if (pOL!=NULL)
    {
        bool bRemap(pOL->GetOwnerObj() && pOL->GetOwnerObj()->ISA(E3dScene));
        E3dScene* pRemapScene = (bRemap ? static_cast<E3dScene*>(pOL->GetOwnerObj()) : 0L);

        const size_t nObjAnz=pOL->GetObjCount();
        size_t nObjNum=bBack ? 0 : nObjAnz;
        while (pRet==NULL && (bBack ? nObjNum<nObjAnz : nObjNum>0)) {
            if (!bBack) nObjNum--;
            SdrObject* pObj;

            if(bRemap)
            {
                pObj = pOL->GetObj(pRemapScene->RemapOrdNum(nObjNum));
            }
            else
            {
                pObj = pOL->GetObj(nObjNum);
            }
            if (bBefMrk)
            {
                if ((pMarkList)!=NULL)
                {
                    if ((*pMarkList).FindObject(pObj)!=SAL_MAX_SIZE)
                    {
                        return NULL;
                    }
                }
            }
            pRet=CheckSingleSdrObjectHit(rPnt,nTol,pObj,pPV,nOptions,pMVisLay);
            if (pRet!=NULL) rpRootObj=pObj;
            if (bBack) nObjNum++;
        }
    }
    return pRet;
}

bool SdrMarkView::PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, SdrSearchOptions nOptions) const
{
    return PickObj(rPnt,nTol,rpObj,rpPV,nOptions,NULL,NULL);
}

bool SdrMarkView::PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, SdrSearchOptions nOptions, SdrObject** ppRootObj, bool* pbHitPassDirect) const
{ // TODO: lacks a Pass2,Pass3
    SortMarkedObjects();
    if (ppRootObj!=NULL) *ppRootObj=NULL;
    if (pbHitPassDirect!=NULL) *pbHitPassDirect=true;
    rpObj=NULL;
    rpPV=NULL;
    bool bWholePage(nOptions & SdrSearchOptions::WHOLEPAGE);
    bool bMarked(nOptions & SdrSearchOptions::MARKED);
    bool bMasters=!bMarked && bool(nOptions & SdrSearchOptions::ALSOONMASTER);
    bool bBack(nOptions & SdrSearchOptions::BACKWARD);
#if OSL_DEBUG_LEVEL > 0
    bool bNext(nOptions & SdrSearchOptions::NEXT); (void)bNext; // n.i.
    bool bBoundCheckOn2ndPass(nOptions & SdrSearchOptions::PASS2BOUND); (void)bBoundCheckOn2ndPass;// n.i.
    bool bCheckNearestOn3rdPass(nOptions & SdrSearchOptions::PASS3NEAREST); (void)bCheckNearestOn3rdPass;// n.i.
#endif
    if (nTol<0) nTol=ImpGetHitTolLogic(nTol,NULL);
    Point aPt(rPnt);
    SdrObject* pObj=NULL;
    SdrObject* pHitObj=NULL;
    SdrPageView* pPV=NULL;
    if (!bBack && static_cast<const SdrObjEditView*>(this)->IsTextEditFrameHit(rPnt)) {
        pObj=static_cast<const SdrObjEditView*>(this)->GetTextEditObject();
        pHitObj=pObj;
        pPV=static_cast<const SdrObjEditView*>(this)->GetTextEditPageView();
    }
    if (bMarked) {
        const size_t nMrkAnz=GetMarkedObjectCount();
        size_t nMrkNum=bBack ? 0 : nMrkAnz;
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

            bool bExtraPassForWholePage=bWholePage && pPage!=pPV->GetObjList();
            if (bExtraPassForWholePage) nPgAnz++; // First search in AktObjList, then on the entire page
            sal_uInt16 nPgNum=bBack ? 0 : nPgAnz;
            while (pHitObj==NULL && (bBack ? nPgNum<nPgAnz : nPgNum>0)) {
                SdrSearchOptions nTmpOptions=nOptions;
                if (!bBack) nPgNum--;
                const SetOfByte* pMVisLay=NULL;
                SdrObjList* pObjList=NULL;
                if (pbHitPassDirect!=NULL) *pbHitPassDirect = true;
                if (nPgNum>=nPgAnz-1 || (bExtraPassForWholePage && nPgNum>=nPgAnz-2))
                {
                    pObjList=pPV->GetObjList();
                    if (bExtraPassForWholePage && nPgNum==nPgAnz-2) {
                        pObjList=pPage;
                        if (pbHitPassDirect!=NULL) *pbHitPassDirect = false;
                    }
                }
                else
                {
                    // otherwise MasterPage
                    SdrPage& rMasterPage = pPage->TRG_GetMasterPage();
                    pMVisLay = &pPage->TRG_GetMasterPageVisibleLayers();
                    pObjList = &rMasterPage;

                    if (pbHitPassDirect!=NULL) *pbHitPassDirect = false;
                    nTmpOptions=nTmpOptions | SdrSearchOptions::IMPISMASTER;
                }
                pHitObj=CheckSingleSdrObjectHit(aPt,nTol,pObjList,pPV,nTmpOptions,pMVisLay,pObj,&(GetMarkedObjectList()));
                if (bBack) nPgNum++;
            }
        }
    }
    if (pHitObj!=NULL) {
        if (ppRootObj!=NULL) *ppRootObj=pObj;
        if (nOptions & SdrSearchOptions::DEEP) pObj=pHitObj;
        if (nOptions & SdrSearchOptions::TESTTEXTEDIT) {
            if (!pObj->HasTextEdit() || pPV->GetLockedLayers().IsSet(pObj->GetLayer())) {
                pObj=NULL;
            }
        }
        if (pObj!=NULL && (nOptions & SdrSearchOptions::TESTMACRO)) {
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aPt;
            aHitRec.aDownPos=aPt;
            aHitRec.nTol=nTol;
            aHitRec.pVisiLayer=&pPV->GetVisibleLayers();
            aHitRec.pPageView=pPV;
            if (!pObj->HasMacro() || !pObj->IsMacroHit(aHitRec)) pObj=NULL;
        }
        if (pObj!=NULL && (nOptions & SdrSearchOptions::WITHTEXT) && pObj->GetOutlinerParaObject()==NULL) pObj=NULL;
        if (pObj!=NULL && (nOptions & SdrSearchOptions::TESTTEXTAREA) && pPV)
        {
            if(!SdrObjectPrimitiveHit(*pObj, aPt, 0, *pPV, 0, true))
            {
                pObj = 0;
            }
        }
        if (pObj!=NULL) {
            rpObj=pObj;
            rpPV=pPV;
        }
    }
    return rpObj!=NULL;
}

bool SdrMarkView::PickMarkedObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, SdrSearchOptions nOptions) const
{
    SortMarkedObjects();
    bool bBoundCheckOn2ndPass(nOptions & SdrSearchOptions::PASS2BOUND);
    bool bCheckNearestOn3rdPass(nOptions & SdrSearchOptions::PASS3NEAREST);
    rpObj=NULL;
    rpPV=NULL;
    Point aPt(rPnt);
    sal_uInt16 nTol=(sal_uInt16)nHitTolLog;
    bool bFnd=false;
    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nMarkNum=nMarkCount; nMarkNum>0 && !bFnd;) {
        --nMarkNum;
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        SdrPageView* pPV=pM->GetPageView();
        SdrObject* pObj=pM->GetMarkedSdrObj();
        bFnd = 0 != CheckSingleSdrObjectHit(aPt,nTol,pObj,pPV,SdrSearchOptions::TESTMARKABLE,0);
        if (bFnd) {
            rpObj=pObj;
            rpPV=pPV;
        }
    }
    if ((bBoundCheckOn2ndPass || bCheckNearestOn3rdPass) && !bFnd) {
        SdrObject* pBestObj=NULL;
        SdrPageView* pBestPV=NULL;
        sal_uIntPtr nBestDist=ULONG_MAX;
        for (size_t nMarkNum=nMarkCount; nMarkNum>0 && !bFnd;) {
            --nMarkNum;
            SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            SdrPageView* pPV=pM->GetPageView();
            SdrObject* pObj=pM->GetMarkedSdrObj();
            Rectangle aRect(pObj->GetCurrentBoundRect());
            aRect.Left  ()-=nTol;
            aRect.Top   ()-=nTol;
            aRect.Right ()+=nTol;
            aRect.Bottom()+=nTol;
            if (aRect.IsInside(aPt)) {
                bFnd=true;
                rpObj=pObj;
                rpPV=pPV;
            } else if (bCheckNearestOn3rdPass) {
                sal_uIntPtr nDist=0;
                if (aPt.X()<aRect.Left())   nDist+=aRect.Left()-aPt.X();
                if (aPt.X()>aRect.Right())  nDist+=aPt.X()-aRect.Right();
                if (aPt.Y()<aRect.Top())    nDist+=aRect.Top()-aPt.Y();
                if (aPt.Y()>aRect.Bottom()) nDist+=aPt.Y()-aRect.Bottom();
                if (nDist<nBestDist) {
                    pBestObj=pObj;
                    pBestPV=pPV;
                }
            }
        }
        if (bCheckNearestOn3rdPass && !bFnd) {
            rpObj=pBestObj;
            rpPV=pBestPV;
            bFnd=pBestObj!=NULL;
        }
    }
    return bFnd;
}


void SdrMarkView::UnmarkAllObj(SdrPageView* pPV)
{
    if (GetMarkedObjectCount()!=0) {
        BrkAction();
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
        AdjustMarkHdl();
    }
}

void SdrMarkView::MarkAllObj(SdrPageView* _pPV)
{
    BrkAction();

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
        AdjustMarkHdl();
    }
}

void SdrMarkView::AdjustMarkHdl()
{
    CheckMarked();
    SetMarkRects();
    SetMarkHandles();
}

Rectangle SdrMarkView::GetMarkedObjBoundRect() const
{
    Rectangle aRect;
    for (size_t nm=0; nm<GetMarkedObjectCount(); ++nm) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        Rectangle aR1(pO->GetCurrentBoundRect());
        // Ensure marked area includes the calc offset
        // ( if applicable ) to sync to grid
        aR1 += pO->GetGridOffset();
        if (aRect.IsEmpty()) aRect=aR1;
        else aRect.Union(aR1);
    }
    return aRect;
}

Point SdrMarkView::GetGridOffset() const
{
    Point aOffset;
    // calculate the area occupied by the union of each marked object
    // ( synced to grid ) and compare to the same unsynced area to calculate
    // the offset. Hopefully that's the sensible thing to do
    const Rectangle& aGroupSyncedRect = GetMarkedObjRect();
    aOffset =   aGroupSyncedRect.TopLeft() - aMarkedObjRectNoOffset.TopLeft();
    return aOffset;
}

const Rectangle& SdrMarkView::GetMarkedObjRect() const
{
    if (bMarkedObjRectDirty) {
        const_cast<SdrMarkView*>(this)->bMarkedObjRectDirty=false;
        Rectangle aRect;
        Rectangle aRect2;
        for (size_t nm=0; nm<GetMarkedObjectCount(); ++nm) {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pO = pM->GetMarkedSdrObj();
            if (!pO)
                continue;
            Rectangle aR1(pO->GetSnapRect());
            // apply calc offset to marked object rect
            // ( necessary for handles to be displayed in
            // correct position )
            if (aRect2.IsEmpty()) aRect2=aR1;
            else aRect2.Union( aR1 );
            aR1 += pO->GetGridOffset();
            if (aRect.IsEmpty()) aRect=aR1;
            else aRect.Union(aR1);
        }
        const_cast<SdrMarkView*>(this)->aMarkedObjRect=aRect;
        const_cast<SdrMarkView*>(this)->aMarkedObjRectNoOffset=aRect2;
    }
    return aMarkedObjRect;
}



void SdrMarkView::ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, sal_uInt16 nVal, ImpTakeDescriptionOptions nOpt) const
{
    rStr = ImpGetResStr(nStrCacheID);
    sal_Int32 nPos = rStr.indexOf("%1");

    if(nPos != -1)
    {
        if(nOpt == ImpTakeDescriptionOptions::POINTS)
        {
            rStr = rStr.replaceAt(nPos, 2, GetDescriptionOfMarkedPoints());
        }
        else if(nOpt == ImpTakeDescriptionOptions::GLUEPOINTS)
        {
            rStr = rStr.replaceAt(nPos, 2, GetDescriptionOfMarkedGluePoints());
        }
        else
        {
            rStr = rStr.replaceAt(nPos, 2, GetDescriptionOfMarkedObjects());
        }
    }

    rStr = rStr.replaceFirst("%2", OUString::number( nVal ));
}



bool SdrMarkView::EnterMarkedGroup()
{
    bool bRet=false;
    // We enter only the first group found (in only one PageView), because
    // PageView::EnterGroup calls an AdjustMarkHdl.
    // TODO: I'll have to prevent that via a flag.
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        bool bEnter=false;
        for (size_t nm = GetMarkedObjectCount(); nm > 0 && !bEnter;)
        {
            --nm;
            SdrMark* pM=GetSdrMarkByIndex(nm);
            if (pM->GetPageView()==pPV) {
                SdrObject* pObj=pM->GetMarkedSdrObj();
                if (pObj->IsGroupObject()) {
                    if (pPV->EnterGroup(pObj)) {
                        bRet=true;
                        bEnter=true;
                    }
                }
            }
        }
    }
    return bRet;
}



void SdrMarkView::MarkListHasChanged()
{
    GetMarkedObjectListWriteAccess().SetNameDirty();
    SetEdgesOfMarkedNodesDirty();

    bMarkedObjRectDirty=true;
    bMarkedPointsRectsDirty=true;
#ifdef DBG_UTIL
    if (pItemBrowser!=nullptr) pItemBrowser->SetDirty();
#endif
    bool bOneEdgeMarked=false;
    if (GetMarkedObjectCount()==1) {
        const SdrObject* pObj=GetMarkedObjectByIndex(0);
        if (pObj->GetObjInventor()==SdrInventor) {
            sal_uInt16 nIdent=pObj->GetObjIdentifier();
            bOneEdgeMarked=nIdent==OBJ_EDGE;
        }
    }
    ImpSetGlueVisible4(bOneEdgeMarked);
}



void SdrMarkView::SetMoveOutside(bool bOn)
{
    maHdlList.SetMoveOutside(bOn);
}

void SdrMarkView::SetDesignMode( bool _bOn )
{
    if ( bDesignMode != _bOn )
    {
        bDesignMode = _bOn;
        SdrPageView* pPageView = GetSdrPageView();
        if ( pPageView )
            pPageView->SetDesignMode( _bOn );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
