/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SectionWindow.cxx,v $
 * $Revision: 1.2 $
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
#include "SectionWindow.hxx"
#include "ReportWindow.hxx"
#include "ReportRuler.hxx"
#include "rptui_slotid.hrc"
#include <svtools/colorcfg.hxx>
#include "ReportController.hxx"
#include "SectionView.hxx"
#include "RptDef.hxx"
#include "ReportSection.hxx"
#include "DesignView.hxx"
#include "uistrings.hrc"
#include "helpids.hrc"
#include "RptResId.hrc"
#include <boost/bind.hpp>
#include <functional>
#include <algorithm>
#include <vcl/svapp.hxx>
#include "StartMarker.hxx"
#include "EndMarker.hxx"
#include "ViewsWindow.hxx"

namespace rptui
{
#define SECTION_OFFSET  3

using namespace ::com::sun::star;
using namespace ::comphelper;

DBG_NAME( rpt_OSectionWindow );
OSectionWindow::OSectionWindow( OViewsWindow* _pParent,const uno::Reference< report::XSection >& _xSection,const ::rtl::OUString& _sColorEntry)
: Window( _pParent,WB_DIALOGCONTROL)
,OPropertyChangeListener(m_aMutex)
,m_pParent(_pParent)
,m_aStartMarker( this,_sColorEntry)
,m_aReportSection( this,_xSection)
,m_aSplitter(this)
,m_aEndMarker( this,_sColorEntry)
{
    DBG_CTOR( rpt_OSectionWindow,NULL);
    SetUniqueId(UID_RPT_SECTIONSWINDOW);
    SetMapMode( MapMode( MAP_100TH_MM ) );
    EnableMapMode();
    ImplInitSettings();

    // TRY
    m_aSplitter.SetMapMode( MapMode( MAP_100TH_MM ) );
    m_aSplitter.EnableMapMode();

    m_aSplitter.SetStartSplitHdl(LINK(this, OSectionWindow,StartSplitHdl));
    m_aSplitter.SetSplitHdl(LINK(this, OSectionWindow,SplitHdl));
    m_aSplitter.SetEndSplitHdl(LINK(this, OSectionWindow,EndSplitHdl));
    m_aSplitter.SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
    m_aSplitter.SetSplitPosPixel(m_aSplitter.LogicToPixel(Size(0,_xSection->getHeight())).Height());
    m_aSplitter.Show();

    m_aStartMarker.setCollapsedHdl(LINK(this,OSectionWindow,Collapsed));

    m_aStartMarker.Show();
    m_aReportSection.Show();
    m_aEndMarker.Show();
    Show();

    m_pSectionMulti = new OPropertyChangeMultiplexer(this,_xSection.get());
    m_pSectionMulti->addProperty(PROPERTY_NAME);
    m_pSectionMulti->addProperty(PROPERTY_HEIGHT);

    beans::PropertyChangeEvent aEvent;
    aEvent.Source = _xSection;
    aEvent.PropertyName = PROPERTY_NAME;
    uno::Reference< report::XGroup > xGroup(_xSection->getGroup());
    if ( xGroup.is() )
    {
        m_pGroupMulti = new OPropertyChangeMultiplexer(this,xGroup.get());
        m_pGroupMulti->addProperty(PROPERTY_EXPRESSION);
        aEvent.Source = xGroup;
        aEvent.PropertyName = PROPERTY_EXPRESSION;
    }

    _propertyChanged(aEvent);
    SetPaintTransparent(TRUE);
}
// -----------------------------------------------------------------------------
OSectionWindow::~OSectionWindow()
{
    DBG_DTOR( rpt_OSectionWindow,NULL);
    try
    {
        if ( m_pSectionMulti.is() )
            m_pSectionMulti->dispose();
        if ( m_pGroupMulti.is() )
            m_pGroupMulti->dispose();
    }
    catch (uno::Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void OSectionWindow::_propertyChanged(const beans::PropertyChangeEvent& _rEvent) throw( uno::RuntimeException)
{
    const uno::Reference< report::XSection > xSection(_rEvent.Source,uno::UNO_QUERY);
    if ( xSection.is() )
    {
        const uno::Reference< report::XSection> xCurrentSection = m_aReportSection.getSection();
        if ( _rEvent.PropertyName.equals(PROPERTY_HEIGHT) )
        {
            m_pParent->getView()->SetUpdateMode(FALSE);
            Resize();
            m_pParent->getView()->notifyHeightChanged();
            m_pParent->resize(*this);
            m_pParent->getView()->SetUpdateMode(TRUE);
            m_aStartMarker.Invalidate(INVALIDATE_NOERASE);
            m_aEndMarker.Invalidate(INVALIDATE_NOERASE);
            m_aReportSection.Invalidate(/*INVALIDATE_NOERASE*/);
            // Invalidate(INVALIDATE_NOCHILDREN | INVALIDATE_TRANSPARENT);
            // m_pParent->Invalidate(INVALIDATE_NOCHILDREN|INVALIDATE_NOERASE|INVALIDATE_TRANSPARENT);
            // m_pParent->Invalidate(/*INVALIDATE_NOCHILDREN | INVALIDATE_NOERASE |*/ INVALIDATE_NOCHILDREN | INVALIDATE_TRANSPARENT);
        }
        else if ( _rEvent.PropertyName.equals(PROPERTY_NAME) && !xSection->getGroup().is() )
        {
            m_aStartMarker.setTitle(xSection->getName());
            m_aStartMarker.Invalidate(INVALIDATE_NOERASE);
        }
    } // if ( xSection.is() )
    else if ( _rEvent.PropertyName.equals(PROPERTY_EXPRESSION) )
    {
        uno::Reference< report::XGroup > xGroup(_rEvent.Source,uno::UNO_QUERY);
        if ( xGroup.is() )
        {
            setGroupSectionTitle(xGroup,RID_STR_HEADER,::std::mem_fun(&OGroupHelper::getHeader),::std::mem_fun(&OGroupHelper::getHeaderOn));
            setGroupSectionTitle(xGroup,RID_STR_FOOTER,::std::mem_fun(&OGroupHelper::getFooter),::std::mem_fun(&OGroupHelper::getFooterOn));
        }
    }
}
// -----------------------------------------------------------------------------
void OSectionWindow::setGroupSectionTitle(const uno::Reference< report::XGroup>& _xGroup,USHORT _nResId,::std::mem_fun_t<uno::Reference<report::XSection> , OGroupHelper> _pGetSection,::std::mem_fun_t<sal_Bool,OGroupHelper> _pIsSectionOn)
{
    OGroupHelper aGroupHelper(_xGroup);
    if ( _pIsSectionOn(&aGroupHelper) )
    {
        uno::Reference< report::XSection > xSection = _pGetSection(&aGroupHelper);
        String sTitle = String(ModuleRes(_nResId));
        sTitle.SearchAndReplace('#',_xGroup->getExpression());
        m_aStartMarker.setTitle(sTitle);
        m_aStartMarker.Invalidate(INVALIDATE_CHILDREN);
    }
}
//------------------------------------------------------------------------------
void OSectionWindow::ImplInitSettings()
{
    SetBackground( );
    // SetBackground( Wallpaper( COL_RED ));
}
//-----------------------------------------------------------------------------
void OSectionWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}
//------------------------------------------------------------------------------
void OSectionWindow::Resize()
{
    Window::Resize();

    //const Point aOffset = LogicToPixel( Point( SECTION_OFFSET, SECTION_OFFSET ), MAP_APPFONT );

    Size aOutputSize = GetOutputSizePixel();
    Fraction aEndWidth(long(REPORT_ENDMARKER_WIDTH));
    aEndWidth *= GetMapMode().GetScaleX();

    const Point aOffset = m_pParent->getView()->getScrollOffset();
    aOutputSize.Width() -= aOffset.X();
    aOutputSize.Height() -=  m_aSplitter.GetSizePixel().Height();

    if ( m_aStartMarker.isCollapsed() )
    {
        Point aPos(0,0);
        m_aStartMarker.SetPosSizePixel(aPos,aOutputSize);
    }
    else
    {
        const bool bShowEndMarker = m_pParent->getView()->GetTotalWidth() <= (aOffset.X() +  aOutputSize.Width() );

        Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH));
        aStartWidth *= GetMapMode().GetScaleX();

        // set start marker
        m_aStartMarker.SetPosSizePixel(Point(0,0),Size(aStartWidth,aOutputSize.Height()));

        // set report section
        const uno::Reference< report::XSection> xSection = m_aReportSection.getSection();
        Size aSectionSize = LogicToPixel( Size( 0,xSection->getHeight() ) );
        Point aReportPos(aStartWidth,0);
        aSectionSize.Width() = aOutputSize.Width() - (long)aStartWidth;
        if ( bShowEndMarker )
            aSectionSize.Width() -= (long)aEndWidth;
        m_aReportSection.SetPosSizePixel(aReportPos,aSectionSize);

        // set splitter
        aReportPos.Y() += aSectionSize.Height();
        m_aSplitter.SetPosSizePixel(aReportPos,Size(aSectionSize.Width(),m_aSplitter.GetSizePixel().Height()));
        aSectionSize.Height() = (long)(1000 * (double)GetMapMode().GetScaleY());
        m_aSplitter.SetDragRectPixel( Rectangle(Point(aStartWidth,0),aSectionSize));

        // set end marker
        aReportPos.X() += aSectionSize.Width();
        aReportPos.Y() = 0;
        m_aEndMarker.Show(bShowEndMarker);
        m_aEndMarker.SetPosSizePixel(aReportPos,Size(aEndWidth,aOutputSize.Height()));
    }
}
// -----------------------------------------------------------------------------
void OSectionWindow::setCollapsed(sal_Bool _bCollapsed)
{
    m_aReportSection.Show(_bCollapsed);
    m_aEndMarker.Show/*setCollapsed*/(_bCollapsed);
    m_aSplitter.Show(_bCollapsed);
}
//-----------------------------------------------------------------------------
void OSectionWindow::showProperties()
{
    m_pParent->getView()->showProperties( m_aReportSection.getSection().get() );
}
//-----------------------------------------------------------------------------
void OSectionWindow::setMarked(sal_Bool _bMark)
{
    m_aStartMarker.setMarked(_bMark);
    m_aEndMarker.setMarked(_bMark);
}
// -----------------------------------------------------------------------------
IMPL_LINK( OSectionWindow, Collapsed, OStartMarker *, _pMarker )
{
    if ( _pMarker )
    {
        setCollapsed(!_pMarker->isCollapsed());
        m_pParent->resize(*this);
        Resize();
        // TRY
        // m_pParent->Invalidate(INVALIDATE_TRANSPARENT | INVALIDATE_NOCHILDREN);
        Invalidate();
        // _pMarker->Invalidate();
    }
    return 0L;
}
// -----------------------------------------------------------------------------
void OSectionWindow::zoom(const sal_Int16 _nZoom)
{
    setZoomFactor(_nZoom,*this);
    m_aStartMarker.zoom(_nZoom);
    setZoomFactor(_nZoom,m_aReportSection);
    setZoomFactor(_nZoom,m_aSplitter);
    setZoomFactor(_nZoom,m_aEndMarker);
    Resize();
    Invalidate(/*INVALIDATE_UPDATE |*/ /* | INVALIDATE_TRANSPARENT *//*INVALIDATE_NOCHILDREN*/);
}
//-----------------------------------------------------------------------------
IMPL_LINK( OSectionWindow, StartSplitHdl, Splitter*,  )
{
    const String sEmpty(ModuleRes(RID_STR_UNDO_CHANGE_SIZE));
    getViewsWindow()->getView()->getReportView()->getController().getUndoMgr()->EnterListAction(sEmpty,String());
    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK( OSectionWindow, EndSplitHdl, Splitter*,  )
{
    getViewsWindow()->getView()->getReportView()->getController().getUndoMgr()->LeaveListAction();
    /*getViewsWindow()->Resize();*/
    return 0L;
}
//-----------------------------------------------------------------------------
IMPL_LINK( OSectionWindow, SplitHdl, Splitter*, _pSplitter )
{
    if ( !getViewsWindow()->getView()->getReportView()->getController().isEditable() )
    {
        return 0L;
    }

    sal_Int32 nSplitPos = _pSplitter->GetSplitPosPixel();
    const Point aPos = _pSplitter->GetPosPixel();
    _pSplitter->SetPosPixel( Point( aPos.X(),nSplitPos ));

    const uno::Reference< report::XSection> xSection = m_aReportSection.getSection();
    nSplitPos = m_aSplitter.PixelToLogic(Size(0,nSplitPos)).Height();
    // nSplitPos = xSection->getHeight() + m_aSplitter.PixelToLogic(Size(0,nSplitPos - aPos.Y() )).Height();

    const sal_Int32 nCount = xSection->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference<report::XReportComponent> xReportComponent(xSection->getByIndex(i),uno::UNO_QUERY);
        if ( xReportComponent.is() && nSplitPos < (xReportComponent->getPositionY() + xReportComponent->getHeight()) )
        {
            nSplitPos = xReportComponent->getPositionY() + xReportComponent->getHeight();
            break;
        }
    }

    //nSplitPos += xSection->getHeight();
    if ( nSplitPos < 0 )
        nSplitPos = 0;

    xSection->setHeight(nSplitPos);

    return 0L;
}
// -----------------------------------------------------------------------------
void lcl_scroll(Window& _rWindow,const Size& _aDelta)
{
    _rWindow.Scroll(-_aDelta.Width(),-_aDelta.Height(),SCROLL_CHILDREN/*|SCROLL_CLIP*/);
    _rWindow.Invalidate(INVALIDATE_TRANSPARENT);
}
// -----------------------------------------------------------------------------
void lcl_setMapMode(Window& _rWindow,long _nDeltaX, long _nDeltaY)
{
    MapMode aMap = _rWindow.GetMapMode();
    Point aOrg = aMap.GetOrigin();
    aMap.SetOrigin( Point(aOrg.X() - _nDeltaX, aOrg.Y() - _nDeltaY));
    _rWindow.SetMapMode( aMap );
}
//----------------------------------------------------------------------------
void OSectionWindow::scrollChildren(long _nDeltaX)
{
    const Size aDelta( PixelToLogic(Size(_nDeltaX,0)) );
    lcl_setMapMode(m_aReportSection,aDelta.Width(), 0);
    {
        OWindowPositionCorrector aCorrector(&m_aReportSection,-_nDeltaX,0);
        lcl_scroll(m_aReportSection,aDelta);
    }

    // lcl_setMapMode(m_aEndMarker,_nDeltaX, 0);
    lcl_scroll(m_aEndMarker,aDelta);

    lcl_setMapMode(m_aSplitter,_nDeltaX, 0);
    lcl_scroll(m_aSplitter,aDelta);

    Resize();
}
//==============================================================================
} // rptui
//==============================================================================

