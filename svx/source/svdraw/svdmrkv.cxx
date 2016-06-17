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
#include <sfx2/viewsh.hxx>

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
    meDragMode=SDRDRAG_MOVE;
    mbRefHdlShownOnly=false;
    meEditMode=SDREDITMODE_EDIT;
    meEditMode0=SDREDITMODE_EDIT;
    mbDesignMode=false;
    mpMarkedObj=nullptr;
    mpMarkedPV=nullptr;
    mbForceFrameHandles=false;
    mbPlusHdlAlways=false;
    mnFrameHandlesLimit=50;
    mbInsPolyPoint=false;
    mnInsPointNum = 0L;
    mbMarkedObjRectDirty=false;
    mbMarkedPointsRectsDirty=false;
    mbMarkHandlesHidden = false;
    mbMrkPntDirty=false;
    mbMarkHdlWhenTextEdit=false;
    mbMarkableObjCountDirty=false; // not yet implemented
    mnMarkableObjCount=0;          // not yet implemented

    // Migrate selections
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

SdrMarkView::SdrMarkView(SdrModel* pModel1, OutputDevice* pOut)
:   SdrSnapView(pModel1,pOut),
    mpMarkObjOverlay(nullptr),
    mpMarkPointsOverlay(nullptr),
    mpMarkGluePointsOverlay(nullptr),
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
            mbMarkedObjRectDirty=true;
            mbMarkedPointsRectsDirty=true;
        }
    }
    SdrSnapView::Notify(rBC,rHint);
}

void SdrMarkView::ModelHasChanged()
{
    SdrPaintView::ModelHasChanged();
    GetMarkedObjectListWriteAccess().SetNameDirty();
    mbMarkedObjRectDirty=true;
    mbMarkedPointsRectsDirty=true;
    // Example: Obj is selected and maMarkedObjectList is sorted.
    // In another View 2, the ObjOrder is changed (e. g. MovToTop())
    // Then we need to re-sort MarkList.
    GetMarkedObjectListWriteAccess().SetUnsorted();
    SortMarkedObjects();
    mbMrkPntDirty=true;
    UndirtyMrkPnt();
    SdrView* pV=static_cast<SdrView*>(this);
    if (pV!=nullptr && !pV->IsDragObj() && !pV->IsInsObjPoint()) {
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
            if (mpMarkedPV)
            {
                if (OutputDevice* pOutputDevice = mpMarkedPV->GetView().GetFirstOutputDevice())
                {
                    if (pOutputDevice->GetMapMode().GetMapUnit() == MAP_100TH_MM)
                        aSelection = OutputDevice::LogicToLogic(aSelection, MAP_100TH_MM, MAP_TWIP);
                }
            }

            sSelection = aSelection.toString();
        }

        if(SfxViewShell* pViewShell = SfxViewShell::Current())
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_TILES, sSelection.getStr());
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
        rRect = Rectangle(maDragStat.GetStart(), maDragStat.GetNow());
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

    DBG_ASSERT(nullptr == mpMarkObjOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkObjOverlay (!)");
    basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
    mpMarkObjOverlay = new ImplMarkingOverlay(*this, aStartPos, bUnmark);

    maDragStat.Reset(rPnt);
    maDragStat.NextPoint();
    maDragStat.SetMinMove(mnMinMovLog);

    return true;
}

void SdrMarkView::MovMarkObj(const Point& rPnt)
{
    if(IsMarkObj() && maDragStat.CheckMinMoved(rPnt))
    {
        maDragStat.NextMove(rPnt);
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
        if(maDragStat.IsMinMoved())
        {
            Rectangle aRect(maDragStat.GetStart(), maDragStat.GetNow());
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
        mpMarkObjOverlay = nullptr;
    }
}


bool SdrMarkView::BegMarkPoints(const Point& rPnt, bool bUnmark)
{
    if(HasMarkablePoints())
    {
        BrkAction();

        DBG_ASSERT(nullptr == mpMarkPointsOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkPointsOverlay (!)");
        basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
        mpMarkPointsOverlay = new ImplMarkingOverlay(*this, aStartPos, bUnmark);

        maDragStat.Reset(rPnt);
        maDragStat.NextPoint();
        maDragStat.SetMinMove(mnMinMovLog);

        return true;
    }

    return false;
}

void SdrMarkView::MovMarkPoints(const Point& rPnt)
{
    if(IsMarkPoints() && maDragStat.CheckMinMoved(rPnt))
    {
        maDragStat.NextMove(rPnt);

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
        if(maDragStat.IsMinMoved())
        {
            Rectangle aRect(maDragStat.GetStart(), maDragStat.GetNow());
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
        mpMarkPointsOverlay = nullptr;
    }
}


bool SdrMarkView::BegMarkGluePoints(const Point& rPnt, bool bUnmark)
{
    if(HasMarkableGluePoints())
    {
        BrkAction();

        DBG_ASSERT(nullptr == mpMarkGluePointsOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkGluePointsOverlay (!)");

        basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
        mpMarkGluePointsOverlay = new ImplMarkingOverlay(*this, aStartPos, bUnmark);
        maDragStat.Reset(rPnt);
        maDragStat.NextPoint();
        maDragStat.SetMinMove(mnMinMovLog);

        return true;
    }

    return false;
}

void SdrMarkView::MovMarkGluePoints(const Point& rPnt)
{
    if(IsMarkGluePoints() && maDragStat.CheckMinMoved(rPnt))
    {
        maDragStat.NextMove(rPnt);

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
        if(maDragStat.IsMinMoved())
        {
            Rectangle aRect(maDragStat.GetStart(),maDragStat.GetNow());
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
        mpMarkGluePointsOverlay = nullptr;
    }
}

sal_uIntPtr SdrMarkView::GetMarkableObjCount() const
{
    sal_uIntPtr nCount=0;
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        SdrObjList* pOL=pPV->GetObjList();
        const size_t nObjCount = pOL->GetObjCount();
        for (size_t nObjNum=0; nObjNum<nObjCount; ++nObjNum) {
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
        const size_t nObjCount = pOL->GetObjCount();
        for (size_t nObjNum = 0; nObjNum<nObjCount; ++nObjNum) {
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
    bool bFrmHdl=nMarkCount>static_cast<size_t>(mnFrameHandlesLimit) || mbForceFrameHandles;
    bool bStdDrag=meDragMode==SDRDRAG_MOVE;
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
        if (meDragMode==SDRDRAG_ROTATE) {
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
    if(bFrmHdl && SDRDRAG_CROP == meDragMode)
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
    SdrObject* pSaveObj = nullptr;

    if(pSaveOldFocusHdl
        && pSaveOldFocusHdl->GetObj()
        && dynamic_cast<const SdrPathObj*>(pSaveOldFocusHdl->GetObj()) != nullptr
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
    maHdlList.SetRotateShear(meDragMode==SDRDRAG_ROTATE);
    maHdlList.SetDistortShear(meDragMode==SDRDRAG_SHEAR);
    mpMarkedObj=nullptr;
    mpMarkedPV=nullptr;

    // are handles enabled at all? Create only then
    if(!areMarkHandlesHidden())
    {
        const size_t nMarkCount=GetMarkedObjectCount();
        bool bStdDrag=meDragMode==SDRDRAG_MOVE;
        bool bSingleTextObjMark=false;

        if (nMarkCount==1)
        {
            mpMarkedObj=GetMarkedObjectByIndex(0);
            bSingleTextObjMark =
                mpMarkedObj &&
                dynamic_cast<const SdrTextObj*>( mpMarkedObj) !=  nullptr &&
                static_cast<SdrTextObj*>(mpMarkedObj)->IsTextFrame();
        }

        bool bFrmHdl=ImpIsFrameHandles();

        if (nMarkCount>0)
        {
            mpMarkedPV=GetSdrPageViewOfMarkedByIndex(0);

            for (size_t nMarkNum=0; nMarkNum<nMarkCount && (mpMarkedPV!=nullptr || !bFrmHdl); ++nMarkNum)
            {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);

                if (mpMarkedPV!=pM->GetPageView())
                {
                    mpMarkedPV=nullptr;
                }
            }
        }

        // apply calc offset to marked object rect
        // ( necessary for handles to be displayed in
        // correct position )
        Point aGridOff = GetGridOffset();

        // There can be multiple mark views, but we're only interested in the one that has a window associated.
        const bool bTiledRendering = comphelper::LibreOfficeKit::isActive() && GetFirstOutputDevice() && GetFirstOutputDevice()->GetOutDevType() == OUTDEV_WINDOW;

        // check if text edit or ole is active and handles need to be suppressed. This may be the case
        // when a single object is selected
        // Using a strict return statement is okay here; no handles means *no* handles.
        if(mpMarkedObj)
        {
            // formally #i33755#: If TextEdit is active the EditEngine will directly paint
            // to the window, so suppress Overlay and handles completely; a text frame for
            // the active text edit will be painted by the repaint mechanism in
            // SdrObjEditView::ImpPaintOutlinerView in this case. This needs to be reworked
            // in the future
            // Also formally #122142#: Pretty much the same for SdrCaptionObj's in calc.
            if(static_cast<SdrView*>(this)->IsTextEdit())
            {
                const SdrTextObj* pSdrTextObj = dynamic_cast< const SdrTextObj* >(mpMarkedObj);

                if(pSdrTextObj && pSdrTextObj->IsInEditMode())
                {
                    if (bTiledRendering)
                    {
                        // Suppress handles -> empty graphic selection.
                        if(SfxViewShell* pViewShell = SfxViewShell::Current())
                            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_GRAPHIC_SELECTION, "EMPTY");
                    }
                    return;
                }
            }

            // formally #i118524#: if inplace activated OLE is selected, suppress handles
            const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(mpMarkedObj);

            if(pSdrOle2Obj && (pSdrOle2Obj->isInplaceActive() || pSdrOle2Obj->isUiActive()))
            {
                return;
            }

            if (bTiledRendering && mpMarkedObj->GetObjIdentifier() == OBJ_TABLE)
            {
                rtl::Reference<sdr::SelectionController> xController = static_cast<SdrView*>(this)->getSelectionController();
                if (xController.is() && xController->hasSelectedCells())
                {
                    // The table shape has selected cells, which provide text selection already -> no graphic selection.
                    if(SfxViewShell* pViewShell = SfxViewShell::Current())
                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_GRAPHIC_SELECTION, "EMPTY");
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
                if (mpMarkedPV)
                {
                    if (OutputDevice* pOutputDevice = mpMarkedPV->GetView().GetFirstOutputDevice())
                    {
                        if (pOutputDevice->GetMapMode().GetMapUnit() == MAP_100TH_MM)
                            aSelection = OutputDevice::LogicToLogic(aSelection, MAP_100TH_MM, MAP_TWIP);
                    }
                }

                sSelection = aSelection.toString();

                // hide the text selection too
                if(SfxViewShell* pViewShell = SfxViewShell::Current())
                    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, "");
            }
            if(SfxViewShell* pViewShell = SfxViewShell::Current())
                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_GRAPHIC_SELECTION, sSelection.getStr());
        }

        if (bFrmHdl)
        {
            if(!aRect.IsEmpty())
            { // otherwise nothing is found
                if( bSingleTextObjMark )
                {
                    const size_t nSiz0=maHdlList.GetHdlCount();
                    mpMarkedObj->AddToHdlList(maHdlList);
                    const size_t nSiz1=maHdlList.GetHdlCount();
                    for (size_t i=nSiz0; i<nSiz1; ++i)
                    {
                        SdrHdl* pHdl=maHdlList.GetHdl(i);
                        pHdl->SetObj(mpMarkedObj);
                        pHdl->SetPos( pHdl->GetPos() + aGridOff );
                        pHdl->SetPageView(mpMarkedPV);
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
            if(1 == nMarkCount && mpMarkedObj && SDRDRAG_CROP == meDragMode)
            {
                // Default addCropHandles from SdrObject does nothing. When pMarkedObj is SdrGrafObj, previous
                // behaviour occurs (code in svx/source/svdraw/svdograf.cxx). When pMarkedObj is SwVirtFlyDrawObj
                // writer takes the responsibility of adding handles (code in sw/source/core/draw/dflyobj.cxx)
                mpMarkedObj->addCropHandles(maHdlList);
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
                            bool bSelected=pMrkPnts!=nullptr
                                      && pMrkPnts->find( sal_uInt16(i-nSiz0) ) != pMrkPnts->end();
                            pHdl->SetSelected(bSelected);
                            if (mbPlusHdlAlways || bSelected)
                            {
                                sal_uInt32 nPlusAnz=pObj->GetPlusHdlCount(*pHdl);
                                for (sal_uInt32 nPlusNum=0; nPlusNum<nPlusAnz; nPlusNum++)
                                {
                                    SdrHdl* pPlusHdl=pObj->GetPlusHdl(*pHdl,nPlusNum);
                                    if (pPlusHdl!=nullptr)
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
            if (pMrkGlue!=nullptr)
            {
                const SdrGluePointList* pGPL=pObj->GetGluePointList();
                if (pGPL!=nullptr)
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
        AddDragModeHdl(meDragMode);

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
    SdrDragMode eMode0=meDragMode;
    meDragMode=eMode;
    if (meDragMode==SDRDRAG_RESIZE) meDragMode=SDRDRAG_MOVE;
    if (meDragMode!=eMode0) {
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
            SdrHdl* pHdl = new SdrHdl(maRef1, HDL_REF1);

            maHdlList.AddHdl(pHdl);

            break;
        }
        case SDRDRAG_MIRROR:
        {
            // add axis of reflection
            SdrHdl* pHdl3 = new SdrHdl(maRef2, HDL_REF2);
            SdrHdl* pHdl2 = new SdrHdl(maRef1, HDL_REF1);
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
        SdrHdl* pMouseOverHdl = nullptr;
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
    switch(meDragMode)
    {
        case SDRDRAG_ROTATE:
        {
            Rectangle aR(GetMarkedObjRect());
            maRef1 = aR.Center();

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
            if (pOut!=nullptr) {
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

            if (pOut!=nullptr && nMinLen>nOutHgt) nMinLen=nOutHgt; // TODO: maybe shorten this a little

            if (pOut!=nullptr) { // now move completely into the visible area
                if (nY1<nOutMin) {
                    nY1=nOutMin;
                    if (nY2<nY1+nMinLen) nY2=nY1+nMinLen;
                }
                if (nY2>nOutMax) {
                    nY2=nOutMax;
                    if (nY1>nY2-nMinLen) nY1=nY2-nMinLen;
                }
            }

            maRef1.X()=aCenter.X();
            maRef1.Y()=nY1;
            maRef2.X()=aCenter.X();
            maRef2.Y()=nY2;

            break;
        }

        case SDRDRAG_TRANSPARENCE:
        case SDRDRAG_GRADIENT:
        case SDRDRAG_CROP:
        {
            Rectangle aRect(GetMarkedObjBoundRect());
            maRef1 = aRect.TopLeft();
            maRef2 = aRect.BottomRight();
            break;
        }
        default: break;
    }
}

void SdrMarkView::SetRef1(const Point& rPt)
{
    if(meDragMode == SDRDRAG_ROTATE || meDragMode == SDRDRAG_MIRROR)
    {
        maRef1 = rPt;
        SdrHdl* pH = maHdlList.GetHdl(HDL_REF1);
        if(pH)
            pH->SetPos(rPt);
    }
}

void SdrMarkView::SetRef2(const Point& rPt)
{
    if(meDragMode == SDRDRAG_MIRROR)
    {
        maRef2 = rPt;
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
        while (pOOL!=nullptr && pOOL!=pVOL) {
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
        SdrMark* pM = GetSdrMarkByIndex(nm);
        SdrObject* pObj = pM->GetMarkedSdrObj();
        SdrPageView* pPV = pM->GetPageView();
        bool bRaus = !pObj || !pPV->IsObjSelectable(pObj);
        if (bRaus)
        {
            GetMarkedObjectListWriteAccess().DeleteMark(nm);
        }
        else
        {
            if (!IsGluePointEditMode()) { // selected glue points only in GlueEditMode
                SdrUShortCont* pPts=pM->GetMarkedGluePoints();
                if (pPts!=nullptr) {
                    pPts->clear();
                }
            }
        }
    }

    // at least reset the remembered BoundRect to prevent handle
    // generation if bForceFrameHandles is TRUE.
    mbMarkedObjRectDirty = true;
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
    if (bOn!=mbForceFrameHandles) {
        bool bOld=ImpIsFrameHandles();
        mbForceFrameHandles=bOn;
        bool bNew=ImpIsFrameHandles();
        if (bNew!=bOld) {
            AdjustMarkHdl();
            MarkListHasChanged();
        }
    }
}

void SdrMarkView::SetEditMode(SdrViewEditMode eMode)
{
    if (eMode!=meEditMode) {
        bool bGlue0=meEditMode==SDREDITMODE_GLUEPOINTEDIT;
        bool bEdge0=static_cast<SdrCreateView*>(this)->IsEdgeTool();
        meEditMode0=meEditMode;
        meEditMode=eMode;
        bool bGlue1=meEditMode==SDREDITMODE_GLUEPOINTEDIT;
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
            (!mbDesignMode && pObj->IsUnoObj()))
        {
            // object not selectable or
            // SdrUnoObj not in DesignMode
            return false;
        }
    }
    return pPV==nullptr || pPV->IsObjMarkable(pObj);
}

bool SdrMarkView::IsMarkedObjHit(const Point& rPnt, short nTol) const
{
    bool bRet=false;
    nTol=ImpGetHitTolLogic(nTol,nullptr);
    Point aPt(rPnt);
    for (size_t nm=0; nm<GetMarkedObjectCount() && !bRet; ++nm) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        bRet = nullptr != CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pM->GetPageView(),SdrSearchOptions::NONE,nullptr);
    }
    return bRet;
}

SdrHdl* SdrMarkView::PickHandle(const Point& rPnt, SdrSearchOptions nOptions, SdrHdl* pHdl0) const
{
    if (mbSomeObjChgdFlag) { // recalculate handles, if necessary
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
    nTol=ImpGetHitTolLogic(nTol,nullptr);
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
        OSL_ASSERT(pM!=nullptr);
        if (pM->GetMarkedSdrObj() != nullptr)
            nSearchObjNum = pM->GetMarkedSdrObj()->GetNavigationPosition();
    }

    SdrObject* pMarkObj=nullptr;
    SdrObjList* pSearchObjList=pPageView->GetObjList();
    const size_t nObjCount = pSearchObjList->GetObjCount();
    if (nObjCount!=0) {
        if (nSearchObjNum>nObjCount) nSearchObjNum=nObjCount;
        while (pMarkObj==nullptr && ((!bPrev && nSearchObjNum>0) || (bPrev && nSearchObjNum<nObjCount)))
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
    nTol=ImpGetHitTolLogic(nTol,nullptr);
    Point aPt(rPnt);
    SdrMark* pTopMarkHit=nullptr;
    SdrMark* pBtmMarkHit=nullptr;
    size_t nTopMarkHit=0;
    size_t nBtmMarkHit=0;
    // find topmost of the selected objects that is hit by rPnt
    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nm=nMarkCount; nm>0 && pTopMarkHit==nullptr;) {
        --nm;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        if(CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pM->GetPageView(),SdrSearchOptions::NONE,nullptr))
        {
            pTopMarkHit=pM;
            nTopMarkHit=nm;
        }
    }
    // nothing found, in this case, just select an object
    if (pTopMarkHit==nullptr) return MarkObj(rPnt,sal_uInt16(nTol));

    SdrObject* pTopObjHit=pTopMarkHit->GetMarkedSdrObj();
    SdrObjList* pObjList=pTopObjHit->GetObjList();
    SdrPageView* pPV=pTopMarkHit->GetPageView();
    // find lowermost of the selected objects that is hit by rPnt
    // and is placed on the same PageView as pTopMarkHit
    for (size_t nm=0; nm<nMarkCount && pBtmMarkHit==nullptr; ++nm) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrPageView* pPV2=pM->GetPageView();
        if (pPV2==pPV && CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pPV2,SdrSearchOptions::NONE,nullptr))
        {
            pBtmMarkHit=pM;
            nBtmMarkHit=nm;
        }
    }
    if (pBtmMarkHit==nullptr) { pBtmMarkHit=pTopMarkHit; nBtmMarkHit=nTopMarkHit; }
    SdrObject* pBtmObjHit=pBtmMarkHit->GetMarkedSdrObj();
    const size_t nObjCount = pObjList->GetObjCount();

    size_t nSearchBeg = 0;
    E3dScene* pScene = nullptr;
    SdrObject* pObjHit = (bPrev) ? pBtmObjHit : pTopObjHit;
    bool bRemap = dynamic_cast< const E3dCompoundObject* >(pObjHit) !=  nullptr
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
    SdrObject* pFndObj=nullptr;
    while (pFndObj==nullptr && ((!bPrev && no>0) || (bPrev && no<nObjCount))) {
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

        if (CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pObj,pPV,SdrSearchOptions::TESTMARKABLE,nullptr))
        {
            if (TryToFindMarkedObject(pObj)==SAL_MAX_SIZE) {
                pFndObj=pObj;
            } else {
                // TODO: for performance reasons set on to Top or Btm, if necessary
            }
        }
        if (bPrev) no++;
    }
    if (pFndObj!=nullptr)
    {
        GetMarkedObjectListWriteAccess().DeleteMark(bPrev?nBtmMarkHit:nTopMarkHit);
        GetMarkedObjectListWriteAccess().InsertEntry(SdrMark(pFndObj,pPV));
        MarkListHasChanged();
        AdjustMarkHdl();
    }
    return pFndObj!=nullptr;
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
        const size_t nObjCount = pObjList->GetObjCount();
        for (size_t nO=0; nO<nObjCount; ++nO) {
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
    if (pObj!=nullptr && pPV!=nullptr && IsObjMarkable(pObj, pPV)) {
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
        return nullptr;
    }

    const bool bCheckIfMarkable(nOptions & SdrSearchOptions::TESTMARKABLE);
    const bool bDeep(nOptions & SdrSearchOptions::DEEP);
    const bool bOLE(dynamic_cast< const SdrOle2Obj* >(pObj) !=  nullptr);
    const bool bTXT(dynamic_cast<const SdrTextObj*>( pObj) != nullptr && static_cast<SdrTextObj*>(pObj)->IsTextFrame());
    SdrObject* pRet=nullptr;
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

            if (pOL!=nullptr && pOL->GetObjCount()!=0)
            {
                SdrObject* pTmpObj;
                // adjustment hit point for virtual objects
                Point aPnt( rPnt );

                if ( dynamic_cast<const SdrVirtObj*>( pObj) !=  nullptr )
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

    if (!bDeep && pRet!=nullptr)
    {
        pRet=pObj;
    }

    return pRet;
}

SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList* pOL, SdrPageView* pPV, SdrSearchOptions nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj) const
{
    return (*this).CheckSingleSdrObjectHit(rPnt,nTol,pOL,pPV,nOptions,pMVisLay,rpRootObj,nullptr);
}
SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList* pOL, SdrPageView* pPV, SdrSearchOptions nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj,const SdrMarkList * pMarkList) const
{
    SdrObject* pRet=nullptr;
    rpRootObj=nullptr;
    if (pOL!=nullptr)
    {
        bool bBack(nOptions & SdrSearchOptions::BACKWARD);
        bool bRemap(pOL->GetOwnerObj() && dynamic_cast< const E3dScene* >(pOL->GetOwnerObj()) != nullptr);
        E3dScene* pRemapScene = (bRemap ? static_cast<E3dScene*>(pOL->GetOwnerObj()) : nullptr);

        const size_t nObjCount=pOL->GetObjCount();
        size_t nObjNum=bBack ? 0 : nObjCount;
        while (pRet==nullptr && (bBack ? nObjNum<nObjCount : nObjNum>0)) {
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
            if (nOptions & SdrSearchOptions::BEFOREMARK)
            {
                if ((pMarkList)!=nullptr)
                {
                    if ((*pMarkList).FindObject(pObj)!=SAL_MAX_SIZE)
                    {
                        return nullptr;
                    }
                }
            }
            pRet=CheckSingleSdrObjectHit(rPnt,nTol,pObj,pPV,nOptions,pMVisLay);
            if (pRet!=nullptr) rpRootObj=pObj;
            if (bBack) nObjNum++;
        }
    }
    return pRet;
}

bool SdrMarkView::PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, SdrSearchOptions nOptions) const
{
    return PickObj(rPnt,nTol,rpObj,rpPV,nOptions,nullptr);
}

bool SdrMarkView::PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, SdrSearchOptions nOptions, SdrObject** ppRootObj, bool* pbHitPassDirect) const
{ // TODO: lacks a Pass2,Pass3
    SortMarkedObjects();
    if (ppRootObj!=nullptr) *ppRootObj=nullptr;
    if (pbHitPassDirect!=nullptr) *pbHitPassDirect=true;
    rpObj=nullptr;
    rpPV=nullptr;
    bool bWholePage(nOptions & SdrSearchOptions::WHOLEPAGE);
    bool bMarked(nOptions & SdrSearchOptions::MARKED);
    bool bMasters=!bMarked && bool(nOptions & SdrSearchOptions::ALSOONMASTER);
    bool bBack(nOptions & SdrSearchOptions::BACKWARD);
#if OSL_DEBUG_LEVEL > 0
    bool bNext(nOptions & SdrSearchOptions::NEXT); (void)bNext; // n.i.
    bool bBoundCheckOn2ndPass(nOptions & SdrSearchOptions::PASS2BOUND); (void)bBoundCheckOn2ndPass;// n.i.
    bool bCheckNearestOn3rdPass(nOptions & SdrSearchOptions::PASS3NEAREST); (void)bCheckNearestOn3rdPass;// n.i.
#endif
    if (nTol<0) nTol=ImpGetHitTolLogic(nTol,nullptr);
    Point aPt(rPnt);
    SdrObject* pObj=nullptr;
    SdrObject* pHitObj=nullptr;
    SdrPageView* pPV=nullptr;
    if (!bBack && static_cast<const SdrObjEditView*>(this)->IsTextEditFrameHit(rPnt)) {
        pObj=static_cast<const SdrObjEditView*>(this)->GetTextEditObject();
        pHitObj=pObj;
        pPV=static_cast<const SdrObjEditView*>(this)->GetTextEditPageView();
    }
    if (bMarked) {
        const size_t nMrkAnz=GetMarkedObjectCount();
        size_t nMrkNum=bBack ? 0 : nMrkAnz;
        while (pHitObj==nullptr && (bBack ? nMrkNum<nMrkAnz : nMrkNum>0)) {
            if (!bBack) nMrkNum--;
            SdrMark* pM=GetSdrMarkByIndex(nMrkNum);
            pObj=pM->GetMarkedSdrObj();
            pPV=pM->GetPageView();
            pHitObj=CheckSingleSdrObjectHit(aPt,nTol,pObj,pPV,nOptions,nullptr);
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
            while (pHitObj==nullptr && (bBack ? nPgNum<nPgAnz : nPgNum>0)) {
                SdrSearchOptions nTmpOptions=nOptions;
                if (!bBack) nPgNum--;
                const SetOfByte* pMVisLay=nullptr;
                SdrObjList* pObjList=nullptr;
                if (pbHitPassDirect!=nullptr) *pbHitPassDirect = true;
                if (nPgNum>=nPgAnz-1 || (bExtraPassForWholePage && nPgNum>=nPgAnz-2))
                {
                    pObjList=pPV->GetObjList();
                    if (bExtraPassForWholePage && nPgNum==nPgAnz-2) {
                        pObjList=pPage;
                        if (pbHitPassDirect!=nullptr) *pbHitPassDirect = false;
                    }
                }
                else
                {
                    // otherwise MasterPage
                    SdrPage& rMasterPage = pPage->TRG_GetMasterPage();
                    pMVisLay = &pPage->TRG_GetMasterPageVisibleLayers();
                    pObjList = &rMasterPage;

                    if (pbHitPassDirect!=nullptr) *pbHitPassDirect = false;
                    nTmpOptions=nTmpOptions | SdrSearchOptions::IMPISMASTER;
                }
                pHitObj=CheckSingleSdrObjectHit(aPt,nTol,pObjList,pPV,nTmpOptions,pMVisLay,pObj,&(GetMarkedObjectList()));
                if (bBack) nPgNum++;
            }
        }
    }
    if (pHitObj!=nullptr) {
        if (ppRootObj!=nullptr) *ppRootObj=pObj;
        if (nOptions & SdrSearchOptions::DEEP) pObj=pHitObj;
        if (nOptions & SdrSearchOptions::TESTTEXTEDIT) {
            if (!pObj->HasTextEdit() || pPV->GetLockedLayers().IsSet(pObj->GetLayer())) {
                pObj=nullptr;
            }
        }
        if (pObj!=nullptr && (nOptions & SdrSearchOptions::TESTMACRO)) {
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aPt;
            aHitRec.aDownPos=aPt;
            aHitRec.nTol=nTol;
            aHitRec.pVisiLayer=&pPV->GetVisibleLayers();
            aHitRec.pPageView=pPV;
            if (!pObj->HasMacro() || !pObj->IsMacroHit(aHitRec)) pObj=nullptr;
        }
        if (pObj!=nullptr && (nOptions & SdrSearchOptions::WITHTEXT) && pObj->GetOutlinerParaObject()==nullptr) pObj=nullptr;
        if (pObj!=nullptr && (nOptions & SdrSearchOptions::TESTTEXTAREA) && pPV)
        {
            if(!SdrObjectPrimitiveHit(*pObj, aPt, 0, *pPV, nullptr, true))
            {
                pObj = nullptr;
            }
        }
        if (pObj!=nullptr) {
            rpObj=pObj;
            rpPV=pPV;
        }
    }
    return rpObj!=nullptr;
}

bool SdrMarkView::PickMarkedObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, SdrSearchOptions nOptions) const
{
    SortMarkedObjects();
    bool bBoundCheckOn2ndPass(nOptions & SdrSearchOptions::PASS2BOUND);
    bool bCheckNearestOn3rdPass(nOptions & SdrSearchOptions::PASS3NEAREST);
    rpObj=nullptr;
    rpPV=nullptr;
    Point aPt(rPnt);
    sal_uInt16 nTol=(sal_uInt16)mnHitTolLog;
    bool bFnd=false;
    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nMarkNum=nMarkCount; nMarkNum>0 && !bFnd;) {
        --nMarkNum;
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        SdrPageView* pPV=pM->GetPageView();
        SdrObject* pObj=pM->GetMarkedSdrObj();
        bFnd = nullptr != CheckSingleSdrObjectHit(aPt,nTol,pObj,pPV,SdrSearchOptions::TESTMARKABLE,nullptr);
        if (bFnd) {
            rpObj=pObj;
            rpPV=pPV;
        }
    }
    if ((bBoundCheckOn2ndPass || bCheckNearestOn3rdPass) && !bFnd) {
        SdrObject* pBestObj=nullptr;
        SdrPageView* pBestPV=nullptr;
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
            bFnd=pBestObj!=nullptr;
        }
    }
    return bFnd;
}


void SdrMarkView::UnmarkAllObj(SdrPageView* pPV)
{
    if (GetMarkedObjectCount()!=0) {
        BrkAction();
        if (pPV!=nullptr)
        {
            GetMarkedObjectListWriteAccess().DeletePageView(*pPV);
        }
        else
        {
            GetMarkedObjectListWriteAccess().Clear();
        }
        mpMarkedObj=nullptr;
        mpMarkedPV=nullptr;
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
    aOffset =   aGroupSyncedRect.TopLeft() - maMarkedObjRectNoOffset.TopLeft();
    return aOffset;
}

const Rectangle& SdrMarkView::GetMarkedObjRect() const
{
    if (mbMarkedObjRectDirty) {
        const_cast<SdrMarkView*>(this)->mbMarkedObjRectDirty=false;
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
        const_cast<SdrMarkView*>(this)->maMarkedObjRect=aRect;
        const_cast<SdrMarkView*>(this)->maMarkedObjRectNoOffset=aRect2;
    }
    return maMarkedObjRect;
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

    mbMarkedObjRectDirty=true;
    mbMarkedPointsRectsDirty=true;
#ifdef DBG_UTIL
    if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
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
    if ( mbDesignMode != _bOn )
    {
        mbDesignMode = _bOn;
        SdrPageView* pPageView = GetSdrPageView();
        if ( pPageView )
            pPageView->SetDesignMode( _bOn );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
