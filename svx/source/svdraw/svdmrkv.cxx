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
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdomedia.hxx>

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/strbuf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include "gradtrns.hxx"
#include <svx/xflftrit.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdundo.hxx>
#include <svx/svdopath.hxx>
#include <svx/scene3d.hxx>
#include <svx/svdovirt.hxx>
#include <sdr/overlay/overlayrollingrectangle.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <vcl/window.hxx>
#include <o3tl/string_view.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/lokcomponenthelpers.hxx>
#include <sfx2/viewsh.hxx>
#include <svtools/optionsdrawinglayer.hxx>

#include <drawinglayer/processor2d/textextractor2d.hxx>

#include <array>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <boost/property_tree/json_parser.hpp>

using namespace com::sun::star;

// Migrate Marking of Objects, Points and GluePoints

class ImplMarkingOverlay
{
    // The OverlayObjects
    sdr::overlay::OverlayObjectList               maObjects;

    // The remembered second position in logical coordinates
    basegfx::B2DPoint                               maSecondPosition;

    // A flag to remember if the action is for unmarking.
    bool                                            mbUnmarking : 1;

public:
    ImplMarkingOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos, bool bUnmarking);

    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.

    void SetSecondPosition(const basegfx::B2DPoint& rNewPosition);
    bool IsUnmarking() const { return mbUnmarking; }
};

ImplMarkingOverlay::ImplMarkingOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos, bool bUnmarking)
:   maSecondPosition(rStartPos),
    mbUnmarking(bUnmarking)
{
    if (comphelper::LibreOfficeKit::isActive())
        return; // We do client-side object manipulation with the Kit API

    for(sal_uInt32 a(0); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        const rtl::Reference< sdr::overlay::OverlayManager >& xTargetOverlay = pCandidate->GetOverlayManager();

        if (xTargetOverlay.is())
        {
            std::unique_ptr<sdr::overlay::OverlayRollingRectangleStriped> pNew(new sdr::overlay::OverlayRollingRectangleStriped(
                rStartPos, rStartPos, false));
            xTargetOverlay->add(*pNew);
            maObjects.append(std::move(pNew));
        }
    }
}

void ImplMarkingOverlay::SetSecondPosition(const basegfx::B2DPoint& rNewPosition)
{
    if(rNewPosition != maSecondPosition)
    {
        // apply to OverlayObjects
        for(sal_uInt32 a(0); a < maObjects.count(); a++)
        {
            sdr::overlay::OverlayRollingRectangleStriped& rCandidate = static_cast< sdr::overlay::OverlayRollingRectangleStriped&>(maObjects.getOverlayObject(a));
            rCandidate.setSecondPosition(rNewPosition);
        }

        // remember new position
        maSecondPosition = rNewPosition;
    }
}

class MarkingSelectionOverlay
{
    sdr::overlay::OverlayObjectList maObjects;
public:
    MarkingSelectionOverlay(const SdrPaintView& rView, basegfx::B2DRectangle const& rSelection)
    {
        if (comphelper::LibreOfficeKit::isActive())
            return; // We do client-side object manipulation with the Kit API

        for (sal_uInt32 a(0); a < rView.PaintWindowCount(); a++)
        {
            SdrPaintWindow* pPaintWindow = rView.GetPaintWindow(a);
            const rtl::Reference<sdr::overlay::OverlayManager>& xTargetOverlay = pPaintWindow->GetOverlayManager();

            if (xTargetOverlay.is())
            {
                basegfx::B2DPolyPolygon aPolyPoly(basegfx::utils::createPolygonFromRect(rSelection));
                auto pNew = std::make_unique<sdr::overlay::OverlayPolyPolygon>(aPolyPoly, COL_GRAY, 0, COL_TRANSPARENT);
                xTargetOverlay->add(*pNew);
                maObjects.append(std::move(pNew));
            }
        }
    }
};

class MarkingSubSelectionOverlay
{
    sdr::overlay::OverlayObjectList maObjects;

public:
    MarkingSubSelectionOverlay(const SdrPaintView& rView, std::vector<basegfx::B2DRectangle> const & rSelections)
    {
        if (comphelper::LibreOfficeKit::isActive())
            return; // We do client-side object manipulation with the Kit API

        for (sal_uInt32 a(0); a < rView.PaintWindowCount(); a++)
        {
            SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
            const rtl::Reference<sdr::overlay::OverlayManager>& xTargetOverlay = pCandidate->GetOverlayManager();

            if (xTargetOverlay.is())
            {
                const Color aHighlightColor = SvtOptionsDrawinglayer::getHilightColor();

                std::unique_ptr<sdr::overlay::OverlaySelection> pNew =
                    std::make_unique<sdr::overlay::OverlaySelection>(
                        sdr::overlay::OverlayType::Transparent,
                        aHighlightColor, std::vector(rSelections), false);

                xTargetOverlay->add(*pNew);
                maObjects.append(std::move(pNew));
            }
        }
    }
};

SdrMarkView::SdrMarkView(SdrModel& rSdrModel, OutputDevice* pOut)
    : SdrSnapView(rSdrModel, pOut)
    , mpMarkedObj(nullptr)
    , mpMarkedPV(nullptr)
    , maHdlList(this)
    , meDragMode(SdrDragMode::Move)
    , meEditMode(SdrViewEditMode::Edit)
    , meEditMode0(SdrViewEditMode::Edit)
    , mbDesignMode(false)
    , mbForceFrameHandles(false)
    , mbPlusHdlAlways(false)
    , mbInsPolyPoint(false)
    , mbMarkedObjRectDirty(false)
    , mbMrkPntDirty(false)
    , mbMarkedPointsRectsDirty(false)
    , mbMarkHandlesHidden(false)
    , mbNegativeX(false)
{

    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();

    StartListening(rSdrModel);
}

SdrMarkView::~SdrMarkView()
{
    // Migrate selections
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

void SdrMarkView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::ThisIsAnSdrHint)
    {
        const SdrHint* pSdrHint = static_cast<const SdrHint*>(&rHint);
        SdrHintKind eKind=pSdrHint->GetKind();
        if (eKind==SdrHintKind::ObjectChange || eKind==SdrHintKind::ObjectInserted || eKind==SdrHintKind::ObjectRemoved)
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
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    rMarkList.ForceSort();
    mbMrkPntDirty=true;
    UndirtyMrkPnt();
    SdrView* pV=static_cast<SdrView*>(this);
    if (pV!=nullptr && !pV->IsDragObj() && !pV->IsInsObjPoint()) {
        AdjustMarkHdl();
    }

    if (comphelper::LibreOfficeKit::isActive())
        modelHasChangedLOKit();
}

void SdrMarkView::modelHasChangedLOKit()
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if (rMarkList.GetMarkCount() <= 0)
        return;

    //TODO: Is MarkedObjRect valid at this point?
    tools::Rectangle aSelection(GetMarkedObjRect());
    tools::Rectangle* pResultSelection;
    if (aSelection.IsEmpty())
        pResultSelection = nullptr;
    else
    {
        sal_uInt32 nTotalPaintWindows = this->PaintWindowCount();
        if (nTotalPaintWindows == 1)
        {
            const OutputDevice* pOut = this->GetFirstOutputDevice();
            const vcl::Window* pWin = pOut ? pOut->GetOwnerWindow() : nullptr;
            if (pWin && pWin->IsChart())
            {
                if (SfxViewShell* pViewShell = GetSfxViewShell())
                {
                    const vcl::Window* pViewShellWindow = pViewShell->GetEditWindowForActiveOLEObj();
                    if (pViewShellWindow && pViewShellWindow->IsAncestorOf(*pWin))
                    {
                        Point aOffsetPx = pWin->GetOffsetPixelFrom(*pViewShellWindow);
                        Point aLogicOffset = pWin->PixelToLogic(aOffsetPx);
                        aSelection.Move(aLogicOffset.getX(), aLogicOffset.getY());
                    }
                }
            }
        }

        // In case the map mode is in 100th MM, then need to convert the coordinates over to twips for LOK.
        if (mpMarkedPV)
        {
            if (OutputDevice* pOutputDevice = mpMarkedPV->GetView().GetFirstOutputDevice())
            {
                if (pOutputDevice->GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
                    aSelection = o3tl::convert(aSelection, o3tl::Length::mm100, o3tl::Length::twip);
            }
        }

        pResultSelection = &aSelection;

        if (mbNegativeX)
        {
            // Convert to positive X doc-coordinates
            tools::Long nTmp = aSelection.Left();
            aSelection.SetLeft(-aSelection.Right());
            aSelection.SetRight(-nTmp);
        }
    }

    if (SfxViewShell* pViewShell = GetSfxViewShell())
        SfxLokHelper::notifyInvalidation(pViewShell, pResultSelection);
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

void SdrMarkView::TakeActionRect(tools::Rectangle& rRect) const
{
    if(IsMarkObj() || IsMarkPoints() || IsMarkGluePoints())
    {
        rRect = tools::Rectangle(maDragStat.GetStart(), maDragStat.GetNow());
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


void SdrMarkView::BegMarkObj(const Point& rPnt, bool bUnmark)
{
    BrkAction();

    DBG_ASSERT(!mpMarkObjOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkObjOverlay (!)");

    basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
    mpMarkObjOverlay.reset(new ImplMarkingOverlay(*this, aStartPos, bUnmark));

    maDragStat.Reset(rPnt);
    maDragStat.NextPoint();
    maDragStat.SetMinMove(mnMinMovLog);
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
            tools::Rectangle aRect(maDragStat.GetStart(), maDragStat.GetNow());
            aRect.Normalize();
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
        mpMarkObjOverlay.reset();
    }
}


bool SdrMarkView::BegMarkPoints(const Point& rPnt, bool bUnmark)
{
    if(HasMarkablePoints())
    {
        BrkAction();

        DBG_ASSERT(!mpMarkPointsOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkPointsOverlay (!)");
        basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
        mpMarkPointsOverlay.reset(new ImplMarkingOverlay(*this, aStartPos, bUnmark));

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
            tools::Rectangle aRect(maDragStat.GetStart(), maDragStat.GetNow());
            aRect.Normalize();
            MarkPoints(&aRect, mpMarkPointsOverlay->IsUnmarking());

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
        mpMarkPointsOverlay.reset();
    }
}


bool SdrMarkView::BegMarkGluePoints(const Point& rPnt, bool bUnmark)
{
    if(HasMarkableGluePoints())
    {
        BrkAction();

        DBG_ASSERT(!mpMarkGluePointsOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkGluePointsOverlay (!)");

        basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
        mpMarkGluePointsOverlay.reset(new ImplMarkingOverlay(*this, aStartPos, bUnmark));
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

void SdrMarkView::EndMarkGluePoints()
{
    if(IsMarkGluePoints())
    {
        if(maDragStat.IsMinMoved())
        {
            tools::Rectangle aRect(maDragStat.GetStart(),maDragStat.GetNow());
            aRect.Normalize();
            MarkGluePoints(&aRect, mpMarkGluePointsOverlay->IsUnmarking());
        }

        // cleanup
        BrkMarkGluePoints();
    }
}

void SdrMarkView::BrkMarkGluePoints()
{
    if(IsMarkGluePoints())
    {
        DBG_ASSERT(mpMarkGluePointsOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        mpMarkGluePointsOverlay.reset();
    }
}

bool SdrMarkView::MarkableObjectsExceed( int n ) const
{
    SdrPageView* pPV = GetSdrPageView();
    if (!pPV)
        return false;

    SdrObjList* pOL=pPV->GetObjList();
    for (const rtl::Reference<SdrObject>& pObj : *pOL)
        if (IsObjMarkable(pObj.get(),pPV) && --n<0)
            return true;

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
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    const size_t nMarkCount=rMarkList.GetMarkCount();
    bool bFrmHdl=nMarkCount>static_cast<size_t>(mnFrameHandlesLimit) || mbForceFrameHandles;
    bool bStdDrag=meDragMode==SdrDragMode::Move;
    if (nMarkCount==1 && bStdDrag && bFrmHdl)
    {
        const SdrObject* pObj=rMarkList.GetMark(0)->GetMarkedSdrObj();
        if (pObj && pObj->GetObjInventor()==SdrInventor::Default)
        {
            SdrObjKind nIdent=pObj->GetObjIdentifier();
            if (nIdent==SdrObjKind::Line || nIdent==SdrObjKind::Edge || nIdent==SdrObjKind::Caption || nIdent==SdrObjKind::Measure || nIdent==SdrObjKind::CustomShape || nIdent==SdrObjKind::Table )
            {
                bFrmHdl=false;
            }
        }
    }
    if (!bStdDrag && !bFrmHdl) {
        // all other drag modes only with FrameHandles
        bFrmHdl=true;
        if (meDragMode==SdrDragMode::Rotate) {
            // when rotating, use ObjOwn drag, if there's at least 1 PolyObj
            for (size_t nMarkNum=0; nMarkNum<nMarkCount && bFrmHdl; ++nMarkNum) {
                const SdrMark* pM=rMarkList.GetMark(nMarkNum);
                const SdrObject* pObj=pM->GetMarkedSdrObj();
                bFrmHdl=!pObj->IsPolyObj();
            }
        }
    }
    if (!bFrmHdl) {
        // FrameHandles, if at least 1 Obj can't do SpecialDrag
        for (size_t nMarkNum=0; nMarkNum<nMarkCount && !bFrmHdl; ++nMarkNum) {
            const SdrMark* pM=rMarkList.GetMark(nMarkNum);
            const SdrObject* pObj=pM->GetMarkedSdrObj();
            bFrmHdl=!pObj->hasSpecialDrag();
        }
    }

    // no FrameHdl for crop
    if(bFrmHdl && SdrDragMode::Crop == meDragMode)
    {
        bFrmHdl = false;
    }

    return bFrmHdl;
}

namespace
{
std::u16string_view lcl_getDragMethodServiceName( std::u16string_view rCID )
{
    std::u16string_view aRet;

    size_t nIndexStart = rCID.find( u"DragMethod=" );
    if( nIndexStart != std::u16string_view::npos )
    {
        nIndexStart = rCID.find( '=', nIndexStart );
        if( nIndexStart != std::u16string_view::npos )
        {
            nIndexStart++;
            size_t nNextSlash = rCID.find( '/', nIndexStart );
            if( nNextSlash != std::u16string_view::npos )
            {
                sal_Int32 nIndexEnd = nNextSlash;
                size_t nNextColon = rCID.find( ':', nIndexStart );
                if( nNextColon == std::u16string_view::npos || nNextColon < nNextSlash )
                    nIndexEnd = nNextColon;
                aRet = rCID.substr(nIndexStart,nIndexEnd-nIndexStart);
            }
        }
    }
    return aRet;
}

std::u16string_view lcl_getDragParameterString( std::u16string_view rCID )
{
    std::u16string_view aRet;

    size_t nIndexStart = rCID.find( u"DragParameter=" );
    if( nIndexStart != std::u16string_view::npos )
    {
        nIndexStart = rCID.find( '=', nIndexStart );
        if( nIndexStart != std::u16string_view::npos )
        {
            nIndexStart++;
            size_t nNextSlash = rCID.find( '/', nIndexStart );
            if( nNextSlash != std::u16string_view::npos )
            {
                sal_Int32 nIndexEnd = nNextSlash;
                size_t nNextColon = rCID.find( ':', nIndexStart );
                if( nNextColon == std::u16string_view::npos || nNextColon < nNextSlash )
                    nIndexEnd = nNextColon;
                aRet = rCID.substr(nIndexStart,nIndexEnd-nIndexStart);
            }
        }
    }
    return aRet;
}
} // anonymous namespace

bool SdrMarkView::dumpGluePointsToJSON(boost::property_tree::ptree& rTree)
{
    bool result = false;
    tools::Long nSignX = mbNegativeX ? -1 : 1;
    if (OutputDevice* pOutDev = mpMarkedPV ? mpMarkedPV->GetView().GetFirstOutputDevice() : nullptr)
    {
        bool bConvertUnit = false;
        if (pOutDev->GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
            bConvertUnit = true;
        const SdrObjList* pOL = mpMarkedPV->GetObjList();
        if (!pOL)
            return false;
        boost::property_tree::ptree elements;
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        for (const rtl::Reference<SdrObject>& pObj : *pOL)
        {
            if (!pObj)
                continue;
            if (pObj == rMarkList.GetMark(0)->GetMarkedSdrObj())
                continue;
            const SdrGluePointList* pGPL = pObj->GetGluePointList();
            bool VertexObject = !(pGPL && pGPL->GetCount());
            const size_t count = !VertexObject ? pGPL->GetCount() : 4;
            boost::property_tree::ptree object;
            boost::property_tree::ptree points;
            for (size_t i = 0; i < count; ++i)
            {
                boost::property_tree::ptree node;
                boost::property_tree::ptree point;
                const SdrGluePoint& rGP = !VertexObject ? (*pGPL)[i] : pObj->GetVertexGluePoint(i);
                Point rPoint = rGP.GetAbsolutePos(*pObj);
                if (bConvertUnit)
                {
                    rPoint = o3tl::convert(rPoint, o3tl::Length::mm100, o3tl::Length::twip);
                }
                point.put("x", nSignX * rPoint.getX());
                point.put("y", rPoint.getY());
                node.add_child("point", point);
                points.push_back(std::make_pair("", node));
            }
            basegfx::B2DVector aGridOffset(0.0, 0.0);
            Point objLogicRectTopLeft = pObj->GetLogicRect().TopLeft();
            if(getPossibleGridOffsetForPosition(aGridOffset, basegfx::B2DPoint(objLogicRectTopLeft.X(), objLogicRectTopLeft.Y()), GetSdrPageView()))
            {
                Point p(aGridOffset.getX(), aGridOffset.getY());
                if (bConvertUnit)
                {
                    p = o3tl::convert(p, o3tl::Length::mm100, o3tl::Length::twip);
                }
                boost::property_tree::ptree gridOffset;
                gridOffset.put("x", nSignX * p.getX());
                gridOffset.put("y", p.getY());
                object.add_child("gridoffset", gridOffset);
            }
            object.put("ordnum", pObj->GetOrdNum());
            object.add_child("gluepoints", points);
            elements.push_back(std::make_pair("", object));
            result = true;
        }
        rTree.add_child("shapes", elements);
    }
    return result;
}

namespace
{
    class TextBoundsExtractor final : public drawinglayer::processor2d::TextExtractor2D
    {
    private:
        basegfx::B2DRange maTextRange;
        void processTextPrimitive2D(const drawinglayer::primitive2d::BasePrimitive2D& rCandidate) override
        {
            maTextRange.expand(rCandidate.getB2DRange(getViewInformation2D()));
        }
    public:
        explicit TextBoundsExtractor(const drawinglayer::geometry::ViewInformation2D& rViewInformation)
            : drawinglayer::processor2d::TextExtractor2D(rViewInformation)
        {
        }

        basegfx::B2DRange getTextBounds(const sdr::contact::ViewObjectContact &rVOC, sdr::contact::DisplayInfo &raDisplayInfo)
        {
            this->process(rVOC.getPrimitive2DSequence(raDisplayInfo));
            return maTextRange;
        }
    };
}

OString SdrMarkView::CreateInnerTextRectString() const
{
    if (!mpMarkedObj)
        return OString();

    SdrPageView* pPageView = GetSdrPageView();
    const sdr::contact::ViewObjectContact& rVOC = mpMarkedObj->GetViewContact().GetViewObjectContact(
        pPageView->GetPageWindow(0)->GetObjectContact());

    sdr::contact::DisplayInfo aDisplayInfo;
    TextBoundsExtractor aTextBoundsExtractor(rVOC.GetObjectContact().getViewInformation2D());
    basegfx::B2DRange aRange = aTextBoundsExtractor.getTextBounds(rVOC, aDisplayInfo);
    if (!aRange.isEmpty()) {
        tools::Rectangle rect(aRange.getMinX(), aRange.getMinY(), aRange.getMaxX(), aRange.getMaxY());
        tools::Rectangle aRangeTWIP = o3tl::convert(rect, o3tl::Length::mm100, o3tl::Length::twip);
        OString innerTextInfo = "\"innerTextRect\":[" +
            OString::number(aRangeTWIP.getX()) + "," +
            OString::number(aRangeTWIP.getY()) + "," +
            OString::number(aRangeTWIP.GetWidth()) + "," +
            OString::number(aRangeTWIP.GetHeight()) + "]";
        return innerTextInfo;
    }

    return OString();
}

void SdrMarkView::SetInnerTextAreaForLOKit() const
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;
    SfxViewShell* pViewShell = GetSfxViewShell();
    OString sRectString = CreateInnerTextRectString();
    if (pViewShell && !sRectString.isEmpty())
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_SHAPE_INNER_TEXT, sRectString);
}

void SdrMarkView::SetMarkHandlesForLOKit(tools::Rectangle const & rRect, const SfxViewShell* pOtherShell)
{
    SfxViewShell* pViewShell = GetSfxViewShell();

    tools::Rectangle aSelection(rRect);
    tools::Long nSignX = mbNegativeX ? -1 : 1;
    bool bIsChart = false;
    Point addLogicOffset(0, 0);
    bool convertMapMode = false;
    if (!rRect.IsEmpty())
    {
        sal_uInt32 nTotalPaintWindows = this->PaintWindowCount();
        if (nTotalPaintWindows == 1)
        {
            const OutputDevice* pOut = this->GetFirstOutputDevice();
            const vcl::Window* pWin = pOut ? pOut->GetOwnerWindow() : nullptr;
            if (pWin && pWin->IsChart())
            {
                bIsChart = true;
                const vcl::Window* pViewShellWindow = GetSfxViewShell()->GetEditWindowForActiveOLEObj();
                if (pViewShellWindow && pViewShellWindow->IsAncestorOf(*pWin))
                {
                    Point aOffsetPx = pWin->GetOffsetPixelFrom(*pViewShellWindow);
                    if (mbNegativeX && AllSettings::GetLayoutRTL())
                    {
                        // mbNegativeX is set only for Calc in RTL mode.
                        // If global RTL flag is set, vcl-window X offset of chart window is
                        // mirrored w.r.t parent window rectangle. This needs to be reverted.
                        aOffsetPx.setX(pViewShellWindow->GetOutOffXPixel() + pViewShellWindow->GetSizePixel().Width()
                            - pWin->GetOutOffXPixel() - pWin->GetSizePixel().Width());
                    }
                    Point aLogicOffset = pWin->PixelToLogic(aOffsetPx);
                    addLogicOffset = aLogicOffset;
                    aSelection.Move(aLogicOffset.getX(), aLogicOffset.getY());
                }
            }
        }
    }

    if (!aSelection.IsEmpty())
    {
        // In case the map mode is in 100th MM, then need to convert the coordinates over to twips for LOK.
        if (mpMarkedPV)
        {
            if (OutputDevice* pOutputDevice = mpMarkedPV->GetView().GetFirstOutputDevice())
            {
                if (pOutputDevice->GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
                {
                    aSelection = o3tl::convert(aSelection, o3tl::Length::mm100, o3tl::Length::twip);
                    convertMapMode = true;
                }
            }
        }

        // hide the text selection too
        if (pViewShell)
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, ""_ostr);
    }

    {
        OStringBuffer aExtraInfo;
        OString sSelectionText;
        OString sSelectionTextView;
        boost::property_tree::ptree aTableJsonTree;
        boost::property_tree::ptree aGluePointsTree;
        const bool bMediaObj = (mpMarkedObj && mpMarkedObj->GetObjIdentifier() == SdrObjKind::Media);
        bool bTableSelection = false;
        bool bConnectorSelection = false;

        if (mpMarkedObj && mpMarkedObj->GetObjIdentifier() == SdrObjKind::Table)
        {
            auto& rTableObject = dynamic_cast<sdr::table::SdrTableObj&>(*mpMarkedObj);
            bTableSelection = rTableObject.createTableEdgesJson(aTableJsonTree);
        }
        if (mpMarkedObj && mpMarkedObj->GetObjIdentifier() == SdrObjKind::Edge)
        {
            bConnectorSelection = dumpGluePointsToJSON(aGluePointsTree);
        }

        SdrPageView* pPageView = GetSdrPageView();

        const SdrMarkList& rMarkList = GetMarkedObjectList();
        if (rMarkList.GetMarkCount())
        {
            SdrMark* pM = rMarkList.GetMark(0);
            SdrObject* pO = pM->GetMarkedSdrObj();
            Degree100 nRotAngle = pO->GetRotateAngle();
            // true if we are dealing with a RotGrfFlyFrame
            // (SwVirtFlyDrawObj with a SwGrfNode)
            bool bWriterGraphic = pO->HasLimitedRotation();

            OString handleArrayStr;

            aExtraInfo.append("{\"id\":\""
                + OString::number(reinterpret_cast<sal_IntPtr>(pO))
                + "\",\"type\":"
                + OString::number(static_cast<sal_Int32>(pO->GetObjIdentifier())));

            if (mpMarkedObj && !pOtherShell)
            {
                OString innerTextInfo = CreateInnerTextRectString();
                if (!innerTextInfo.isEmpty())
                    aExtraInfo.append("," + innerTextInfo);
            }

            // In core, the gridOffset is calculated based on the LogicRect's TopLeft coordinate
            // In online, we have the SnapRect and we calculate it based on its TopLeft coordinate
            // SnapRect's TopLeft and LogicRect's TopLeft match unless there is rotation
            // but the rotation is not applied to the LogicRect. Therefore,
            // what we calculate in online does not match with the core in case of the rotation.
            // Here we can send the correct gridOffset in the selection callback, this way
            // whether the shape is rotated or not, we will always have the correct gridOffset
            // Note that the gridOffset is calculated from the first selected obj
            basegfx::B2DVector aGridOffset(0.0, 0.0);
            if(getPossibleGridOffsetForSdrObject(aGridOffset, rMarkList.GetMark(0)->GetMarkedSdrObj(), pPageView))
            {
                Point p(aGridOffset.getX(), aGridOffset.getY());
                if (convertMapMode)
                    p = o3tl::convert(p, o3tl::Length::mm100, o3tl::Length::twip);
                aExtraInfo.append(",\"gridOffsetX\":"
                    + OString::number(nSignX * p.getX())
                    + ",\"gridOffsetY\":"
                    + OString::number(p.getY()));
            }

            if (bWriterGraphic)
            {
                aExtraInfo.append(", \"isWriterGraphic\": true");
            }
            else if (bIsChart)
            {
                LokChartHelper aChartHelper(pViewShell);
                css::uno::Reference<css::frame::XController>& xChartController = aChartHelper.GetXController();
                css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier( xChartController, uno::UNO_QUERY);
                if (xSelectionSupplier.is())
                {
                    uno::Any aSel = xSelectionSupplier->getSelection();
                    OUString aValue;
                    if (aSel >>= aValue)
                    {
                        OString aObjectCID(aValue.getStr(), aValue.getLength(), osl_getThreadTextEncoding());
                        const std::vector<OString> aProps{"Draggable"_ostr, "Resizable"_ostr, "Rotatable"_ostr};
                        for (const auto& rProp: aProps)
                        {
                            sal_Int32 nPos = aObjectCID.indexOf(rProp);
                            if (nPos == -1) continue;
                            nPos += rProp.getLength() + 1; // '='
                            if (aExtraInfo.getLength() > 2) // != "{ "
                                aExtraInfo.append(", ");
                            aExtraInfo.append("\"is" + rProp + "\": "
                                + OString::boolean(aObjectCID[nPos] == '1'));
                        }

                        std::u16string_view sDragMethod = lcl_getDragMethodServiceName(aValue);
                        if (sDragMethod == u"PieSegmentDragging")
                        {
                            // old initial offset inside the CID returned by xSelectionSupplier->getSelection()
                            // after a pie segment dragging; using SdrObject::GetName for getting a CID with the updated offset
                            aValue = pO->GetName();
                            std::u16string_view sDragParameters = lcl_getDragParameterString(aValue);
                            if (!sDragParameters.empty())
                            {
                                aExtraInfo.append(", \"dragInfo\": { "
                                    "\"dragMethod\": \""
                                    + OUString(sDragMethod).toUtf8()
                                    + "\"");

                                sal_Int32 nStartIndex = 0;
                                std::array<int, 5> aDragParameters;
                                for (auto& rParam : aDragParameters)
                                {
                                    std::u16string_view sParam = o3tl::getToken(sDragParameters, 0, ',', nStartIndex);
                                    if (sParam.empty())
                                        break;
                                    rParam = o3tl::toInt32(sParam);
                                }

                                // initial offset in %
                                if (aDragParameters[0] < 0)
                                    aDragParameters[0] = 0;
                                else if (aDragParameters[0] > 100)
                                    aDragParameters[0] = 100;

                                aExtraInfo.append(", \"initialOffset\": "
                                    + OString::number(static_cast<sal_Int32>(aDragParameters[0])));

                                // drag direction constraint
                                Point aMinPos(aDragParameters[1], aDragParameters[2]);
                                Point aMaxPos(aDragParameters[3], aDragParameters[4]);
                                Point aDragDirection = aMaxPos - aMinPos;
                                aDragDirection = o3tl::convert(aDragDirection, o3tl::Length::mm100, o3tl::Length::twip);

                                aExtraInfo.append(", \"dragDirection\": ["
                                    + aDragDirection.toString()
                                    + "]");

                                // polygon approximating the pie segment or donut segment
                                if (pViewShell && pO->GetObjIdentifier() == SdrObjKind::PathFill)
                                {
                                    const basegfx::B2DPolyPolygon aPolyPolygon(pO->TakeXorPoly());
                                    if (aPolyPolygon.count() == 1)
                                    {
                                        const basegfx::B2DPolygon aPolygon = aPolyPolygon.getB2DPolygon(0);
                                        if (sal_uInt32 nPolySize = aPolygon.count())
                                        {
                                            const OutputDevice* pOut = this->GetFirstOutputDevice();
                                            const vcl::Window* pWin = pOut ? pOut->GetOwnerWindow() : nullptr;
                                            const vcl::Window* pViewShellWindow = pViewShell->GetEditWindowForActiveOLEObj();
                                            if (pWin && pViewShellWindow && pViewShellWindow->IsAncestorOf(*pWin))
                                            {
                                                // in the following code escaping sequences used inside raw literal strings
                                                // are for making them understandable by the JSON parser

                                                Point aOffsetPx = pWin->GetOffsetPixelFrom(*pViewShellWindow);
                                                Point aLogicOffset = pWin->PixelToLogic(aOffsetPx);
                                                OString sPolygonElem("<polygon points=\\\""_ostr);
                                                for (sal_uInt32 nIndex = 0; nIndex < nPolySize; ++nIndex)
                                                {
                                                    const basegfx::B2DPoint aB2Point = aPolygon.getB2DPoint(nIndex);
                                                    Point aPoint(aB2Point.getX(), aB2Point.getY());
                                                    aPoint.Move(aLogicOffset.getX(), aLogicOffset.getY());
                                                    if (mbNegativeX)
                                                        aPoint.setX(-aPoint.X());
                                                    if (nIndex > 0)
                                                        sPolygonElem += " ";
                                                    sPolygonElem += aPoint.toString();
                                                }
                                                sPolygonElem += R"elem(\" style=\"stroke: none; fill: rgb(114,159,207); fill-opacity: 0.8\"/>)elem";

                                                OString sSVGElem = R"elem(<svg version=\"1.2\" width=\")elem" +
                                                    OString::number(aSelection.GetWidth() / 100.0) +
                                                    R"elem(mm\" height=\")elem" +
                                                    OString::number(aSelection.GetHeight() / 100.0) +
                                                    R"elem(mm\" viewBox=\")elem" +
                                                    aSelection.toString() +
                                                    R"elem(\" preserveAspectRatio=\"xMidYMid\" xmlns=\"http://www.w3.org/2000/svg\">)elem";

                                                aExtraInfo.append(", \"svg\": \""
                                                    + sSVGElem
                                                    + "\\n  "
                                                    + sPolygonElem
                                                    + "\\n</svg>"
                                                    "\""); // svg
                                            }
                                        }
                                    }
                                }
                                aExtraInfo.append("}"); // dragInfo
                            }
                        }
                    }
                }
            }
            if (!bTableSelection && !pOtherShell && maHdlList.GetHdlCount())
            {
                boost::property_tree::ptree responseJSON;
                boost::property_tree::ptree others;
                boost::property_tree::ptree anchor;
                boost::property_tree::ptree rectangle;
                boost::property_tree::ptree poly;
                boost::property_tree::ptree custom;
                boost::property_tree::ptree nodes;
                for (size_t i = 0; i < maHdlList.GetHdlCount(); i++)
                {
                    SdrHdl *pHdl = maHdlList.GetHdl(i);
                    boost::property_tree::ptree child;
                    boost::property_tree::ptree point;
                    sal_Int32 kind = static_cast<sal_Int32>(pHdl->GetKind());
                    child.put("id", pHdl->GetObjHdlNum());
                    child.put("kind", kind);
                    child.put("pointer", static_cast<sal_Int32>(pHdl->GetPointer()));
                    Point pHdlPos = pHdl->GetPos();
                    pHdlPos.Move(addLogicOffset.getX(), addLogicOffset.getY());
                    if (convertMapMode)
                    {
                        pHdlPos = o3tl::convert(pHdlPos, o3tl::Length::mm100, o3tl::Length::twip);
                    }
                    point.put("x", pHdlPos.getX());
                    point.put("y", pHdlPos.getY());
                    child.add_child("point", point);
                    const auto node = std::make_pair("", child);
                    boost::property_tree::ptree* selectedNode = nullptr;
                    if (kind >= static_cast<sal_Int32>(SdrHdlKind::UpperLeft) && kind <= static_cast<sal_Int32>(SdrHdlKind::LowerRight))
                    {
                        selectedNode = &rectangle;
                    }
                    else if (kind == static_cast<sal_Int32>(SdrHdlKind::Poly))
                    {
                        selectedNode = &poly;
                    }
                    else if (kind == static_cast<sal_Int32>(SdrHdlKind::CustomShape1))
                    {
                        selectedNode = &custom;
                    }
                    else if (kind == static_cast<sal_Int32>(SdrHdlKind::Anchor) || kind == static_cast<sal_Int32>(SdrHdlKind::Anchor_TR))
                    {
                        if (getSdrModelFromSdrView().IsWriter())
                            selectedNode = &anchor;
                        else
                            // put it to others as we don't render them except in writer
                            selectedNode = &others;
                    }
                    else
                    {
                        selectedNode = &others;
                    }
                    std::string sKind = std::to_string(kind);
                    boost::optional< boost::property_tree::ptree& > kindNode = selectedNode->get_child_optional(sKind.c_str());
                    if (!kindNode)
                    {
                        boost::property_tree::ptree newChild;
                        newChild.push_back(node);
                        selectedNode->add_child(sKind.c_str(), newChild);
                    }
                    else
                        kindNode.get().push_back(node);
                }
                nodes.add_child("rectangle", rectangle);
                nodes.add_child("poly", poly);
                nodes.add_child("custom", custom);
                nodes.add_child("anchor", anchor);
                nodes.add_child("others", others);
                responseJSON.add_child("kinds", nodes);
                std::stringstream aStream;
                boost::property_tree::write_json(aStream, responseJSON, /*pretty=*/ false);
                handleArrayStr = ", \"handles\":"_ostr;
                handleArrayStr = handleArrayStr + aStream.str().c_str();
                if (bConnectorSelection)
                {
                    aStream.str("");
                    boost::property_tree::write_json(aStream, aGluePointsTree, /*pretty=*/ false);
                    handleArrayStr = handleArrayStr + ", \"GluePoints\":";
                    handleArrayStr = handleArrayStr + aStream.str().c_str();
                }
            }

            if (mbNegativeX)
            {
                tools::Rectangle aNegatedRect(aSelection);
                aNegatedRect.SetLeft(-aNegatedRect.Left());
                aNegatedRect.SetRight(-aNegatedRect.Right());
                aNegatedRect.Normalize();
                sSelectionText = aNegatedRect.toString() +
                    ", " + OString::number(nRotAngle.get());
            }
            else
            {
                sSelectionText = aSelection.toString() +
                    ", " + OString::number(nRotAngle.get());
            }

            if (!aExtraInfo.isEmpty())
            {
                sSelectionTextView = sSelectionText + ", " + aExtraInfo + "}";

                if (bMediaObj && pOtherShell == nullptr)
                {
                    // Add the URL only if we have a Media Object and
                    // we are the selecting view.
                    SdrMediaObj* mediaObj = dynamic_cast<SdrMediaObj*>(mpMarkedObj);
                    if (mediaObj)
                        aExtraInfo.append(", \"url\": \"" + mediaObj->getTempURL().toUtf8() + "\"");
                }

                aExtraInfo.append(handleArrayStr
                    + "}");
                sSelectionText += ", " + aExtraInfo;
            }
        }

        if (sSelectionText.isEmpty())
        {
            sSelectionText = "EMPTY"_ostr;
            sSelectionTextView = "EMPTY"_ostr;
            if (!pOtherShell && pViewShell)
                pViewShell->NotifyOtherViews(LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection"_ostr, OString());
        }

        if (bTableSelection)
        {
            boost::property_tree::ptree aTableRectangle;
            aTableRectangle.put("x", aSelection.Left());
            aTableRectangle.put("y", aSelection.Top());
            aTableRectangle.put("width", aSelection.GetWidth());
            aTableRectangle.put("height", aSelection.GetHeight());
            aTableJsonTree.push_back(std::make_pair("rectangle", aTableRectangle));

            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTableJsonTree);
            if (pViewShell)
                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TABLE_SELECTED, OString(aStream.str()));
        }
        else if (!getSdrModelFromSdrView().IsWriter() && pViewShell)
        {
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TABLE_SELECTED, "{}"_ostr);
        }

        if (pOtherShell)
        {
            // Another shell wants to know about our existing
            // selection.
            if (pViewShell != pOtherShell)
                SfxLokHelper::notifyOtherView(pViewShell, pOtherShell, LOK_CALLBACK_GRAPHIC_VIEW_SELECTION, "selection", sSelectionTextView);
        }
        else
        {
            // We have a new selection, so both pViewShell and the
            // other views want to know about it.
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_GRAPHIC_SELECTION, sSelectionText);

            SfxLokHelper::notifyOtherViews(pViewShell, LOK_CALLBACK_GRAPHIC_VIEW_SELECTION, "selection", sSelectionTextView);
        }
    }
}

void SdrMarkView::SetMarkHandles(SfxViewShell* pOtherShell)
{
    // remember old focus handle values to search for it again
    const SdrHdl* pSaveOldFocusHdl = maHdlList.GetFocusHdl();
    bool bSaveOldFocus(false);
    sal_uInt32 nSavePolyNum(0), nSavePointNum(0);
    SdrHdlKind eSaveKind(SdrHdlKind::Move);
    SdrObject* pSaveObj = nullptr;

    mpMarkingSubSelectionOverlay.reset();
    mpMarkingSelectionOverlay.reset();

    if(pSaveOldFocusHdl
        && pSaveOldFocusHdl->GetObj()
        && dynamic_cast<const SdrPathObj*>(pSaveOldFocusHdl->GetObj()) != nullptr
        && (pSaveOldFocusHdl->GetKind() == SdrHdlKind::Poly || pSaveOldFocusHdl->GetKind() == SdrHdlKind::BezierWeight))
    {
        bSaveOldFocus = true;
        nSavePolyNum = pSaveOldFocusHdl->GetPolyNum();
        nSavePointNum = pSaveOldFocusHdl->GetPointNum();
        pSaveObj = pSaveOldFocusHdl->GetObj();
        eSaveKind = pSaveOldFocusHdl->GetKind();
    }

    // delete/clear all handles. This will always be done, even with areMarkHandlesHidden()
    maHdlList.Clear();
    maHdlList.SetRotateShear(meDragMode==SdrDragMode::Rotate);
    maHdlList.SetDistortShear(meDragMode==SdrDragMode::Shear);
    mpMarkedObj=nullptr;
    mpMarkedPV=nullptr;

    // are handles enabled at all? Create only then
    if(areMarkHandlesHidden())
        return;

    // There can be multiple mark views, but we're only interested in the one that has a window associated.
    const bool bTiledRendering = comphelper::LibreOfficeKit::isActive() && GetFirstOutputDevice() && GetFirstOutputDevice()->GetOutDevType() == OUTDEV_WINDOW;

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    const size_t nMarkCount=rMarkList.GetMarkCount();
    bool bStdDrag=meDragMode==SdrDragMode::Move;
    bool bSingleTextObjMark=false;
    bool bLimitedRotation(false);

    if (nMarkCount==1)
    {
        mpMarkedObj=rMarkList.GetMark(0)->GetMarkedSdrObj();

        if(nullptr != mpMarkedObj)
        {
            bSingleTextObjMark =
                DynCastSdrTextObj( mpMarkedObj) !=  nullptr &&
                static_cast<SdrTextObj*>(mpMarkedObj)->IsTextFrame();

            // RotGrfFlyFrame: we may have limited rotation
            bLimitedRotation = SdrDragMode::Rotate == meDragMode && mpMarkedObj->HasLimitedRotation();
        }
    }

    bool bFrmHdl=ImpIsFrameHandles();

    if (nMarkCount>0)
    {
        mpMarkedPV=rMarkList.GetMark(0)->GetPageView();

        for (size_t nMarkNum=0; nMarkNum<nMarkCount && (mpMarkedPV!=nullptr || !bFrmHdl); ++nMarkNum)
        {
            const SdrMark* pM=rMarkList.GetMark(nMarkNum);

            if (mpMarkedPV!=pM->GetPageView())
            {
                mpMarkedPV=nullptr;
            }
        }
    }

    tools::Rectangle aRect(GetMarkedObjRect());

    if (mpMarkedObj && mpMarkedObj->GetObjIdentifier() == SdrObjKind::Annotation)
    {
        basegfx::B2DRectangle aB2DRect(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
        mpMarkingSelectionOverlay = std::make_unique<MarkingSelectionOverlay>(*this, aB2DRect);

        return;

    }

    SfxViewShell* pViewShell = GetSfxViewShell();

    // check if text edit or ole is active and handles need to be suppressed. This may be the case
    // when a single object is selected
    // Using a strict return statement is okay here; no handles means *no* handles.
    if (mpMarkedObj)
    {
        // formerly #i33755#: If TextEdit is active the EditEngine will directly paint
        // to the window, so suppress Overlay and handles completely; a text frame for
        // the active text edit will be painted by the repaint mechanism in
        // SdrObjEditView::ImpPaintOutlinerView in this case. This needs to be reworked
        // in the future
        // Also formerly #122142#: Pretty much the same for SdrCaptionObj's in calc.
        if(static_cast<SdrView*>(this)->IsTextEdit())
        {
            const SdrTextObj* pSdrTextObj = DynCastSdrTextObj(mpMarkedObj);

            if (pSdrTextObj && pSdrTextObj->IsInEditMode())
            {
                if (!bTiledRendering)
                    return;
            }
        }

        // formerly #i118524#: if inplace activated OLE is selected, suppress handles
        const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(mpMarkedObj);

        if(pSdrOle2Obj && (pSdrOle2Obj->isInplaceActive() || pSdrOle2Obj->isUiActive()))
        {
            return;
        }

        if (!maSubSelectionList.empty())
        {
            mpMarkingSubSelectionOverlay = std::make_unique<MarkingSubSelectionOverlay>(*this, maSubSelectionList);
        }
    }

    if (bFrmHdl)
    {
        if(!aRect.IsEmpty())
        {
            // otherwise nothing is found
            const size_t nSiz0(maHdlList.GetHdlCount());

            if( bSingleTextObjMark )
            {
                mpMarkedObj->AddToHdlList(maHdlList);
            }
            else
            {
                const bool bWdt0(aRect.Left() == aRect.Right());
                const bool bHgt0(aRect.Top() == aRect.Bottom());

                if (bWdt0 && bHgt0)
                {
                    maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.TopLeft(), SdrHdlKind::UpperLeft));
                }
                else if (!bStdDrag && (bWdt0 || bHgt0))
                {
                    maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.TopLeft(), SdrHdlKind::UpperLeft));
                    maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.BottomRight(), SdrHdlKind::LowerRight));
                }
                else
                {
                    if (!bWdt0 && !bHgt0)
                    {
                        maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.TopLeft(), SdrHdlKind::UpperLeft));
                    }

                    if (!bLimitedRotation && !bHgt0)
                    {
                        maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.TopCenter(), SdrHdlKind::Upper));
                    }

                    if (!bWdt0 && !bHgt0)
                    {
                        maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.TopRight(), SdrHdlKind::UpperRight));
                    }

                    if (!bLimitedRotation && !bWdt0)
                    {
                        maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.LeftCenter(), SdrHdlKind::Left ));
                        maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.RightCenter(), SdrHdlKind::Right));
                    }

                    if (!bWdt0 && !bHgt0)
                    {
                        maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.BottomLeft(), SdrHdlKind::LowerLeft));
                    }

                    if (!bLimitedRotation && !bHgt0)
                    {
                        maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.BottomCenter(), SdrHdlKind::Lower));
                    }

                    if (!bWdt0 && !bHgt0)
                    {
                        maHdlList.AddHdl(std::make_unique<SdrHdl>(aRect.BottomRight(), SdrHdlKind::LowerRight));
                    }
                }
            }

            // Diagram selection visualization support
            // Caution: CppunitTest_sd_tiledrendering shows that mpMarkedObj *can* actually be nullptr (!)
            if(nullptr != mpMarkedObj && mpMarkedObj->isDiagram())
            {
                mpMarkedObj->AddToHdlList(maHdlList);
            }

            const size_t nSiz1(maHdlList.GetHdlCount());

            // moved setting the missing parameters at SdrHdl here from the
            // single loop above (bSingleTextObjMark), this was missing all
            // the time. Setting SdrObject is now required to correctly get
            // the View-Dependent evtl. GridOffset adapted
            for (size_t i=nSiz0; i<nSiz1; ++i)
            {
                SdrHdl* pHdl=maHdlList.GetHdl(i);
                pHdl->SetObj(mpMarkedObj);
                pHdl->SetPageView(mpMarkedPV);
                pHdl->SetObjHdlNum(sal_uInt16(i-nSiz0));
            }
        }
    }
    else
    {
        bool bDone(false);

        // moved crop handling to non-frame part and the handle creation to SdrGrafObj
        if(1 == nMarkCount && mpMarkedObj && SdrDragMode::Crop == meDragMode)
        {
            // Default addCropHandles from SdrObject does nothing. When pMarkedObj is SdrGrafObj, previous
            // behaviour occurs (code in svx/source/svdraw/svdograf.cxx). When pMarkedObj is SwVirtFlyDrawObj
            // writer takes the responsibility of adding handles (code in sw/source/core/draw/dflyobj.cxx)
            const size_t nSiz0(maHdlList.GetHdlCount());
            mpMarkedObj->addCropHandles(maHdlList);
            const size_t nSiz1(maHdlList.GetHdlCount());

            // Was missing: Set infos at SdrCropHdl
            for (size_t i=nSiz0; i<nSiz1; ++i)
            {
                SdrHdl* pHdl=maHdlList.GetHdl(i);
                pHdl->SetObj(mpMarkedObj);
                pHdl->SetPageView(mpMarkedPV);
                pHdl->SetObjHdlNum(sal_uInt16(i-nSiz0));
            }

            bDone = true;
        }

        if(!bDone)
        {
            for (size_t nMarkNum=0; nMarkNum<nMarkCount; ++nMarkNum)
            {
                const SdrMark* pM=rMarkList.GetMark(nMarkNum);
                SdrObject* pObj=pM->GetMarkedSdrObj();
                SdrPageView* pPV=pM->GetPageView();
                const size_t nSiz0=maHdlList.GetHdlCount();
                pObj->AddToHdlList(maHdlList);
                const size_t nSiz1=maHdlList.GetHdlCount();
                bool bPoly=pObj->IsPolyObj();
                const SdrUShortCont& rMrkPnts = pM->GetMarkedPoints();
                for (size_t i=nSiz0; i<nSiz1; ++i)
                {
                    SdrHdl* pHdl=maHdlList.GetHdl(i);
                    pHdl->SetObj(pObj);
                    pHdl->SetPageView(pPV);
                    pHdl->SetObjHdlNum(sal_uInt16(i-nSiz0));

                    if (bPoly)
                    {
                        bool bSelected= rMrkPnts.find( sal_uInt16(i-nSiz0) ) != rMrkPnts.end();
                        pHdl->SetSelected(bSelected);
                        if (mbPlusHdlAlways || bSelected)
                        {
                            SdrHdlList plusList(nullptr);
                            pObj->AddToPlusHdlList(plusList, *pHdl);
                            sal_uInt32 nPlusHdlCnt=plusList.GetHdlCount();
                            for (sal_uInt32 nPlusNum=0; nPlusNum<nPlusHdlCnt; nPlusNum++)
                            {
                                SdrHdl* pPlusHdl=plusList.GetHdl(nPlusNum);
                                pPlusHdl->SetObj(pObj);
                                pPlusHdl->SetPageView(pPV);
                                pPlusHdl->SetPlusHdl(true);
                            }
                            plusList.MoveTo(maHdlList);
                        }
                    }
                }
            }
        }
    }

    // GluePoint handles
    for (size_t nMarkNum=0; nMarkNum<nMarkCount; ++nMarkNum)
    {
        const SdrMark* pM=rMarkList.GetMark(nMarkNum);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        if (!pGPL)
            continue;

        SdrPageView* pPV=pM->GetPageView();
        const SdrUShortCont& rMrkGlue=pM->GetMarkedGluePoints();
        for (sal_uInt16 nId : rMrkGlue)
        {
            //nNum changed to nNumGP because already used in for loop
            sal_uInt16 nNumGP=pGPL->FindGluePoint(nId);
            if (nNumGP!=SDRGLUEPOINT_NOTFOUND)
            {
                const SdrGluePoint& rGP=(*pGPL)[nNumGP];
                Point aPos(rGP.GetAbsolutePos(*pObj));
                std::unique_ptr<SdrHdl> pGlueHdl(new SdrHdl(aPos,SdrHdlKind::Glue));
                pGlueHdl->SetObj(pObj);
                pGlueHdl->SetPageView(pPV);
                pGlueHdl->SetObjHdlNum(nId);
                maHdlList.AddHdl(std::move(pGlueHdl));
            }
        }
    }

    // rotation point/axis of reflection
    if(!bLimitedRotation)
    {
        AddDragModeHdl(meDragMode);
    }

    // sort handles
    maHdlList.Sort();

    // add custom handles (used by other apps, e.g. AnchorPos)
    AddCustomHdl();

    // moved it here to access all the handles for callback.
    if (bTiledRendering && pViewShell)
    {
        SetMarkHandlesForLOKit(aRect, pOtherShell);
    }

    // try to restore focus handle index from remembered values
    if(!bSaveOldFocus)
        return;

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

void SdrMarkView::AddCustomHdl()
{
    // add custom handles (used by other apps, e.g. AnchorPos)
}

void SdrMarkView::SetDragMode(SdrDragMode eMode)
{
    SdrDragMode eMode0=meDragMode;
    meDragMode=eMode;
    if (meDragMode==SdrDragMode::Resize) meDragMode=SdrDragMode::Move;
    if (meDragMode!=eMode0) {
        ForceRefToMarked();
        SetMarkHandles(nullptr);
        {
            const SdrMarkList& rMarkList = GetMarkedObjectList();
            if (rMarkList.GetMarkCount() != 0) MarkListHasChanged();
        }
    }
}

void SdrMarkView::AddDragModeHdl(SdrDragMode eMode)
{
    switch(eMode)
    {
        case SdrDragMode::Rotate:
        {
            // add rotation center
            maHdlList.AddHdl(std::make_unique<SdrHdl>(maRef1, SdrHdlKind::Ref1));
            break;
        }
        case SdrDragMode::Mirror:
        {
            // add axis of reflection
            std::unique_ptr<SdrHdl> pHdl3(new SdrHdl(maRef2, SdrHdlKind::Ref2));
            std::unique_ptr<SdrHdl> pHdl2(new SdrHdl(maRef1, SdrHdlKind::Ref1));
            std::unique_ptr<SdrHdl> pHdl1(new SdrHdlLine(*pHdl2, *pHdl3, SdrHdlKind::MirrorAxis));

            pHdl1->SetObjHdlNum(1); // for sorting
            pHdl2->SetObjHdlNum(2); // for sorting
            pHdl3->SetObjHdlNum(3); // for sorting

            maHdlList.AddHdl(std::move(pHdl1)); // line comes first, so it is the last in HitTest
            maHdlList.AddHdl(std::move(pHdl2));
            maHdlList.AddHdl(std::move(pHdl3));

            break;
        }
        case SdrDragMode::Transparence:
        {
            // add interactive transparency handle
            const SdrMarkList& rMarkList = GetMarkedObjectList();
            const size_t nMarkCount = rMarkList.GetMarkCount();
            if(nMarkCount == 1)
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                SdrModel& rModel = GetModel();
                const SfxItemSet& rSet = pObj->GetMergedItemSet();

                if(SfxItemState::SET != rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, false))
                {
                    // add this item, it's not yet there
                    XFillFloatTransparenceItem aNewItem(rSet.Get(XATTR_FILLFLOATTRANSPARENCE));
                    basegfx::BGradient aGrad = aNewItem.GetGradientValue();

                    aNewItem.SetEnabled(true);
                    aGrad.SetStartIntens(100);
                    aGrad.SetEndIntens(100);
                    aNewItem.SetGradientValue(aGrad);

                    // add undo to allow user to take back this step
                    if (rModel.IsUndoEnabled())
                    {
                        rModel.BegUndo(SvxResId(SIP_XA_FILLTRANSPARENCE));
                        rModel.AddUndo(rModel.GetSdrUndoFactory().CreateUndoAttrObject(*pObj));
                        rModel.EndUndo();
                    }

                    SfxItemSet aNewSet(rModel.GetItemPool());
                    aNewSet.Put(aNewItem);
                    pObj->SetMergedItemSetAndBroadcast(aNewSet);
                }

                // set values and transform to vector set
                GradTransVector aGradTransVector;
                GradTransGradient aGradTransGradient;

                aGradTransGradient.aGradient = rSet.Get(XATTR_FILLFLOATTRANSPARENCE).GetGradientValue();
                GradTransformer::GradToVec(aGradTransGradient, aGradTransVector, pObj);

                // build handles
                const Point aTmpPos1(basegfx::fround<tools::Long>(aGradTransVector.maPositionA.getX()), basegfx::fround<tools::Long>(aGradTransVector.maPositionA.getY()));
                const Point aTmpPos2(basegfx::fround<tools::Long>(aGradTransVector.maPositionB.getX()), basegfx::fround<tools::Long>(aGradTransVector.maPositionB.getY()));
                std::unique_ptr<SdrHdlColor> pColHdl1(new SdrHdlColor(aTmpPos1, aGradTransVector.aCol1, SDR_HANDLE_COLOR_SIZE_NORMAL, true));
                std::unique_ptr<SdrHdlColor> pColHdl2(new SdrHdlColor(aTmpPos2, aGradTransVector.aCol2, SDR_HANDLE_COLOR_SIZE_NORMAL, true));
                std::unique_ptr<SdrHdlGradient> pGradHdl(new SdrHdlGradient(aTmpPos1, aTmpPos2, false));
                DBG_ASSERT(pColHdl1 && pColHdl2 && pGradHdl, "Could not get all necessary handles!");

                // link them
                pGradHdl->SetColorHandles(pColHdl1.get(), pColHdl2.get());
                pGradHdl->SetObj(pObj);
                pColHdl1->SetColorChangeHdl(LINK(pGradHdl.get(), SdrHdlGradient, ColorChangeHdl));
                pColHdl2->SetColorChangeHdl(LINK(pGradHdl.get(), SdrHdlGradient, ColorChangeHdl));

                // insert them
                maHdlList.AddHdl(std::move(pColHdl1));
                maHdlList.AddHdl(std::move(pColHdl2));
                maHdlList.AddHdl(std::move(pGradHdl));
            }
            break;
        }
        case SdrDragMode::Gradient:
        {
            // add interactive gradient handle
            const SdrMarkList& rMarkList = GetMarkedObjectList();
            const size_t nMarkCount = rMarkList.GetMarkCount();
            if(nMarkCount == 1)
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                const SfxItemSet& rSet = pObj->GetMergedItemSet();
                drawing::FillStyle eFillStyle = rSet.Get(XATTR_FILLSTYLE).GetValue();

                if(eFillStyle == drawing::FillStyle_GRADIENT)
                {
                    // set values and transform to vector set
                    GradTransVector aGradTransVector;
                    GradTransGradient aGradTransGradient;
                    Size aHdlSize(15, 15);

                    aGradTransGradient.aGradient = rSet.Get(XATTR_FILLGRADIENT).GetGradientValue();
                    GradTransformer::GradToVec(aGradTransGradient, aGradTransVector, pObj);

                    // build handles
                    const Point aTmpPos1(basegfx::fround<tools::Long>(aGradTransVector.maPositionA.getX()), basegfx::fround<tools::Long>(aGradTransVector.maPositionA.getY()));
                    const Point aTmpPos2(basegfx::fround<tools::Long>(aGradTransVector.maPositionB.getX()), basegfx::fround<tools::Long>(aGradTransVector.maPositionB.getY()));
                    std::unique_ptr<SdrHdlColor> pColHdl1(new SdrHdlColor(aTmpPos1, aGradTransVector.aCol1, aHdlSize, false));
                    std::unique_ptr<SdrHdlColor> pColHdl2(new SdrHdlColor(aTmpPos2, aGradTransVector.aCol2, aHdlSize, false));
                    std::unique_ptr<SdrHdlGradient> pGradHdl(new SdrHdlGradient(aTmpPos1, aTmpPos2, true));
                    DBG_ASSERT(pColHdl1 && pColHdl2 && pGradHdl, "Could not get all necessary handles!");

                    // link them
                    pGradHdl->SetColorHandles(pColHdl1.get(), pColHdl2.get());
                    pGradHdl->SetObj(pObj);
                    pColHdl1->SetColorChangeHdl(LINK(pGradHdl.get(), SdrHdlGradient, ColorChangeHdl));
                    pColHdl2->SetColorChangeHdl(LINK(pGradHdl.get(), SdrHdlGradient, ColorChangeHdl));

                    // insert them
                    maHdlList.AddHdl(std::move(pColHdl1));
                    maHdlList.AddHdl(std::move(pColHdl2));
                    maHdlList.AddHdl(std::move(pGradHdl));
                }
            }
            break;
        }
        case SdrDragMode::Crop:
        {
            // TODO
            break;
        }
        default: break;
    }
}

/** handle mouse over effects for handles */
bool SdrMarkView::MouseMove(const MouseEvent& rMEvt, OutputDevice* pWin)
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

bool SdrMarkView::RequestHelp(const HelpEvent& rHEvt)
{
    if (maHdlList.GetHdlCount())
    {
        const size_t nHdlCount = maHdlList.GetHdlCount();

        for (size_t nHdl = 0; nHdl < nHdlCount; ++nHdl)
        {
            SdrHdl* pCurrentHdl = GetHdl(nHdl);
            if (pCurrentHdl->mbMouseOver)
            {
                pCurrentHdl->onHelpRequest();
                return true;
            }
        }
    }
    return SdrSnapView::RequestHelp(rHEvt);
}

void SdrMarkView::ForceRefToMarked()
{
    switch(meDragMode)
    {
        case SdrDragMode::Rotate:
        {
            tools::Rectangle aR(GetMarkedObjRect());
            maRef1 = aR.Center();

            break;
        }

        case SdrDragMode::Mirror:
        {
            // first calculate the length of the axis of reflection
            tools::Long nOutMin=0;
            tools::Long nOutMax=0;
            tools::Long nMinLen=0;
            tools::Long nObjDst=0;
            tools::Long nOutHgt=0;
            OutputDevice* pOut=GetFirstOutputDevice();
            if (pOut!=nullptr) {
                // minimum length: 50 pixels
                nMinLen=pOut->PixelToLogic(Size(0,50)).Height();
                // 20 pixels distance to the Obj for the reference point
                nObjDst=pOut->PixelToLogic(Size(0,20)).Height();
                // MinY/MaxY
                // margin = minimum length = 10 pixels
                tools::Long nDst=pOut->PixelToLogic(Size(0,10)).Height();
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
                tools::Long nTemp=nOutHgt/4;
                if (nTemp>nMinLen) nMinLen=nTemp;
            }

            tools::Rectangle aR(GetMarkedObjBoundRect());
            Point aCenter(aR.Center());
            tools::Long nMarkHgt=aR.GetHeight()-1;
            tools::Long nHgt=nMarkHgt+nObjDst*2;       // 20 pixels overlapping above and below
            if (nHgt<nMinLen) nHgt=nMinLen;     // minimum length 50 pixels or 1/4 OutHgt, respectively

            tools::Long nY1=aCenter.Y()-(nHgt+1)/2;
            tools::Long nY2=nY1+nHgt;

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

            maRef1.setX(aCenter.X() );
            maRef1.setY(nY1 );
            maRef2.setX(aCenter.X() );
            maRef2.setY(nY2 );

            break;
        }

        case SdrDragMode::Transparence:
        case SdrDragMode::Gradient:
        case SdrDragMode::Crop:
        {
            tools::Rectangle aRect(GetMarkedObjBoundRect());
            maRef1 = aRect.TopLeft();
            maRef2 = aRect.BottomRight();
            break;
        }
        default: break;
    }
}

void SdrMarkView::SetRef1(const Point& rPt)
{
    if(meDragMode == SdrDragMode::Rotate || meDragMode == SdrDragMode::Mirror)
    {
        maRef1 = rPt;
        SdrHdl* pH = maHdlList.GetHdl(SdrHdlKind::Ref1);
        if(pH)
            pH->SetPos(rPt);
    }
}

void SdrMarkView::SetRef2(const Point& rPt)
{
    if(meDragMode == SdrDragMode::Mirror)
    {
        maRef2 = rPt;
        SdrHdl* pH = maHdlList.GetHdl(SdrHdlKind::Ref2);
        if(pH)
            pH->SetPos(rPt);
    }
}

SfxViewShell* SdrMarkView::GetSfxViewShell() const
{
    return SfxViewShell::Current();
}

void SdrMarkView::CheckMarked()
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    for (size_t nm=rMarkList.GetMarkCount(); nm>0;) {
        --nm;
        SdrMark* pM = rMarkList.GetMark(nm);
        SdrObject* pObj = pM->GetMarkedSdrObj();
        SdrPageView* pPV = pM->GetPageView();
        bool bRaus = !pObj || !pPV->IsObjMarkable(pObj);
        if (bRaus)
        {
            GetMarkedObjectListWriteAccess().DeleteMark(nm);
        }
        else
        {
            if (!IsGluePointEditMode()) { // selected gluepoints only in GlueEditMode
                SdrUShortCont& rPts = pM->GetMarkedGluePoints();
                rPts.clear();
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
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        pPV->SetHasMarkedObj(rMarkList.TakeSnapRect(pPV, pPV->MarkSnap()));
        rMarkList.TakeBoundRect(pPV, pPV->MarkBound());
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
    if (eMode==meEditMode)        return;

    bool bGlue0=meEditMode==SdrViewEditMode::GluePointEdit;
    bool bEdge0=static_cast<SdrCreateView*>(this)->IsEdgeTool();
    meEditMode0=meEditMode;
    meEditMode=eMode;
    bool bGlue1=meEditMode==SdrViewEditMode::GluePointEdit;
    bool bEdge1=static_cast<SdrCreateView*>(this)->IsEdgeTool();
    // avoid flickering when switching between GlueEdit and EdgeTool
    if (bGlue1 && !bGlue0) ImpSetGlueVisible2(bGlue1);
    if (bEdge1!=bEdge0) ImpSetGlueVisible3(bEdge1);
    if (!bGlue1 && bGlue0) ImpSetGlueVisible2(bGlue1);
    if (bGlue0 && !bGlue1) UnmarkAllGluePoints();
}


bool SdrMarkView::IsObjMarkable(SdrObject const * pObj, SdrPageView const * pPV) const
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
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    for (size_t nm=0; nm<rMarkList.GetMarkCount() && !bRet; ++nm) {
        SdrMark* pM=rMarkList.GetMark(nm);
        bRet = nullptr != CheckSingleSdrObjectHit(rPnt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pM->GetPageView(),SdrSearchOptions::NONE,nullptr);
    }
    return bRet;
}

SdrHdl* SdrMarkView::PickHandle(const Point& rPnt) const
{
    if (mbSomeObjChgdFlag) { // recalculate handles, if necessary
        FlushComeBackTimer();
    }
    return maHdlList.IsHdlListHit(rPnt);
}

bool SdrMarkView::MarkObj(const Point& rPnt, short nTol, bool bToggle, bool bDeep)
{
    SdrPageView* pPV;
    nTol=ImpGetHitTolLogic(nTol,nullptr);
    SdrSearchOptions nOptions=SdrSearchOptions::PICKMARKABLE;
    if (bDeep) nOptions=nOptions|SdrSearchOptions::DEEP;
    SdrObject* pObj = PickObj(rPnt, static_cast<sal_uInt16>(nTol), pPV, nOptions);
    if (pObj) {
        bool bUnmark=bToggle && IsObjMarked(pObj);
        MarkObj(pObj,pPV,bUnmark);
    }
    return pObj != nullptr;
}

bool SdrMarkView::MarkNextObj(bool bPrev)
{
    SdrPageView* pPageView = GetSdrPageView();

    if(!pPageView)
    {
        return false;
    }

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    rMarkList.ForceSort();
    const size_t nMarkCount=rMarkList.GetMarkCount();
    size_t nChgMarkNum = SAL_MAX_SIZE; // number of the MarkEntry we want to replace
    size_t nSearchObjNum = bPrev ? 0 : SAL_MAX_SIZE;
    if (nMarkCount!=0) {
        nChgMarkNum=bPrev ? 0 : nMarkCount-1;
        SdrMark* pM=rMarkList.GetMark(nChgMarkNum);
        assert(pM != nullptr);
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
                if (rMarkList.FindObject(pSearchObj)==SAL_MAX_SIZE)
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
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    rMarkList.ForceSort();
    nTol=ImpGetHitTolLogic(nTol,nullptr);
    SdrMark* pTopMarkHit=nullptr;
    SdrMark* pBtmMarkHit=nullptr;
    size_t nTopMarkHit=0;
    size_t nBtmMarkHit=0;
    // find topmost of the selected objects that is hit by rPnt
    const size_t nMarkCount=rMarkList.GetMarkCount();
    for (size_t nm=nMarkCount; nm>0 && pTopMarkHit==nullptr;) {
        --nm;
        SdrMark* pM=rMarkList.GetMark(nm);
        if(CheckSingleSdrObjectHit(rPnt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pM->GetPageView(),SdrSearchOptions::NONE,nullptr))
        {
            pTopMarkHit=pM;
            nTopMarkHit=nm;
        }
    }
    // nothing found, in this case, just select an object
    if (pTopMarkHit==nullptr) return MarkObj(rPnt,sal_uInt16(nTol));

    SdrObject* pTopObjHit=pTopMarkHit->GetMarkedSdrObj();
    SdrObjList* pObjList=pTopObjHit->getParentSdrObjListFromSdrObject();
    SdrPageView* pPV=pTopMarkHit->GetPageView();
    // find lowermost of the selected objects that is hit by rPnt
    // and is placed on the same PageView as pTopMarkHit
    for (size_t nm=0; nm<nMarkCount && pBtmMarkHit==nullptr; ++nm) {
        SdrMark* pM=rMarkList.GetMark(nm);
        SdrPageView* pPV2=pM->GetPageView();
        if (pPV2==pPV && CheckSingleSdrObjectHit(rPnt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pPV2,SdrSearchOptions::NONE,nullptr))
        {
            pBtmMarkHit=pM;
            nBtmMarkHit=nm;
        }
    }
    if (pBtmMarkHit==nullptr) { pBtmMarkHit=pTopMarkHit; nBtmMarkHit=nTopMarkHit; }
    SdrObject* pBtmObjHit=pBtmMarkHit->GetMarkedSdrObj();
    const size_t nObjCount = pObjList->GetObjCount();

    size_t nSearchBeg(0);
    E3dScene* pScene(nullptr);
    SdrObject* pObjHit(bPrev ? pBtmObjHit : pTopObjHit);
    bool bRemap =
        nullptr != dynamic_cast< const E3dCompoundObject* >(pObjHit);
    if (bRemap)
    {
        pScene = DynCastE3dScene(pObjHit->getParentSdrObjectFromSdrObject());
        bRemap = nullptr != pScene;
    }

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

        if (CheckSingleSdrObjectHit(rPnt,sal_uInt16(nTol),pObj,pPV,SdrSearchOptions::TESTMARKABLE,nullptr))
        {
            if (rMarkList.FindObject(pObj)==SAL_MAX_SIZE) {
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

void SdrMarkView::MarkObj(const tools::Rectangle& rRect, bool bUnmark)
{
    bool bFnd=false;
    tools::Rectangle aR(rRect);
    SdrObjList* pObjList;
    BrkAction();
    SdrPageView* pPV = GetSdrPageView();

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if(pPV)
    {
        pObjList=pPV->GetObjList();
        tools::Rectangle aFrm1(aR);
        for (const rtl::Reference<SdrObject>& pObj : *pObjList) {
            tools::Rectangle aRect(pObj->GetCurrentBoundRect());
            if (aFrm1.Contains(aRect)) {
                if (!bUnmark) {
                    if (IsObjMarkable(pObj.get(),pPV))
                    {
                        GetMarkedObjectListWriteAccess().InsertEntry(SdrMark(pObj.get(),pPV));
                        bFnd=true;
                    }
                } else {
                    const size_t nPos=rMarkList.FindObject(pObj.get());
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
        rMarkList.ForceSort();
        MarkListHasChanged();
        AdjustMarkHdl();
    }
}

namespace {

void collectUIInformation(const SdrObject* pObj)
{
    EventDescription aDescription;
    aDescription.aAction = "SELECT";
    aDescription.aParent = "MainWindow";
    aDescription.aKeyWord = "CurrentApp";

    if (!pObj->GetName().isEmpty())
        aDescription.aParameters = {{"OBJECT", pObj->GetName()}};
    else
        aDescription.aParameters = {{"OBJECT", "Unnamed_Obj_" + OUString::number(pObj->GetOrdNum())}};

    UITestLogger::getInstance().logEvent(aDescription);
}

}

 void SdrMarkView::MarkObj(SdrObject* pObj, SdrPageView* pPV, bool bUnmark, bool bDoNoSetMarkHdl,
                          std::vector<basegfx::B2DRectangle> && rSubSelections)
{
    if (!(pObj!=nullptr && pPV!=nullptr && IsObjMarkable(pObj, pPV)))
        return;

    BrkAction();
    if (!bUnmark)
    {
        GetMarkedObjectListWriteAccess().InsertEntry(SdrMark(pObj,pPV));
        collectUIInformation(pObj);
    }
    else
    {
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        const size_t nPos=rMarkList.FindObject(pObj);
        if (nPos!=SAL_MAX_SIZE)
        {
            GetMarkedObjectListWriteAccess().DeleteMark(nPos);
        }
    }

    maSubSelectionList = std::move(rSubSelections);

    if (!bDoNoSetMarkHdl) {
        MarkListHasChanged();
        AdjustMarkHdl();
    }
}

bool SdrMarkView::IsObjMarked(SdrObject const * pObj) const
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    return rMarkList.FindObject(pObj)!=SAL_MAX_SIZE;
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

bool SdrMarkView::getPossibleGridOffsetForSdrObject(
    basegfx::B2DVector& rOffset,
    const SdrObject* pObj,
    const SdrPageView* pPV) const
{
    if(nullptr == pObj || nullptr == pPV)
    {
        return false;
    }

    const OutputDevice* pOutputDevice(GetFirstOutputDevice());

    if(nullptr == pOutputDevice)
    {
        return false;
    }

    const SdrPageWindow* pSdrPageWindow(pPV->FindPageWindow(*pOutputDevice));

    if(nullptr == pSdrPageWindow)
    {
        return false;
    }

    const sdr::contact::ObjectContact& rObjectContact(pSdrPageWindow->GetObjectContact());

    if(!rObjectContact.supportsGridOffsets())
    {
        return false;
    }

    const sdr::contact::ViewObjectContact& rVOC(pObj->GetViewContact().GetViewObjectContact(
        const_cast<sdr::contact::ObjectContact&>(rObjectContact)));

    rOffset = rVOC.getGridOffset();

    return !rOffset.equalZero();
}

bool SdrMarkView::getPossibleGridOffsetForPosition(
    basegfx::B2DVector& rOffset,
    const basegfx::B2DPoint& rPoint,
    const SdrPageView* pPV) const
{
    if(nullptr == pPV)
    {
        return false;
    }

    const OutputDevice* pOutputDevice(GetFirstOutputDevice());

    if(nullptr == pOutputDevice)
    {
        return false;
    }

    const SdrPageWindow* pSdrPageWindow(pPV->FindPageWindow(*pOutputDevice));

    if(nullptr == pSdrPageWindow)
    {
        return false;
    }

    const sdr::contact::ObjectContact& rObjectContact(pSdrPageWindow->GetObjectContact());

    if(!rObjectContact.supportsGridOffsets())
    {
        return false;
    }

    rObjectContact.calculateGridOffsetForB2DRange(rOffset, basegfx::B2DRange(rPoint));

    return !rOffset.equalZero();
}

SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, SdrSearchOptions nOptions, const SdrLayerIDSet* pMVisLay) const
{
    assert(pObj);
    if(((nOptions & SdrSearchOptions::IMPISMASTER) && pObj->IsNotVisibleAsMaster()) || (!pObj->IsVisible()))
    {
        return nullptr;
    }

    const bool bCheckIfMarkable(nOptions & SdrSearchOptions::TESTMARKABLE);
    const bool bDeep(nOptions & SdrSearchOptions::DEEP);
    const bool bOLE(dynamic_cast< const SdrOle2Obj* >(pObj) !=  nullptr);
    auto pTextObj = DynCastSdrTextObj( pObj);
    const bool bTXT(pTextObj && pTextObj->IsTextFrame());
    SdrObject* pRet=nullptr;
    tools::Rectangle aRect(pObj->GetCurrentBoundRect());

    // add possible GridOffset to up-to-now view-independent BoundRect data
    basegfx::B2DVector aGridOffset(0.0, 0.0);
    if(getPossibleGridOffsetForSdrObject(aGridOffset, pObj, pPV))
    {
        aRect += Point(
            basegfx::fround<tools::Long>(aGridOffset.getX()),
            basegfx::fround<tools::Long>(aGridOffset.getY()));
    }

    double nTol2(nTol);

    // double tolerance for OLE, text frames and objects in
    // active text edit
    if(bOLE || bTXT || pObj==static_cast<const SdrObjEditView*>(this)->GetTextEditObject())
    {
        nTol2*=2;
    }

    aRect.expand(nTol2); // add 1 tolerance for all objects

    if (aRect.Contains(rPnt))
    {
        if (!bCheckIfMarkable || IsObjMarkable(pObj,pPV))
        {
            SdrObjList* pOL=pObj->GetSubList();

            if (pOL!=nullptr && pOL->GetObjCount()!=0)
            {
                SdrObject* pTmpObj;
                // adjustment hit point for virtual objects
                Point aPnt( rPnt );

                if ( auto pVirtObj = dynamic_cast<const SdrVirtObj*>( pObj) )
                {
                    Point aOffset = pVirtObj->GetOffset();
                    aPnt.Move( -aOffset.X(), -aOffset.Y() );
                }

                pRet=CheckSingleSdrObjectHit(aPnt,nTol,pOL,pPV,nOptions,pMVisLay,pTmpObj);
            }
            else
            {
                if(!pMVisLay || pMVisLay->IsSet(pObj->GetLayer()))
                {
                    pRet = SdrObjectPrimitiveHit(*pObj, rPnt, {nTol2, nTol2}, *pPV, &pPV->GetVisibleLayers(), false);
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

SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList const * pOL, SdrPageView* pPV, SdrSearchOptions nOptions, const SdrLayerIDSet* pMVisLay, SdrObject*& rpRootObj) const
{
    return (*this).CheckSingleSdrObjectHit(rPnt,nTol,pOL,pPV,nOptions,pMVisLay,rpRootObj,nullptr);
}
SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList const * pOL, SdrPageView* pPV, SdrSearchOptions nOptions, const SdrLayerIDSet* pMVisLay, SdrObject*& rpRootObj,const SdrMarkList * pMarkList) const
{
    SdrObject* pRet=nullptr;
    rpRootObj=nullptr;
    if (!pOL)
        return nullptr;
    const E3dScene* pRemapScene = DynCastE3dScene(pOL->getSdrObjectFromSdrObjList());
    const size_t nObjCount(pOL->GetObjCount());
    size_t nObjNum(nObjCount);

    while (pRet==nullptr && nObjNum>0)
    {
        nObjNum--;
        SdrObject* pObj;

        if(pRemapScene)
        {
            pObj = pOL->GetObj(pRemapScene->RemapOrdNum(nObjNum));
        }
        else
        {
            pObj = pOL->GetObj(nObjNum);
        }
        if (nOptions & SdrSearchOptions::BEFOREMARK)
        {
            if (pMarkList!=nullptr)
            {
                if ((*pMarkList).FindObject(pObj)!=SAL_MAX_SIZE)
                {
                    return nullptr;
                }
            }
        }
        pRet=CheckSingleSdrObjectHit(rPnt,nTol,pObj,pPV,nOptions,pMVisLay);
        if (pRet!=nullptr) rpRootObj=pObj;
    }
    return pRet;
}

SdrObject* SdrMarkView::PickObj(const Point& rPnt, short nTol, SdrPageView*& rpPV, SdrSearchOptions nOptions) const
{
    return PickObj(rPnt, nTol, rpPV, nOptions, nullptr);
}

SdrObject* SdrMarkView::PickObj(const Point& rPnt, short nTol, SdrPageView*& rpPV, SdrSearchOptions nOptions, SdrObject** ppRootObj, bool* pbHitPassDirect) const
{ // TODO: lacks a Pass2,Pass3
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    rMarkList.ForceSort();
    if (ppRootObj!=nullptr) *ppRootObj=nullptr;
    if (pbHitPassDirect!=nullptr) *pbHitPassDirect=true;
    SdrObject* pRet = nullptr;
    rpPV=nullptr;
    bool bMarked(nOptions & SdrSearchOptions::MARKED);
    bool bMasters=!bMarked && bool(nOptions & SdrSearchOptions::ALSOONMASTER);
    // nOptions & SdrSearchOptions::NEXT: n.i.
    // nOptions & SdrSearchOptions::PASS2BOUND: n.i.
    // nOptions & SdrSearchOptions::PASS3NEAREST// n.i.
    if (nTol<0) nTol=ImpGetHitTolLogic(nTol,nullptr);
    SdrObject* pObj=nullptr;
    SdrObject* pHitObj=nullptr;
    SdrPageView* pPV=nullptr;
    if (static_cast<const SdrObjEditView*>(this)->IsTextEditFrameHit(rPnt)) {
        pObj=static_cast<const SdrObjEditView*>(this)->GetTextEditObject();
        pHitObj=pObj;
        pPV=static_cast<const SdrObjEditView*>(this)->GetTextEditPageView();
    }
    if (bMarked) {
        const size_t nMrkCnt=rMarkList.GetMarkCount();
        size_t nMrkNum=nMrkCnt;
        while (pHitObj==nullptr && nMrkNum>0) {
            nMrkNum--;
            SdrMark* pM=rMarkList.GetMark(nMrkNum);
            pObj=pM->GetMarkedSdrObj();
            pPV=pM->GetPageView();
            pHitObj=CheckSingleSdrObjectHit(rPnt,nTol,pObj,pPV,nOptions,nullptr);
        }
    }
    else
    {
        pPV = GetSdrPageView();

        if(pPV)
        {
            SdrPage* pPage=pPV->GetPage();
            sal_uInt16 nPgCount=1;

            if(bMasters && pPage->TRG_HasMasterPage())
            {
                nPgCount++;
            }
            bool bWholePage(nOptions & SdrSearchOptions::WHOLEPAGE);
            bool bExtraPassForWholePage=bWholePage && pPage!=pPV->GetObjList();
            if (bExtraPassForWholePage) nPgCount++; // First search in AktObjList, then on the entire page
            sal_uInt16 nPgNum=nPgCount;
            while (pHitObj==nullptr && nPgNum>0) {
                SdrSearchOptions nTmpOptions=nOptions;
                nPgNum--;
                const SdrLayerIDSet* pMVisLay=nullptr;
                SdrObjList* pObjList=nullptr;
                if (pbHitPassDirect!=nullptr) *pbHitPassDirect = true;
                if (nPgNum>=nPgCount-1 || (bExtraPassForWholePage && nPgNum>=nPgCount-2))
                {
                    pObjList=pPV->GetObjList();
                    if (bExtraPassForWholePage && nPgNum==nPgCount-2) {
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
                pHitObj=CheckSingleSdrObjectHit(rPnt,nTol,pObjList,pPV,nTmpOptions,pMVisLay,pObj,&rMarkList);
            }
        }
    }
    if (pHitObj!=nullptr) {
        if (ppRootObj!=nullptr) *ppRootObj=pObj;
        if (nOptions & SdrSearchOptions::DEEP) pObj=pHitObj;
        if (nOptions & SdrSearchOptions::TESTTEXTEDIT) {
            assert(pPV);
            if (!pObj->HasTextEdit() || pPV->GetLockedLayers().IsSet(pObj->GetLayer())) {
                pObj=nullptr;
            }
        }
        if (pObj!=nullptr && (nOptions & SdrSearchOptions::TESTMACRO)) {
            assert(pPV);
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=rPnt;
            aHitRec.nTol=nTol;
            aHitRec.pVisiLayer=&pPV->GetVisibleLayers();
            aHitRec.pPageView=pPV;
            if (!pObj->HasMacro() || !pObj->IsMacroHit(aHitRec)) pObj=nullptr;
        }
        if (pObj!=nullptr) {
            pRet=pObj;
            rpPV=pPV;
        }
    }
    return pRet;
}

bool SdrMarkView::PickMarkedObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, SdrSearchOptions nOptions) const
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    rMarkList.ForceSort();
    const bool bBoundCheckOn2ndPass(nOptions & SdrSearchOptions::PASS2BOUND);
    rpObj=nullptr;
    rpPV=nullptr;
    const size_t nMarkCount=rMarkList.GetMarkCount();
    for (size_t nMarkNum=nMarkCount; nMarkNum>0;) {
        --nMarkNum;
        SdrMark* pM=rMarkList.GetMark(nMarkNum);
        SdrPageView* pPV=pM->GetPageView();
        SdrObject* pObj=pM->GetMarkedSdrObj();
        if (CheckSingleSdrObjectHit(rPnt,mnHitTolLog,pObj,pPV,SdrSearchOptions::TESTMARKABLE,nullptr)) {
            rpObj=pObj;
            rpPV=pPV;
            return true;
        }
    }
    if (bBoundCheckOn2ndPass) {
        for (size_t nMarkNum=nMarkCount; nMarkNum>0;) {
            --nMarkNum;
            SdrMark* pM=rMarkList.GetMark(nMarkNum);
            SdrPageView* pPV=pM->GetPageView();
            SdrObject* pObj=pM->GetMarkedSdrObj();
            tools::Rectangle aRect(pObj->GetCurrentBoundRect());
            aRect.AdjustLeft( -mnHitTolLog );
            aRect.AdjustTop( -mnHitTolLog );
            aRect.AdjustRight(mnHitTolLog );
            aRect.AdjustBottom(mnHitTolLog );
            if (aRect.Contains(rPnt)) {
                rpObj=pObj;
                rpPV=pPV;
                return true;
            }
        }
    }
    return false;
}


void SdrMarkView::UnmarkAllObj(SdrPageView const * pPV)
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if (rMarkList.GetMarkCount()==0)
        return;

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

void SdrMarkView::MarkAllObj(SdrPageView* pPV)
{
    BrkAction();

    if(!pPV)
    {
        pPV = GetSdrPageView();
    }

    // #i69171# pPV may still be NULL if there is no SDrPageView (!), e.g. when inserting
    // other files
    if(pPV)
    {
        const bool bMarkChg(GetMarkedObjectListWriteAccess().InsertPageView(*pPV));

        if(bMarkChg)
        {
            MarkListHasChanged();
        }
    }

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if(rMarkList.GetMarkCount())
    {
        AdjustMarkHdl();
    }
}

void SdrMarkView::AdjustMarkHdl(SfxViewShell* pOtherShell)
{
    CheckMarked();
    SetMarkRects();
    SetMarkHandles(pOtherShell);
}

// BoundRect in model coordinates, no GridOffset added
tools::Rectangle SdrMarkView::GetMarkedObjBoundRect() const
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    tools::Rectangle aRect;
    for (size_t nm=0; nm<rMarkList.GetMarkCount(); ++nm) {
        SdrMark* pM=rMarkList.GetMark(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        tools::Rectangle aR1(pO->GetCurrentBoundRect());
        if (aRect.IsEmpty()) aRect=aR1;
        else aRect.Union(aR1);
    }
    return aRect;
}

// ObjRect in model coordinates, no GridOffset added
const tools::Rectangle& SdrMarkView::GetMarkedObjRect() const
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if (mbMarkedObjRectDirty) {
        const_cast<SdrMarkView*>(this)->mbMarkedObjRectDirty=false;
        tools::Rectangle aRect;
        for (size_t nm=0; nm<rMarkList.GetMarkCount(); ++nm) {
            SdrMark* pM=rMarkList.GetMark(nm);
            SdrObject* pO = pM->GetMarkedSdrObj();
            if (!pO)
                continue;
            tools::Rectangle aR1(pO->GetSnapRect());
            if (aRect.IsEmpty()) aRect=aR1;
            else aRect.Union(aR1);
        }
        const_cast<SdrMarkView*>(this)->maMarkedObjRect=aRect;
    }
    return maMarkedObjRect;
}


OUString SdrMarkView::ImpGetDescriptionString(TranslateId pStrCacheID, ImpGetDescriptionOptions nOpt) const
{
    OUString sStr = SvxResId(pStrCacheID);
    const sal_Int32 nPos = sStr.indexOf("%1");

    if(nPos != -1)
    {
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        if(nOpt == ImpGetDescriptionOptions::POINTS)
        {
            sStr = sStr.replaceAt(nPos, 2, rMarkList.GetPointMarkDescription());
        }
        else if(nOpt == ImpGetDescriptionOptions::GLUEPOINTS)
        {
            sStr = sStr.replaceAt(nPos, 2, rMarkList.GetGluePointMarkDescription());
        }
        else
        {
            sStr = sStr.replaceAt(nPos, 2, rMarkList.GetMarkDescription());
        }
    }

    return sStr.replaceFirst("%2", "0");
}


void SdrMarkView::EnterMarkedGroup()
{
    // We enter only the first group found (in only one PageView), because
    // PageView::EnterGroup calls an AdjustMarkHdl.
    // TODO: I'll have to prevent that via a flag.
    SdrPageView* pPV = GetSdrPageView();

    if(!pPV)
        return;

    bool bEnter=false;
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    for (size_t nm = rMarkList.GetMarkCount(); nm > 0 && !bEnter;)
    {
        --nm;
        SdrMark* pM=rMarkList.GetMark(nm);
        if (pM->GetPageView()==pPV) {
            SdrObject* pObj=pM->GetMarkedSdrObj();
            if (pObj->IsGroupObject()) {
                if (pPV->EnterGroup(pObj)) {
                    bEnter=true;
                }
            }
        }
    }
}


void SdrMarkView::MarkListHasChanged()
{
    GetMarkedObjectListWriteAccess().SetNameDirty();
    maSdrViewSelection.SetEdgesOfMarkedNodesDirty();

    mbMarkedObjRectDirty=true;
    mbMarkedPointsRectsDirty=true;
    bool bOneEdgeMarked=false;
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if (rMarkList.GetMarkCount()==1) {
        const SdrObject* pObj=rMarkList.GetMark(0)->GetMarkedSdrObj();
        if (pObj->GetObjInventor()==SdrInventor::Default) {
            bOneEdgeMarked = pObj->GetObjIdentifier() == SdrObjKind::Edge;
        }
    }
    ImpSetGlueVisible4(bOneEdgeMarked);
}


void SdrMarkView::SetMoveOutside(bool bOn)
{
    maHdlList.SetMoveOutside(bOn);
}

void SdrMarkView::SetDesignMode( bool bOn )
{
    if ( mbDesignMode != bOn )
    {
        mbDesignMode = bOn;
        SdrPageView* pPageView = GetSdrPageView();
        if ( pPageView )
            pPageView->SetDesignMode( bOn );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
