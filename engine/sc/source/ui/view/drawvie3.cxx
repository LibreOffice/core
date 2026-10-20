/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <cstdlib>

#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/ImageMapInfo.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/kit.hxx>
#include <o3tl/unit_conversion.hxx>
#include <sfx2/kit/helper.hxx>
#include <officecfg/Office/Common.hxx>

#include <strings.hrc>
#include <scresid.hxx>
#include <drawview.hxx>
#include <drwlayer.hxx>
#include "imapwrap.hxx"
#include <viewdata.hxx>
#include <document.hxx>
#include <userdat.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <SheetViewManager.hxx>

ScDrawView::ScDrawView(
    OutputDevice* pOut,
    ScViewData& rData )
:   FmFormView(*rData.GetDocument().GetDrawLayer(), pOut),
    rViewData( rData ),
    pDev( pOut ),
    rDoc( rData.GetDocument() ),
    nTab( rData.CurrentTabForData() ),
    pDropMarkObj( nullptr ),
    bInConstruct( true )
{
    SetNegativeX(comphelper::COKit::isActive() && rDoc.IsLayoutRTL(nTab));
    // #i73602# Use default from the configuration
    SetBufferedOverlayAllowed(!comphelper::IsFuzzing() && officecfg::Office::Common::Drawinglayer::OverlayBuffer_Calc::get());

    // #i74769#, #i75172# Use default from the configuration
    SetBufferedOutputAllowed(!comphelper::IsFuzzing() && officecfg::Office::Common::Drawinglayer::PaintBuffer_Calc::get());

    Construct();
}

// set anchor

void ScDrawView::SetPageAnchored()
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 0 )
        return;

    const size_t nCount = rMarkList.GetMarkCount();

    BegUndo(ScResId(SCSTR_UNDO_PAGE_ANCHOR));
    for( size_t i=0; i<nCount; ++i )
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        AddUndo (std::make_unique<ScUndoAnchorData>( pObj, &rDoc, nTab ));
        ScDrawLayer::SetPageAnchored( *pObj );
    }
    EndUndo();

    rViewData.GetDocShell()->SetDrawModified();

    // Remove the anchor object.
    maHdlList.RemoveAllByKind(SdrHdlKind::Anchor);
    maHdlList.RemoveAllByKind(SdrHdlKind::Anchor_TR);
}

void ScDrawView::SetCellAnchored(bool bResizeWithCell)
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 0 )
        return;

    const size_t nCount = rMarkList.GetMarkCount();

    BegUndo(ScResId(SCSTR_UNDO_CELL_ANCHOR));
    for( size_t i=0; i<nCount; ++i )
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        AddUndo (std::make_unique<ScUndoAnchorData>( pObj, &rDoc, nTab ));
        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, rDoc, nTab, bResizeWithCell);
    }
    EndUndo();

    rViewData.GetDocShell()->SetDrawModified();

    // Set the anchor object.
    AddCustomHdl();
}

ScAnchorType ScDrawView::GetAnchorType() const
{
    bool bPage = false;
    bool bCell = false;
    bool bCellResize = false;
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if( rMarkList.GetMarkCount() != 0 )
    {
        const size_t nCount = rMarkList.GetMarkCount();
        for( size_t i=0; i<nCount; ++i )
        {
            const SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
            const ScAnchorType aAnchorType = ScDrawLayer::GetAnchorType( *pObj );
            if( aAnchorType == SCA_CELL )
                bCell =true;
            else if (aAnchorType == SCA_CELL_RESIZE)
                bCellResize = true;
            else
                bPage = true;
        }
    }
    if( bPage && !bCell && !bCellResize )
        return SCA_PAGE;
    if( !bPage && bCell && !bCellResize )
        return SCA_CELL;
    if( !bPage && !bCell && bCellResize )
        return SCA_CELL_RESIZE;
    return SCA_DONTKNOW;
}

namespace {

bool lcl_AreRectanglesApproxEqual(const tools::Rectangle& rRectA, const tools::Rectangle& rRectB)
{
    // Twips <-> Hmm conversions introduce +-1 differences although the rectangles should actually
    // be equal. Therefore test with == is not appropriate in some cases.
    if (std::abs(rRectA.Left() - rRectB.Left()) > 1)
        return false;
    if (std::abs(rRectA.Top() - rRectB.Top()) > 1)
        return false;
    if (std::abs(rRectA.Right() - rRectB.Right()) > 1)
        return false;
    if (std::abs(rRectA.Bottom() - rRectB.Bottom()) > 1)
        return false;
    return true;
}

/**
 * Updated the anchors of any non-note object that is cell anchored which
 * has been moved since the last anchors for its position was calculated.
 */
void adjustAnchoredPosition(const SdrHint& rHint, const ScDocument& rDoc, SCTAB nTab)
{
    if (rHint.GetKind() != SdrHintKind::ObjectChange && rHint.GetKind() != SdrHintKind::ObjectInserted)
        return;

    SdrObject* pObj = const_cast<SdrObject*>(rHint.GetObject());
    if (!pObj)
        return;

    ScDrawObjData *pAnchor = ScDrawLayer::GetOrCreateObjData(pObj);
    if (!pAnchor)
        return;

    if (pAnchor->meType == ScDrawObjData::CellNote)
        return;

    // SetCellAnchoredFromPosition has to be called only if shape geometry has been changed, and not
    // if only shape visibility has been changed. It is not enough to test shape rect, because e.g. a
    // 180deg rotation changes only the logic rect (tdf#139583).
    ScDrawObjData& rNoRotatedAnchor = *ScDrawLayer::getOrCreateNonRotatedObjData(pObj, true /*bCreate*/);
    if (lcl_AreRectanglesApproxEqual(pAnchor->getShapeRect(), pObj->GetSnapRect())
        && lcl_AreRectanglesApproxEqual(rNoRotatedAnchor.getShapeRect(), pObj->GetLogicRect()))
        return;

    if (pAnchor->maStart.Tab() != nTab)
        // The object is not anchored on the current sheet.  Skip it.
        // TODO: In the future, we may want to adjust objects that are
        // anchored on all selected sheets.
        return;

    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, rDoc, pAnchor->maStart.Tab(), pAnchor->mbResizeWithCell);
}

}

void ScDrawView::notifyObjectTilesInvalidated(const SdrHint& rHint)
{
    if (!comphelper::COKit::isActive())
        return;

    const SdrHintKind eKind = rHint.GetKind();
    if (eKind != SdrHintKind::ObjectChange && eKind != SdrHintKind::ObjectInserted
        && eKind != SdrHintKind::ObjectRemoved)
        return;

    const SdrObject* pObject = rHint.GetObject();
    if (!pObject)
        return;

    const SdrPage* pPage = rHint.GetPage();
    if (!pPage)
        pPage = pObject->getSdrPageFromSdrObject();
    if (!pPage)
        return;

    tools::Rectangle aRect100thMM = pObject->GetCurrentBoundRect();
    if (aRect100thMM.IsEmpty())
        return;

    // The drawing layer repaints an object only through the page a view is showing, so the
    // client keeps the tiles it cached for any other sheet. Each draw page belongs to the table
    // with the same number, so the object's rectangle goes out in twips under that part number
    // and the tiles below it are fetched again when the sheet is shown next.
    const SCTAB nObjectTab = static_cast<SCTAB>(pPage->GetPageNum());
    if (rDoc.IsLayoutRTL(nObjectTab))
    {
        // A right-to-left sheet places its objects at negative X, mirrored at the left edge.
        aRect100thMM = tools::Rectangle(std::max(tools::Long(0), -aRect100thMM.Right()),
                                        aRect100thMM.Top(),
                                        std::max(tools::Long(0), -aRect100thMM.Left()),
                                        aRect100thMM.Bottom());
    }
    const tools::Rectangle aRectTwips
        = o3tl::convert(aRect100thMM, o3tl::Length::mm100, o3tl::Length::twip);

    ScTabViewShell* pViewShell = rViewData.GetViewShell();
    KitHelper::notifyInvalidation(pViewShell, static_cast<int>(nObjectTab), &aRectTwips);

    // A sheet view tab shares the draw page of its base sheet and is a part of its own.
    std::shared_ptr<sc::SheetViewManager> pManager = rDoc.GetSheetViewManager(nObjectTab);
    if (!pManager || pManager->isEmpty())
        return;
    for (auto& rSheetView : pManager->iterateValidSheetViews())
        KitHelper::notifyInvalidation(pViewShell, static_cast<int>(rSheetView.getTableNumber()),
                                      &aRectTwips);
}

void ScDrawView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.GetId() == SfxHintId::ThisIsAnSdrHint)
    {
        const SdrHint* pSdrHint = static_cast<const SdrHint*>( &rHint );
        adjustAnchoredPosition(*pSdrHint, rDoc, nTab);
        notifyObjectTilesInvalidated(*pSdrHint);
        FmFormView::Notify( rBC,rHint );
    }
    else if (rHint.GetId() == SfxHintId::ScTabDeleted) // Sheet has been deleted
    {
        auto pDeletedHint = static_cast<const ScTabDeletedHint*>(&rHint);
        SCTAB nDelTab = pDeletedHint->GetTab();
        if (ValidTab(nDelTab))
        {
            // used to be: HidePagePgNum(nDelTab) - hide only if the deleted sheet is shown here
            if ( nDelTab == nTab )
                HideSdrPage();
        }
    }
    else if (rHint.GetId() == SfxHintId::ScTabSizeChanged) // Size has been changed
    {
        auto pChangedHint = static_cast<const ScTabSizeChangedHint*>(&rHint);
        if ( nTab == pChangedHint->GetTab() )
            UpdateWorkArea();
    }
    else
        FmFormView::Notify( rBC,rHint );
}

void ScDrawView::UpdateIMap( SdrObject* pObj )
{
    if ( rViewData.GetViewShell()->GetViewFrame().HasChildWindow( ScIMapChildWindowId() ) &&
         pObj && ( dynamic_cast<const SdrGrafObj*>( pObj) != nullptr || dynamic_cast<const SdrOle2Obj*>( pObj) != nullptr ) )
        return;

    Graphic     aGraphic;
    TargetList  aTargetList;
    SvxIMapInfo* pIMapInfo = SvxIMapInfo::GetIMapInfo( pObj );
    const ImageMap* pImageMap = nullptr;
    if ( pIMapInfo )
        pImageMap = &pIMapInfo->GetImageMap();

    // handle target list
    SfxViewFrame::GetTargetList( aTargetList );

    // handle graphics from object
    if ( auto pGrafObj = dynamic_cast<SdrGrafObj*>( pObj) )
        aGraphic = pGrafObj->GetGraphic();
    else
    {
        const Graphic* pGraphic = static_cast<const SdrOle2Obj*>(pObj)->GetGraphic();
        if ( pGraphic )
            aGraphic = *pGraphic;
    }

    ScIMapDlgSet( aGraphic, pImageMap, &aTargetList, pObj );    // from imapwrap
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
