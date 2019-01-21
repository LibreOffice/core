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
#include <SectionView.hxx>
#include <DesignView.hxx>
#include <RptPage.hxx>
#include <RptObject.hxx>
#include <RptDef.hxx>
#include <svx/svxids.hrc>
#include <svx/svddrgmt.hxx>
#include <vcl/scrbar.hxx>
#include <ReportSection.hxx>
#include <ReportWindow.hxx>
#include <strings.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

namespace rptui
{
    using namespace ::com::sun::star;

OSectionView::OSectionView(
    SdrModel& rSdrModel,
    OReportSection* _pSectionWindow,
    OReportWindow* pEditor)
:   SdrView(rSdrModel, _pSectionWindow)
    ,m_pReportWindow( pEditor )
    ,m_pSectionWindow(_pSectionWindow)
{
    // SetPagePaintingAllowed(false);
    SetBufferedOutputAllowed(true);
    SetBufferedOverlayAllowed(true);
    SetPageBorderVisible(false);
    SetBordVisible();
    SetQuickTextEditMode(false);
}


OSectionView::~OSectionView()
{
}


void OSectionView::MarkListHasChanged()
{
    SdrView::MarkListHasChanged();

    if ( m_pReportWindow && m_pSectionWindow && !m_pSectionWindow->getPage()->getSpecialMode() )
    {
        DlgEdHint aHint( RPTUI_HINT_SELECTIONCHANGED );
        m_pReportWindow->getReportView()->Broadcast( aHint );
        m_pReportWindow->getReportView()->UpdatePropertyBrowserDelayed(*this);
    }
}


void OSectionView::MakeVisible( const tools::Rectangle& rRect, vcl::Window& rWin )
{
    // visible area
    MapMode aMap( rWin.GetMapMode() );
    const Point aOrg( aMap.GetOrigin() );
    const Size aVisSize( rWin.GetOutputSize() );
    const tools::Rectangle aVisRect( Point(-aOrg.X(),-aOrg.Y()), aVisSize );

    // check, if rectangle is inside visible area
    if ( !aVisRect.IsInside( rRect ) )
    {
        // calculate scroll distance; the rectangle must be inside the visible area
        sal_Int32 nScrollX = 0, nScrollY = 0;

        const sal_Int32 nVisLeft   = aVisRect.Left();
        const sal_Int32 nVisRight  = aVisRect.Right();
        const sal_Int32 nVisTop    = aVisRect.Top();
        const sal_Int32 nVisBottom = aVisRect.Bottom();

        // don't scroll beyond the page size
        Size aPageSize = m_pSectionWindow->getPage()->GetSize();
        const sal_Int32 nPageWidth  = aPageSize.Width();
        const sal_Int32 nPageHeight = aPageSize.Height();

        if ( nVisRight + nScrollX > nPageWidth )
            nScrollX = nPageWidth - nVisRight;

        if ( nVisLeft + nScrollX < 0 )
            nScrollX = -nVisLeft;

        if ( nVisBottom + nScrollY > nPageHeight )
            nScrollY = nPageHeight - nVisBottom;

        if ( nVisTop + nScrollY < 0 )
            nScrollY = -nVisTop;

        // scroll window
        rWin.Update();
        rWin.Scroll( -nScrollX, -nScrollY );
        aMap.SetOrigin( Point( aOrg.X() - nScrollX, aOrg.Y() - nScrollY ) );
        rWin.SetMapMode( aMap );
        rWin.Update();
        rWin.Invalidate();

        if ( m_pReportWindow )
        {
            const DlgEdHint aHint( RPTUI_HINT_WINDOWSCROLLED );
            m_pReportWindow->getReportView()->Broadcast( aHint );
        }
    }
    else
    {
        rWin.Invalidate(InvalidateFlags::NoErase);
    }
}

void OSectionView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SdrView::Notify(rBC,rHint);
    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
    if ( pSdrHint )
    {
        const SdrObject* pObj = pSdrHint->GetObject();
        const SdrHintKind eKind = pSdrHint->GetKind();
        // check for change of selected object
        if(SdrHintKind::ObjectChange == eKind && pObj && IsObjMarked(pObj))
            AdjustMarkHdl();
        else if ( eKind == SdrHintKind::ObjectRemoved )
            ObjectRemovedInAliveMode(pObj);
    }
}


void OSectionView::ObjectRemovedInAliveMode( const SdrObject* _pObject )
{
    const SdrMarkList& rMarkedList = GetMarkedObjectList();
    const size_t nMark = rMarkedList.GetMarkCount();

    for( size_t i = 0; i < nMark; ++i )
    {
        SdrObject* pSdrObj = rMarkedList.GetMark(i)->GetMarkedSdrObj();
        if (_pObject == pSdrObj)
        {
            SdrPageView*    pPgView = GetSdrPageView();
            BrkAction();
            MarkObj( pSdrObj, pPgView, true );
            break;
        }
    }
}


void OSectionView::SetMarkedToLayer( SdrLayerID _nLayerNo )
{
    if (AreObjectsMarked())
    {
        //  #i11702# use SdrUndoObjectLayerChange for undo
        //  STR_UNDO_SELATTR is "Attributes" - should use a different text later
        BegUndo( );

        const SdrMarkList& rMark = GetMarkedObjectList();
        const size_t nCount = rMark.GetMarkCount();
        for (size_t i = 0; i<nCount; ++i)
        {
            SdrObject* pObj = rMark.GetMark(i)->GetMarkedSdrObj();
            if ( dynamic_cast< const OCustomShape *>( pObj ) !=  nullptr )
            {
                AddUndo( std::make_unique<SdrUndoObjectLayerChange>( *pObj, pObj->GetLayer(), _nLayerNo) );
                pObj->SetLayer( _nLayerNo );
                OObjectBase& rBaseObj = dynamic_cast<OObjectBase&>(*pObj);
                try
                {
                    rBaseObj.getReportComponent()->setPropertyValue(PROPERTY_OPAQUE,uno::makeAny(_nLayerNo == RPT_LAYER_FRONT));
                }
                catch(const uno::Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("reportdesign");
                }
            }
        }

        EndUndo();

        // check mark list now instead of later in a timer
        CheckMarked();
        MarkListHasChanged();
    }
}

bool OSectionView::OnlyShapesMarked() const
{
    const SdrMarkList& rMark = GetMarkedObjectList();
    const size_t nCount = rMark.GetMarkCount();
    if ( !nCount )
        return false;
    for (size_t i = 0; i<nCount; ++i)
    {
        SdrObject* pObj = rMark.GetMark(i)->GetMarkedSdrObj();
        if ( dynamic_cast< const OCustomShape *>( pObj ) ==  nullptr )
        {
            return false;
        }
    }
    return true;
}

bool OSectionView::IsDragResize() const
{
    const SdrDragMethod* pDragMethod = GetDragMethod();
    if (pDragMethod)
    {
        bool bMoveOnly = pDragMethod->getMoveOnly();
        if (!bMoveOnly)
        {
            // current marked components will be resized
            return true;
        }
    }
    return false;
}


SdrLayerID OSectionView::GetLayerIdOfMarkedObjects() const
{
    SdrLayerID nRet = SDRLAYER_NOTFOUND;
    const SdrMarkList &rMrkList = GetMarkedObjectList();
    for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        const SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        if ( nRet == SDRLAYER_NOTFOUND )
            nRet = pObj->GetLayer();
        else if ( nRet != pObj->GetLayer() )
        {
            break;
        }
    }
    return nRet;
}


} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
