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
#include "precompiled_reportdesign.hxx"
#include "SectionView.hxx"
#include "DesignView.hxx"
#include <RptPage.hxx>
#include <RptObject.hxx>
#include <RptDef.hxx>
#include <svx/svxids.hrc>
#include <svx/svddrgmt.hxx>
#include <vcl/scrbar.hxx>
#include "ReportSection.hxx"
#include "ReportWindow.hxx"
#include "uistrings.hrc"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

namespace rptui
{
    using namespace ::com::sun::star;
TYPEINIT1( OSectionView, SdrView );

//----------------------------------------------------------------------------
DBG_NAME( rpt_OSectionView )
OSectionView::OSectionView( SdrModel* pModel, OReportSection* _pSectionWindow, OReportWindow* pEditor )
    :SdrView( pModel, _pSectionWindow )
    ,m_pReportWindow( pEditor )
    ,m_pSectionWindow(_pSectionWindow)
{
    DBG_CTOR( rpt_OSectionView,NULL);
    SetBufferedOutputAllowed(true);
    SetBufferedOverlayAllowed(true);
    SetPageBorderVisible(false);
    SetBordVisible();
    SetQuickTextEditMode(FALSE);
}

//----------------------------------------------------------------------------

OSectionView::~OSectionView()
{
    DBG_DTOR( rpt_OSectionView,NULL);
}

//----------------------------------------------------------------------------

void OSectionView::MarkListHasChanged()
{
    DBG_CHKTHIS( rpt_OSectionView,NULL);
    SdrView::MarkListHasChanged();

    if ( m_pReportWindow && m_pSectionWindow && !m_pSectionWindow->getPage()->getSpecialMode() )
    {
        DlgEdHint aHint( RPTUI_HINT_SELECTIONCHANGED );
        m_pReportWindow->getReportView()->Broadcast( aHint );
        m_pReportWindow->getReportView()->UpdatePropertyBrowserDelayed(*this);
    }
}

//----------------------------------------------------------------------------

void OSectionView::MakeVisible( const Rectangle& rRect, Window& rWin )
{
    DBG_CHKTHIS( rpt_OSectionView,NULL);
    // visible area
    MapMode aMap( rWin.GetMapMode() );
    const Point aOrg( aMap.GetOrigin() );
    const Size aVisSize( rWin.GetOutputSize() );
    const Rectangle aVisRect( Point(-aOrg.X(),-aOrg.Y()), aVisSize );

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
        rWin.Invalidate(INVALIDATE_NOERASE);
    }
}
//------------------------------------------------------------------------------
void OSectionView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    DBG_CHKTHIS( rpt_OSectionView,NULL);
    SdrView::Notify(rBC,rHint);
    if ( rHint.ISA(SdrHint) )
    {
        const SdrObject* pObj = ((SdrHint&)rHint).GetObject();
        const SdrHintKind eKind = ((SdrHint&)rHint).GetKind();
        // check for change of selected object
        if(HINT_OBJCHG == eKind && pObj && IsObjMarked(const_cast<SdrObject*>(pObj)))
            AdjustMarkHdl();
        else if ( eKind == HINT_OBJREMOVED )
            ObjectRemovedInAliveMode(pObj);
    }
}

//------------------------------------------------------------------------------
void OSectionView::ObjectRemovedInAliveMode( const SdrObject* _pObject )
{
    DBG_CHKTHIS( rpt_OSectionView,NULL);
    const SdrMarkList& rMarkedList = GetMarkedObjectList();
    const ULONG nMark = rMarkedList.GetMarkCount();

    for( ULONG i = 0; i < nMark; i++ )
    {
        SdrObject* pSdrObj = rMarkedList.GetMark(i)->GetMarkedSdrObj();
        if (_pObject == pSdrObj)
        {
            SdrPageView*    pPgView = GetSdrPageView();
            BrkAction();
            MarkObj( pSdrObj, pPgView, TRUE );
            break;
        }
    }
}

// -----------------------------------------------------------------------------
void OSectionView::SetMarkedToLayer( SdrLayerID _nLayerNo )
{
    if (AreObjectsMarked())
    {
        //  #i11702# use SdrUndoObjectLayerChange for undo
        //  STR_UNDO_SELATTR is "Attributes" - should use a different text later
        BegUndo( );

        const SdrMarkList& rMark = GetMarkedObjectList();
        ULONG nCount = rMark.GetMarkCount();
        for (ULONG i=0; i<nCount; i++)
        {
            SdrObject* pObj = rMark.GetMark(i)->GetMarkedSdrObj();
            if ( pObj->ISA(OCustomShape) )
            {
                AddUndo( new SdrUndoObjectLayerChange( *pObj, pObj->GetLayer(), _nLayerNo) );
                pObj->SetLayer( _nLayerNo );
                OObjectBase* pBaseObj = dynamic_cast<OObjectBase*>(pObj);
                try
                {
                    pBaseObj->getReportComponent()->setPropertyValue(PROPERTY_OPAQUE,uno::makeAny(_nLayerNo == RPT_LAYER_FRONT));
                }
                catch(const uno::Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }

        EndUndo();

        // check mark list now instead of later in a timer
        CheckMarked();
        MarkListHasChanged();
    }
}
// -----------------------------------------------------------------------------
bool OSectionView::OnlyShapesMarked() const
{
    const SdrMarkList& rMark = GetMarkedObjectList();
    const ULONG nCount = rMark.GetMarkCount();
    if ( !nCount )
        return false;
    ULONG i=0;
    for (; i<nCount; i++)
    {
        SdrObject* pObj = rMark.GetMark(i)->GetMarkedSdrObj();
        if ( !pObj->ISA(OCustomShape) )
        {
            break;
        }
    }
    return i == nCount;
}

bool OSectionView::IsDragResize() const
{
    const SdrDragMethod* pDragMethod = GetDragMethod();
    if (pDragMethod)
    {
        bool bMoveOnly = pDragMethod->getMoveOnly();
        if (bMoveOnly == false)
        {
            // current marked components will be resized
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
short OSectionView::GetLayerIdOfMarkedObjects() const
{
    short nRet = SHRT_MAX;
    const SdrMarkList &rMrkList = GetMarkedObjectList();
    for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        const SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        if ( nRet == SHRT_MAX )
            nRet = pObj->GetLayer();
        else if ( nRet != pObj->GetLayer() )
        {
            nRet = -1;
            break;
        }
    }
    if ( nRet == SHRT_MAX )
        nRet = -1;
    return nRet;
}

//============================================================================
} // rptui
//============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
