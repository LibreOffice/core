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
#include "SectionWindow.hxx"
#include "ReportWindow.hxx"
#include "ReportRuler.hxx"
#include "rptui_slotid.hrc"
#include "ReportController.hxx"
#include "SectionView.hxx"
#include "RptDef.hxx"
#include "ReportSection.hxx"
#include "DesignView.hxx"
#include "uistrings.hrc"
#include "helpids.hrc"
#include "RptResId.hrc"
#include "StartMarker.hxx"
#include "EndMarker.hxx"
#include "ViewsWindow.hxx"

#include <svtools/colorcfg.hxx>
#include <functional>
#include <algorithm>
#include <vcl/svapp.hxx>
#include <connectivity/dbtools.hxx>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::comphelper;

DBG_NAME( rpt_OSectionWindow )
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
    const MapMode& rMapMode = _pParent->GetMapMode();
    SetMapMode( rMapMode );
    ImplInitSettings();
    // TRY
    m_aSplitter.SetMapMode( MapMode( MAP_100TH_MM ) );
    m_aSplitter.SetStartSplitHdl(LINK(this, OSectionWindow,StartSplitHdl));
    m_aSplitter.SetSplitHdl(LINK(this, OSectionWindow,SplitHdl));
    m_aSplitter.SetEndSplitHdl(LINK(this, OSectionWindow,EndSplitHdl));
    m_aSplitter.SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
    m_aSplitter.SetSplitPosPixel(m_aSplitter.LogicToPixel(Size(0,_xSection->getHeight())).Height());


    m_aStartMarker.setCollapsedHdl(LINK(this,OSectionWindow,Collapsed));

    m_aStartMarker.zoom(rMapMode.GetScaleX());
    setZoomFactor(rMapMode.GetScaleX(),m_aReportSection);
    setZoomFactor(rMapMode.GetScaleX(),m_aSplitter);
    setZoomFactor(rMapMode.GetScaleX(),m_aEndMarker);

    m_aSplitter.Show();
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
            static bool t4 = true;
        if ( t4 )
            m_pParent->getView()->SetUpdateMode(sal_False);
            //Resize();
            m_pParent->getView()->notifySizeChanged();
            m_pParent->resize(*this);
            if ( t4 )
            m_pParent->getView()->SetUpdateMode(sal_True);
            // getViewsWindow()->getView()->getReportView()->getController().resetZoomType();
        }
        else if ( _rEvent.PropertyName.equals(PROPERTY_NAME) && !xSection->getGroup().is() )
        {
            uno::Reference< report::XReportDefinition > xReport = xSection->getReportDefinition();
            if (    setReportSectionTitle(xReport,RID_STR_REPORT_HEADER,::std::mem_fun(&OReportHelper::getReportHeader),::std::mem_fun(&OReportHelper::getReportHeaderOn))
                ||  setReportSectionTitle(xReport,RID_STR_REPORT_FOOTER,::std::mem_fun(&OReportHelper::getReportFooter),::std::mem_fun(&OReportHelper::getReportFooterOn))
                ||  setReportSectionTitle(xReport,RID_STR_PAGE_HEADER,::std::mem_fun(&OReportHelper::getPageHeader),::std::mem_fun(&OReportHelper::getPageHeaderOn))
                ||  setReportSectionTitle(xReport,RID_STR_PAGE_FOOTER,::std::mem_fun(&OReportHelper::getPageFooter),::std::mem_fun(&OReportHelper::getPageFooterOn)) )
            {
                m_aStartMarker.Invalidate(INVALIDATE_NOERASE);
            }
            else
            {
                String sTitle = String(ModuleRes(RID_STR_DETAIL));
                m_aStartMarker.setTitle(sTitle);
                m_aStartMarker.Invalidate(INVALIDATE_CHILDREN);
            }
        }
    }
    else if ( _rEvent.PropertyName.equals(PROPERTY_EXPRESSION) )
    {
        uno::Reference< report::XGroup > xGroup(_rEvent.Source,uno::UNO_QUERY);
        if ( xGroup.is() && !setGroupSectionTitle(xGroup,RID_STR_HEADER,::std::mem_fun(&OGroupHelper::getHeader),::std::mem_fun(&OGroupHelper::getHeaderOn)))
        {
            setGroupSectionTitle(xGroup,RID_STR_FOOTER,::std::mem_fun(&OGroupHelper::getFooter),::std::mem_fun(&OGroupHelper::getFooterOn));
        }
    }
}
// -----------------------------------------------------------------------------
bool OSectionWindow::setReportSectionTitle(const uno::Reference< report::XReportDefinition>& _xReport,sal_uInt16 _nResId,::std::mem_fun_t<uno::Reference<report::XSection> , OReportHelper> _pGetSection,::std::mem_fun_t<sal_Bool,OReportHelper> _pIsSectionOn)
{
    OReportHelper aReportHelper(_xReport);
    const bool bRet = _pIsSectionOn(&aReportHelper) && _pGetSection(&aReportHelper) == m_aReportSection.getSection();
    if ( bRet )
    {
        String sTitle = String(ModuleRes(_nResId));
        m_aStartMarker.setTitle(sTitle);
        m_aStartMarker.Invalidate(INVALIDATE_CHILDREN);
    }
    return bRet;
}
// -----------------------------------------------------------------------------
bool OSectionWindow::setGroupSectionTitle(const uno::Reference< report::XGroup>& _xGroup,sal_uInt16 _nResId,::std::mem_fun_t<uno::Reference<report::XSection> , OGroupHelper> _pGetSection,::std::mem_fun_t<sal_Bool,OGroupHelper> _pIsSectionOn)
{
    OGroupHelper aGroupHelper(_xGroup);
    const bool bRet = _pIsSectionOn(&aGroupHelper) && _pGetSection(&aGroupHelper) == m_aReportSection.getSection() ;
    if ( bRet )
    {
        ::rtl::OUString sExpression = _xGroup->getExpression();
        ::rtl::OUString sLabel = getViewsWindow()->getView()->getReportView()->getController().getColumnLabel_throw(sExpression);
        if ( !sLabel.isEmpty() )
        {
            sExpression = sLabel;
        }

        String sTitle = ModuleRes(_nResId).toString();
        sTitle.SearchAndReplace(rtl::OUString('#') ,sExpression);
        m_aStartMarker.setTitle(sTitle);
        m_aStartMarker.Invalidate(INVALIDATE_CHILDREN);
    }
    return bRet;
}
//------------------------------------------------------------------------------
void OSectionWindow::ImplInitSettings()
{
    static bool t = false;
    if ( t )
    {
    EnableChildTransparentMode( sal_True );
    SetParentClipMode( PARENTCLIPMODE_NOCLIP );
    SetPaintTransparent( sal_True );
    }
    SetBackground( );
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

    Size aOutputSize = GetOutputSizePixel();
    Fraction aEndWidth(long(REPORT_ENDMARKER_WIDTH));
    aEndWidth *= GetMapMode().GetScaleX();

    const Point aThumbPos = m_pParent->getView()->getThumbPos();
    aOutputSize.Width() -= aThumbPos.X();
    aOutputSize.Height() -=  m_aSplitter.GetSizePixel().Height();

    if ( m_aStartMarker.isCollapsed() )
    {
        Point aPos(0,0);
        m_aStartMarker.SetPosSizePixel(aPos,aOutputSize);
    }
    else
    {
        const bool bShowEndMarker = m_pParent->getView()->GetTotalWidth() <= (aThumbPos.X() +  aOutputSize.Width() );

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
    if ( m_aStartMarker.isCollapsed() != _bCollapsed )
    {
        m_aStartMarker.setCollapsed(_bCollapsed);
    }
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
IMPL_LINK( OSectionWindow, Collapsed, OColorListener *, _pMarker )
{
    if ( _pMarker )
    {
        sal_Bool bShow = !_pMarker->isCollapsed();
        m_aReportSection.Show(bShow);
        m_aEndMarker.Show(bShow);
        m_aSplitter.Show(bShow);

        m_pParent->resize(*this);
    }
    return 0L;
}
// -----------------------------------------------------------------------------
void OSectionWindow::zoom(const Fraction& _aZoom)
{
    setZoomFactor(_aZoom,*this);
    m_aStartMarker.zoom(_aZoom);

    setZoomFactor(_aZoom,m_aReportSection);
    setZoomFactor(_aZoom,m_aSplitter);
    setZoomFactor(_aZoom,m_aEndMarker);
    Invalidate();
}
//-----------------------------------------------------------------------------
IMPL_LINK( OSectionWindow, StartSplitHdl, Splitter*,  )
{
    const String sUndoAction( ModuleRes( RID_STR_UNDO_CHANGE_SIZE ) );
    getViewsWindow()->getView()->getReportView()->getController().getUndoManager().EnterListAction( sUndoAction, String() );
    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK( OSectionWindow, EndSplitHdl, Splitter*,  )
{
    getViewsWindow()->getView()->getReportView()->getController().getUndoManager().LeaveListAction();
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

    const uno::Reference< report::XSection> xSection = m_aReportSection.getSection();
    nSplitPos = m_aSplitter.PixelToLogic(Size(0,nSplitPos)).Height();

    const sal_Int32 nCount = xSection->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference<report::XReportComponent> xReportComponent(xSection->getByIndex(i),uno::UNO_QUERY);
        if ( xReportComponent.is() )
        {
            nSplitPos = ::std::max(nSplitPos,xReportComponent->getPositionY() + xReportComponent->getHeight());
        }
    }

    if ( nSplitPos < 0 )
        nSplitPos = 0;

    xSection->setHeight(nSplitPos);
    m_aSplitter.SetSplitPosPixel(m_aSplitter.LogicToPixel(Size(0,nSplitPos)).Height());

    return 0L;
}
// -----------------------------------------------------------------------------
void lcl_scroll(Window& _rWindow,const Point& _aDelta)
{
    _rWindow.Scroll(-_aDelta.X(),-_aDelta.Y());
    _rWindow.Invalidate(INVALIDATE_TRANSPARENT);
}
// -----------------------------------------------------------------------------
void lcl_setOrigin(Window& _rWindow,long _nX, long _nY)
{
    MapMode aMap = _rWindow.GetMapMode();
    aMap.SetOrigin( Point(- _nX, - _nY));
    _rWindow.SetMapMode( aMap );
}
//----------------------------------------------------------------------------
void OSectionWindow::scrollChildren(long _nX)
{
    const Point aDelta( _nX,0 );

    MapMode aMapMode( m_aReportSection.GetMapMode() );
    const Point aOld = aMapMode.GetOrigin();
    lcl_setOrigin(m_aReportSection,aDelta.X(), 0);

    aMapMode = m_aReportSection.GetMapMode();
    const Point aNew = aMapMode.GetOrigin();
    const Point aDiff = aOld - aNew;
    {
        lcl_scroll(m_aReportSection,aDiff);
    }

    lcl_scroll(m_aEndMarker,m_aEndMarker.PixelToLogic(Point(_nX,0)));

    lcl_setOrigin(m_aSplitter,_nX, 0);
    lcl_scroll(m_aSplitter,aDiff);
}
//==============================================================================
} // rptui
//==============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
