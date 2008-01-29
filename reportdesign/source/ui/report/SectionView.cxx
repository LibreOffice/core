/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SectionView.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:51:43 $
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
#include "precompiled_reportdesign.hxx"

#ifndef _REPORT_SECTIONVIEW_HXX
#include "SectionView.hxx"
#endif
#ifndef RPT_DESIGNVIEW_HXX
#include "DesignView.hxx"
#endif

#ifndef _REPORT_RPTUIPAGE_HXX
#include <RptPage.hxx>
#endif
#ifndef _REPORT_RPTUIOBJ_HXX
#include <RptObject.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef REPORT_REPORTSECTION_HXX
#include "ReportSection.hxx"
#endif
#ifndef RPTUI_REPORT_WINDOW_HXX
#include "ReportWindow.hxx"
#endif


namespace rptui
{
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
    SetBordVisibleOnlyLeftRight();
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

    if ( m_pReportWindow )
    {
        //m_pReportWindow->unmarkAllObjects(this); // WHY
        DlgEdHint aHint( RPTUI_HINT_SELECTIONCHANGED );
        m_pReportWindow->getReportView()->Broadcast( aHint );
        m_pReportWindow->getReportView()->UpdatePropertyBrowserDelayed(this);
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
//============================================================================
} // rptui
//============================================================================
