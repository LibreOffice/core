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
#include "rptui_slotid.hrc"
#include "ReportController.hxx"
#include "SectionView.hxx"
#include "RptDef.hxx"
#include "ReportSection.hxx"
#include "DesignView.hxx"
#include "strings.hxx"
#include "core_resource.hxx"
#include "helpids.hrc"
#include "strings.hrc"
#include "StartMarker.hxx"
#include "EndMarker.hxx"
#include "ViewsWindow.hxx"

#include <svtools/colorcfg.hxx>
#include <functional>
#include <algorithm>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <connectivity/dbtools.hxx>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::comphelper;

OSectionWindow::OSectionWindow( OViewsWindow* _pParent,const uno::Reference< report::XSection >& _xSection,const OUString& _sColorEntry)
: Window( _pParent,WB_DIALOGCONTROL)
,OPropertyChangeListener(m_aMutex)
,m_pParent(_pParent)
,m_aStartMarker( VclPtr<rptui::OStartMarker>::Create(this,_sColorEntry))
,m_aReportSection( VclPtr<rptui::OReportSection>::Create(this,_xSection))
,m_aSplitter(VclPtr<Splitter>::Create(this))
,m_aEndMarker( VclPtr<rptui::OEndMarker>::Create(this,_sColorEntry))
{
    const MapMode& rMapMode = _pParent->GetMapMode();
    SetMapMode( rMapMode );
    ImplInitSettings();
    // TRY
    m_aSplitter->SetMapMode( MapMode( MapUnit::Map100thMM ) );
    m_aSplitter->SetStartSplitHdl(LINK(this, OSectionWindow,StartSplitHdl));
    m_aSplitter->SetSplitHdl(LINK(this, OSectionWindow,SplitHdl));
    m_aSplitter->SetEndSplitHdl(LINK(this, OSectionWindow,EndSplitHdl));
    m_aSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
    m_aSplitter->SetSplitPosPixel(m_aSplitter->LogicToPixel(Size(0,_xSection->getHeight())).Height());


    m_aStartMarker->setCollapsedHdl(LINK(this,OSectionWindow,Collapsed));

    m_aStartMarker->zoom(rMapMode.GetScaleX());
    setZoomFactor(rMapMode.GetScaleX(), *m_aReportSection.get());
    setZoomFactor(rMapMode.GetScaleX(), *m_aSplitter.get());
    setZoomFactor(rMapMode.GetScaleX(), *m_aEndMarker.get());

    m_aSplitter->Show();
    m_aStartMarker->Show();
    m_aReportSection->Show();
    m_aEndMarker->Show();
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

OSectionWindow::~OSectionWindow()
{
    disposeOnce();
}

void OSectionWindow::dispose()
{
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
    m_aStartMarker.disposeAndClear();
    m_aReportSection.disposeAndClear();
    m_aSplitter.disposeAndClear();
    m_aEndMarker.disposeAndClear();
    m_pParent.clear();
    vcl::Window::dispose();
}

void OSectionWindow::_propertyChanged(const beans::PropertyChangeEvent& _rEvent)
{
    SolarMutexGuard g;
    const uno::Reference< report::XSection > xSection(_rEvent.Source,uno::UNO_QUERY);
    if ( xSection.is() )
    {
        if ( _rEvent.PropertyName == PROPERTY_HEIGHT )
        {
            m_pParent->getView()->SetUpdateMode(false);
            //Resize();
            m_pParent->getView()->notifySizeChanged();
            m_pParent->resize(*this);
            m_pParent->getView()->SetUpdateMode(true);
            // getViewsWindow()->getView()->getReportView()->getController().resetZoomType();
        }
        else if ( _rEvent.PropertyName == PROPERTY_NAME && !xSection->getGroup().is() )
        {
            uno::Reference< report::XReportDefinition > xReport = xSection->getReportDefinition();
            if (    setReportSectionTitle(xReport,RID_STR_REPORT_HEADER,::std::mem_fun(&OReportHelper::getReportHeader),::std::mem_fun(&OReportHelper::getReportHeaderOn))
                ||  setReportSectionTitle(xReport,RID_STR_REPORT_FOOTER,::std::mem_fun(&OReportHelper::getReportFooter),::std::mem_fun(&OReportHelper::getReportFooterOn))
                ||  setReportSectionTitle(xReport,RID_STR_PAGE_HEADER,::std::mem_fun(&OReportHelper::getPageHeader),::std::mem_fun(&OReportHelper::getPageHeaderOn))
                ||  setReportSectionTitle(xReport,RID_STR_PAGE_FOOTER,::std::mem_fun(&OReportHelper::getPageFooter),::std::mem_fun(&OReportHelper::getPageFooterOn)) )
            {
                m_aStartMarker->Invalidate(InvalidateFlags::NoErase);
            }
            else
            {
                OUString sTitle = RptResId(RID_STR_DETAIL);
                m_aStartMarker->setTitle(sTitle);
                m_aStartMarker->Invalidate(InvalidateFlags::Children);
            }
        }
    }
    else if ( _rEvent.PropertyName == PROPERTY_EXPRESSION )
    {
        uno::Reference< report::XGroup > xGroup(_rEvent.Source,uno::UNO_QUERY);
        if ( xGroup.is() && !setGroupSectionTitle(xGroup,RID_STR_HEADER,::std::mem_fun(&OGroupHelper::getHeader),::std::mem_fun(&OGroupHelper::getHeaderOn)))
        {
            setGroupSectionTitle(xGroup,RID_STR_FOOTER,::std::mem_fun(&OGroupHelper::getFooter),::std::mem_fun(&OGroupHelper::getFooterOn));
        }
    }
}

bool OSectionWindow::setReportSectionTitle(const uno::Reference< report::XReportDefinition>& _xReport,const char* pResId,::std::mem_fun_t<uno::Reference<report::XSection> , OReportHelper> _pGetSection, const ::std::mem_fun_t<bool,OReportHelper>& _pIsSectionOn)
{
    OReportHelper aReportHelper(_xReport);
    const bool bRet = _pIsSectionOn(&aReportHelper) && _pGetSection(&aReportHelper) == m_aReportSection->getSection();
    if ( bRet )
    {
        OUString sTitle = RptResId(pResId);
        m_aStartMarker->setTitle(sTitle);
        m_aStartMarker->Invalidate(InvalidateFlags::Children);
    }
    return bRet;
}

bool OSectionWindow::setGroupSectionTitle(const uno::Reference< report::XGroup>& _xGroup,const char* pResId,::std::mem_fun_t<uno::Reference<report::XSection> , OGroupHelper> _pGetSection, const ::std::mem_fun_t<bool,OGroupHelper>& _pIsSectionOn)
{
    OGroupHelper aGroupHelper(_xGroup);
    const bool bRet = _pIsSectionOn(&aGroupHelper) && _pGetSection(&aGroupHelper) == m_aReportSection->getSection() ;
    if ( bRet )
    {
        OUString sExpression = _xGroup->getExpression();
        OUString sLabel = getViewsWindow()->getView()->getReportView()->getController().getColumnLabel_throw(sExpression);
        if ( !sLabel.isEmpty() )
        {
            sExpression = sLabel;
        }

        OUString sTitle(RptResId(pResId));
        sTitle = sTitle.replaceFirst("#", sExpression);
        m_aStartMarker->setTitle( sTitle );
        m_aStartMarker->Invalidate(InvalidateFlags::Children);
    }
    return bRet;
}

void OSectionWindow::ImplInitSettings()
{
    static bool t = false;
    if ( t )
    {
    EnableChildTransparentMode();
    SetParentClipMode( ParentClipMode::NoClip );
    SetPaintTransparent( true );
    }
    SetBackground( );
}

void OSectionWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OSectionWindow::Resize()
{
    Window::Resize();

    Size aOutputSize = GetOutputSizePixel();
    Fraction aEndWidth(long(REPORT_ENDMARKER_WIDTH));
    aEndWidth *= GetMapMode().GetScaleX();

    const Point aThumbPos = m_pParent->getView()->getThumbPos();
    aOutputSize.Width() -= aThumbPos.X();
    aOutputSize.Height() -=  m_aSplitter->GetSizePixel().Height();

    if ( m_aStartMarker->isCollapsed() )
    {
        Point aPos(0,0);
        m_aStartMarker->SetPosSizePixel(aPos,aOutputSize);
    }
    else
    {
        const bool bShowEndMarker = m_pParent->getView()->GetTotalWidth() <= (aThumbPos.X() +  aOutputSize.Width() );

        Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH));
        aStartWidth *= GetMapMode().GetScaleX();

        // set start marker
        m_aStartMarker->SetPosSizePixel(Point(0,0),Size(aStartWidth,aOutputSize.Height()));

        // set report section
        const uno::Reference< report::XSection> xSection = m_aReportSection->getSection();
        Size aSectionSize = LogicToPixel( Size( 0,xSection->getHeight() ) );
        Point aReportPos(aStartWidth,0);
        aSectionSize.Width() = aOutputSize.Width() - (long)aStartWidth;
        if ( bShowEndMarker )
            aSectionSize.Width() -= (long)aEndWidth;

        m_aReportSection->SetPosSizePixel(aReportPos,aSectionSize);

        // set splitter
        aReportPos.Y() += aSectionSize.Height();
        m_aSplitter->SetPosSizePixel(aReportPos,Size(aSectionSize.Width(),m_aSplitter->GetSizePixel().Height()));
        aSectionSize.Height() = (long)(1000 * (double)GetMapMode().GetScaleY());
        m_aSplitter->SetDragRectPixel( tools::Rectangle(Point(aStartWidth,0),aSectionSize));

        // set end marker
        aReportPos.X() += aSectionSize.Width();
        aReportPos.Y() = 0;
        m_aEndMarker->Show(bShowEndMarker);
        m_aEndMarker->SetPosSizePixel(aReportPos,Size(aEndWidth,aOutputSize.Height()));
    }
}

void OSectionWindow::setCollapsed(bool _bCollapsed)
{
    if ( m_aStartMarker->isCollapsed() != _bCollapsed )
    {
        m_aStartMarker->setCollapsed(_bCollapsed);
    }
}

void OSectionWindow::showProperties()
{
    m_pParent->getView()->showProperties( m_aReportSection->getSection().get() );
}

void OSectionWindow::setMarked(bool _bMark)
{
    m_aStartMarker->setMarked(_bMark);
    m_aEndMarker->setMarked(_bMark);
}

IMPL_LINK( OSectionWindow, Collapsed, OColorListener&, _rMarker, void )
{
    bool bShow = !_rMarker.isCollapsed();
    m_aReportSection->Show(bShow);
    m_aEndMarker->Show(bShow);
    m_aSplitter->Show(bShow);

    m_pParent->resize(*this);
}

void OSectionWindow::zoom(const Fraction& _aZoom)
{
    setZoomFactor(_aZoom,*this);
    m_aStartMarker->zoom(_aZoom);

    setZoomFactor(_aZoom, *m_aReportSection.get());
    setZoomFactor(_aZoom, *m_aSplitter.get());
    setZoomFactor(_aZoom, *m_aEndMarker.get());
    Invalidate();
}

IMPL_LINK_NOARG( OSectionWindow, StartSplitHdl, Splitter*, void)
{
    const OUString sUndoAction( RptResId( RID_STR_UNDO_CHANGE_SIZE ) );
    getViewsWindow()->getView()->getReportView()->getController().getUndoManager().EnterListAction( sUndoAction, OUString(), 0, ViewShellId(-1) );
}

IMPL_LINK_NOARG( OSectionWindow, EndSplitHdl, Splitter*, void )
{
    getViewsWindow()->getView()->getReportView()->getController().getUndoManager().LeaveListAction();
}

IMPL_LINK( OSectionWindow, SplitHdl, Splitter*, _pSplitter, void )
{
    if ( !getViewsWindow()->getView()->getReportView()->getController().isEditable() )
    {
        return;
    }

    sal_Int32 nSplitPos = _pSplitter->GetSplitPosPixel();

    const uno::Reference< report::XSection> xSection = m_aReportSection->getSection();
    nSplitPos = m_aSplitter->PixelToLogic(Size(0,nSplitPos)).Height();

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
    m_aSplitter->SetSplitPosPixel(m_aSplitter->LogicToPixel(Size(0,nSplitPos)).Height());
}

void lcl_scroll(vcl::Window& _rWindow,const Point& _aDelta)
{
    _rWindow.Scroll(-_aDelta.X(),-_aDelta.Y());
    _rWindow.Invalidate(InvalidateFlags::Transparent);
}

void lcl_setOrigin(vcl::Window& _rWindow,long _nX, long _nY)
{
    MapMode aMap = _rWindow.GetMapMode();
    aMap.SetOrigin( Point(- _nX, - _nY));
    _rWindow.SetMapMode( aMap );
}

void OSectionWindow::scrollChildren(long _nX)
{
    const Point aDelta( _nX,0 );

    MapMode aMapMode( m_aReportSection->GetMapMode() );
    const Point aOld = aMapMode.GetOrigin();
    lcl_setOrigin(*m_aReportSection.get(), aDelta.X(), 0);

    aMapMode = m_aReportSection->GetMapMode();
    const Point aNew = aMapMode.GetOrigin();
    const Point aDiff = aOld - aNew;
    {
        lcl_scroll(*m_aReportSection.get(), aDiff);
    }

    lcl_scroll(*m_aEndMarker.get(), m_aEndMarker->PixelToLogic(Point(_nX,0)));

    lcl_setOrigin(*m_aSplitter.get(),_nX, 0);
    lcl_scroll(*m_aSplitter.get(),aDiff);
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
