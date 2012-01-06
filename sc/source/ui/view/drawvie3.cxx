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




// INCLUDE ---------------------------------------------------------------

#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>

#include "drawview.hxx"
#include "drwlayer.hxx"
#include "viewdata.hxx"
#include "dbfunc.hxx"
#include "document.hxx"
#include "userdat.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"

void ScIMapDlgSet( const Graphic& rGraphic, const ImageMap* pImageMap,
                    const TargetList* pTargetList, void* pEditingObj );     // imapwrap
sal_uInt16 ScIMapChildWindowId();

// STATIC DATA -----------------------------------------------------------

ScDrawView::ScDrawView( OutputDevice* pOut, ScViewData* pData ) :
    FmFormView( pData->GetDocument()->GetDrawLayer(), pOut ),
    pViewData( pData ),
    pDev( pOut ),
    pDoc( pData->GetDocument() ),
    nTab( pData->GetTabNo() ),
    pDropMarker( NULL ),
    pDropMarkObj( NULL ),
    bInConstruct( true )
{
    // #i73602# Use default from the configuration
    SetBufferedOverlayAllowed(getOptionsDrawinglayer().IsOverlayBuffer_Calc());

    // #i74769#, #i75172# Use default from the configuration
    SetBufferedOutputAllowed(getOptionsDrawinglayer().IsPaintBuffer_Calc());

    Construct();
}

// Verankerung setzen

void ScDrawView::SetPageAnchored()
{
    SdrObject* pObj = NULL;
    if( AreObjectsMarked() )
    {
        const SdrMarkList* pMark = &GetMarkedObjectList();
        sal_uLong nCount = pMark->GetMarkCount();
        for( sal_uLong i=0; i<nCount; i++ )
        {
            pObj = pMark->GetMark(i)->GetMarkedSdrObj();
            ScDrawLayer::SetPageAnchored( *pObj );
        }

        if ( pViewData )
            pViewData->GetDocShell()->SetDrawModified();

        // Remove the anchor object.
        aHdl.RemoveAllByKind(HDL_ANCHOR);
        aHdl.RemoveAllByKind(HDL_ANCHOR_TR);
    }
}

void ScDrawView::SetCellAnchored()
{
    if (!pDoc)
        return;

    SdrObject* pObj = NULL;
    if( AreObjectsMarked() )
    {
        const SdrMarkList* pMark = &GetMarkedObjectList();
        sal_uLong nCount = pMark->GetMarkCount();
        for( sal_uLong i=0; i<nCount; i++ )
        {
            pObj = pMark->GetMark(i)->GetMarkedSdrObj();
            ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *pDoc, nTab);
        }

        if ( pViewData )
            pViewData->GetDocShell()->SetDrawModified();

        // Set the anchor object.
        AddCustomHdl();
    }
}

ScAnchorType ScDrawView::GetAnchorType() const
{
    sal_Bool bPage = false;
    sal_Bool bCell = false;
    const SdrObject* pObj = NULL;
    if( AreObjectsMarked() )
    {
        const SdrMarkList* pMark = &GetMarkedObjectList();
        sal_uLong nCount = pMark->GetMarkCount();
        Point p0;
        for( sal_uLong i=0; i<nCount; i++ )
        {
            pObj = pMark->GetMark(i)->GetMarkedSdrObj();
            if( ScDrawLayer::GetAnchorType( *pObj ) == SCA_CELL )
                bCell =true;
            else
                bPage = sal_True;
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

    if (pAnchor->maLastRect == pObj->GetLogicRect())
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
    if (rHint.ISA(ScTabDeletedHint))                        // Tabelle geloescht
    {
        SCTAB nDelTab = ((ScTabDeletedHint&)rHint).GetTab();
        if (ValidTab(nDelTab))
        {
            // used to be: HidePagePgNum(nDelTab) - hide only if the deleted sheet is shown here
            if ( nDelTab == nTab )
                HideSdrPage();
        }
    }
    else if (rHint.ISA(ScTabSizeChangedHint))               // Groesse geaendert
    {
        if ( nTab == ((ScTabSizeChangedHint&)rHint).GetTab() )
            UpdateWorkArea();
    }
    else if ( rHint.ISA( SdrHint ) )
    {
        if (const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint ))
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
         pObj && ( pObj->ISA(SdrGrafObj) || pObj->ISA(SdrOle2Obj) ) )
    {
        Graphic     aGraphic;
        TargetList  aTargetList;
        ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo( pObj );
        const ImageMap* pImageMap = NULL;
        if ( pIMapInfo )
            pImageMap = &pIMapInfo->GetImageMap();

        // Target-Liste besorgen
        pViewData->GetViewShell()->GetViewFrame()->GetTargetList( aTargetList );

        // Grafik vom Objekt besorgen
        if ( pObj->ISA( SdrGrafObj ) )
            aGraphic = ( (SdrGrafObj*) pObj )->GetGraphic();
        else
        {
            Graphic* pGraphic = ((const SdrOle2Obj*) pObj )->GetGraphic();
            if ( pGraphic )
                aGraphic = *pGraphic;
        }

        ScIMapDlgSet( aGraphic, pImageMap, &aTargetList, pObj );    // aus imapwrap

        // TargetListe kann von uns wieder geloescht werden
        for ( size_t i = 0, n = aTargetList.size(); i < n; ++i )
            delete aTargetList[ i ];
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
