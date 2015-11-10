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

#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>

#include "sc.hrc"
#include "scresid.hxx"
#include "drawview.hxx"
#include "drwlayer.hxx"
#include "imapwrap.hxx"
#include "viewdata.hxx"
#include "dbfunc.hxx"
#include "document.hxx"
#include "userdat.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"

ScDrawView::ScDrawView( OutputDevice* pOut, ScViewData* pData ) :
    FmFormView( pData->GetDocument()->GetDrawLayer(), pOut ),
    pViewData( pData ),
    pDev( pOut ),
    pDoc( pData->GetDocument() ),
    nTab( pData->GetTabNo() ),
    pDropMarker( nullptr ),
    pDropMarkObj( nullptr ),
    bInConstruct( true )
{
    // #i73602# Use default from the configuration
    SetBufferedOverlayAllowed(getOptionsDrawinglayer().IsOverlayBuffer_Calc());

    // #i74769#, #i75172# Use default from the configuration
    SetBufferedOutputAllowed(getOptionsDrawinglayer().IsPaintBuffer_Calc());

    Construct();
}

// set anchor

void ScDrawView::SetPageAnchored()
{
    if( AreObjectsMarked() )
    {
        const SdrMarkList* pMark = &GetMarkedObjectList();
        const size_t nCount = pMark->GetMarkCount();

        BegUndo( OUString(ScResId( SCSTR_UNDO_PAGE_ANCHOR )) );
        for( size_t i=0; i<nCount; ++i )
        {
            SdrObject* pObj = pMark->GetMark(i)->GetMarkedSdrObj();
            AddUndo (new ScUndoAnchorData( pObj, pDoc, nTab ));
            ScDrawLayer::SetPageAnchored( *pObj );
        }
        EndUndo();

        if ( pViewData )
            pViewData->GetDocShell()->SetDrawModified();

        // Remove the anchor object.
        maHdlList.RemoveAllByKind(HDL_ANCHOR);
        maHdlList.RemoveAllByKind(HDL_ANCHOR_TR);
    }
}

void ScDrawView::SetCellAnchored()
{
    if (!pDoc)
        return;

    if( AreObjectsMarked() )
    {
        const SdrMarkList* pMark = &GetMarkedObjectList();
        const size_t nCount = pMark->GetMarkCount();

        BegUndo( OUString(ScResId( SCSTR_UNDO_CELL_ANCHOR )) );
        for( size_t i=0; i<nCount; ++i )
        {
            SdrObject* pObj = pMark->GetMark(i)->GetMarkedSdrObj();
            AddUndo (new ScUndoAnchorData( pObj, pDoc, nTab ));
            ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *pDoc, nTab);
        }
        EndUndo();

        if ( pViewData )
            pViewData->GetDocShell()->SetDrawModified();

        // Set the anchor object.
        AddCustomHdl();
    }
}

ScAnchorType ScDrawView::GetAnchorType() const
{
    bool bPage = false;
    bool bCell = false;
    if( AreObjectsMarked() )
    {
        const SdrMarkList* pMark = &GetMarkedObjectList();
        const size_t nCount = pMark->GetMarkCount();
        for( size_t i=0; i<nCount; ++i )
        {
            const SdrObject* pObj = pMark->GetMark(i)->GetMarkedSdrObj();
            if( ScDrawLayer::GetAnchorType( *pObj ) == SCA_CELL )
                bCell =true;
            else
                bPage = true;
        }
    }
    if( bPage && !bCell )
        return SCA_PAGE;
    if( !bPage && bCell )
        return SCA_CELL;
    return SCA_DONTKNOW;
}

namespace {

/**
 * Updated the anchors of any non-note object that is cell anchored which
 * has been moved since the last anchors for its position was calculated.
 */
void adjustAnchoredPosition(const SdrHint& rHint, const ScDocument& rDoc, SCTAB nTab)
{
    if (rHint.GetKind() != HINT_OBJCHG && rHint.GetKind() != HINT_OBJINSERTED)
        return;

    SdrObject* pObj = const_cast<SdrObject*>(rHint.GetObject());
    if (!pObj)
        return;

    ScDrawObjData *pAnchor = ScDrawLayer::GetObjData(pObj);
    if (!pAnchor)
        return;

    if (pAnchor->meType == ScDrawObjData::CellNote)
        return;

    if (pAnchor->maLastRect == pObj->GetSnapRect())
        return;

    if (pAnchor->maStart.Tab() != nTab)
        // The object is not anchored on the current sheet.  Skip it.
        // TODO: In the future, we may want to adjust objects that are
        // anchored on all selected sheets.
        return;

    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, rDoc, pAnchor->maStart.Tab());
}

}

void ScDrawView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (dynamic_cast<const ScTabDeletedHint*>(&rHint))                        // Sheet has been deleted
    {
        SCTAB nDelTab = static_cast<const ScTabDeletedHint&>(rHint).GetTab();
        if (ValidTab(nDelTab))
        {
            // used to be: HidePagePgNum(nDelTab) - hide only if the deleted sheet is shown here
            if ( nDelTab == nTab )
                HideSdrPage();
        }
    }
    else if (dynamic_cast<const ScTabSizeChangedHint*>(&rHint))               // Size has been changed
    {
        if ( nTab == static_cast<const ScTabSizeChangedHint&>(rHint).GetTab() )
            UpdateWorkArea();
    }
    else if ( const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>( &rHint ) )
    {
        adjustAnchoredPosition(*pSdrHint, *pDoc, nTab);
        FmFormView::Notify( rBC,rHint );
    }
    else
        FmFormView::Notify( rBC,rHint );
}

void ScDrawView::UpdateIMap( SdrObject* pObj )
{
    if ( pViewData &&
         pViewData->GetViewShell()->GetViewFrame()->HasChildWindow( ScIMapChildWindowId() ) &&
         pObj && ( dynamic_cast<const SdrGrafObj*>( pObj) != nullptr || dynamic_cast<const SdrOle2Obj*>( pObj) != nullptr ) )
    {
        Graphic     aGraphic;
        TargetList  aTargetList;
        ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo( pObj );
        const ImageMap* pImageMap = nullptr;
        if ( pIMapInfo )
            pImageMap = &pIMapInfo->GetImageMap();

        // handle target list
        pViewData->GetViewShell()->GetViewFrame()->GetTargetList( aTargetList );

        // handle graphics from object
        if ( dynamic_cast<const SdrGrafObj*>( pObj) !=  nullptr )
            aGraphic = static_cast<SdrGrafObj*>(pObj)->GetGraphic();
        else
        {
            const Graphic* pGraphic = static_cast<const SdrOle2Obj*>(pObj)->GetGraphic();
            if ( pGraphic )
                aGraphic = *pGraphic;
        }

        ScIMapDlgSet( aGraphic, pImageMap, &aTargetList, pObj );    // from imapwrap
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
