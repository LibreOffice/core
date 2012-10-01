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
#include "ReportWindow.hxx"
#include "ReportSection.hxx"
#include "SectionView.hxx"
#include "ViewsWindow.hxx"
#include "ReportRuler.hxx"
#include "DesignView.hxx"
#include "UITools.hxx"

#include <tools/debug.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/itempool.hxx>
#include <unotools/syslocale.hxx>

#include "RptDef.hxx"
#include "dlgedfunc.hxx"
#include "RptModel.hxx"
#include "uistrings.hrc"
#include "RptPage.hxx"
#include "ReportController.hxx"
#include "EndMarker.hxx"
#include "ColorChanger.hxx"
#include "ScrollHelper.hxx"

#include "helpids.hrc"
#include "dlgedfac.hxx"
#include <boost/shared_ptr.hpp>
#include <toolkit/helper/vclunohelper.hxx>


#define SECTION_OFFSET  3
namespace rptui
{
using namespace ::com::sun::star;
using namespace ::comphelper;
//==================================================================
// class OReportWindow
//==================================================================
DBG_NAME( rpt_OReportWindow )
//------------------------------------------------------------------------------
OReportWindow::OReportWindow(OScrollWindowHelper* _pParent,ODesignView* _pView)
: Window(_pParent,WB_DIALOGCONTROL)
, ::comphelper::OPropertyChangeListener(m_aMutex)
,m_aHRuler(this)
,m_pView(_pView)
,m_pParent(_pParent)
,m_aViewsWindow(this)
,m_pObjFac( new DlgEdFactory() )
{
    DBG_CTOR( rpt_OReportWindow,NULL);
    SetHelpId(UID_RPT_REPORTWINDOW);
    SetMapMode( MapMode( MAP_100TH_MM ) );

    m_aViewsWindow.Show();

    m_aHRuler.Show();
    m_aHRuler.Activate();
    m_aHRuler.SetPagePos(0);
    m_aHRuler.SetBorders();
    m_aHRuler.SetIndents();
    m_aHRuler.SetMargin1();
    m_aHRuler.SetMargin2();
    const MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    m_aHRuler.SetUnit(MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH);

    ImplInitSettings();
    m_pReportListener = addStyleListener(_pView->getController().getReportDefinition(),this);
}
//------------------------------------------------------------------------------
OReportWindow::~OReportWindow()
{
    DBG_DTOR( rpt_OReportWindow,NULL);
    if ( m_pReportListener.is() )
        m_pReportListener->dispose();
}
// -----------------------------------------------------------------------------
void OReportWindow::initialize()
{
    m_aViewsWindow.initialize();
}
//----------------------------------------------------------------------------
void OReportWindow::SetInsertObj( sal_uInt16 eObj,const ::rtl::OUString& _sShapeType )
{
    m_aViewsWindow.SetInsertObj( eObj,_sShapeType);
}

//----------------------------------------------------------------------------
rtl::OUString OReportWindow::GetInsertObjString() const
{
    return m_aViewsWindow.GetInsertObjString();
}
//------------------------------------------------------------------------------
void OReportWindow::SetMode( DlgEdMode eNewMode )
{
    m_aViewsWindow.SetMode(eNewMode);
}
//----------------------------------------------------------------------------
void OReportWindow::removeSection(sal_uInt16 _nPosition)
{
    m_aViewsWindow.removeSection(_nPosition);
    m_pParent->setTotalSize(GetTotalWidth(),GetTotalHeight());
    m_aViewsWindow.Invalidate(INVALIDATE_TRANSPARENT);
}
//----------------------------------------------------------------------------
void OReportWindow::addSection(const uno::Reference< report::XSection >& _xSection,const ::rtl::OUString& _sColorEntry,sal_uInt16 _nPosition)
{
    if ( !_xSection.is() )
        return;

    m_aViewsWindow.addSection(_xSection,_sColorEntry,_nPosition);

    m_pParent->setTotalSize(GetTotalWidth(),GetTotalHeight());
}
//------------------------------------------------------------------------------
void OReportWindow::toggleGrid(sal_Bool _bVisible)
{
    m_aViewsWindow.toggleGrid(_bVisible);
}
//----------------------------------------------------------------------------
void OReportWindow::showRuler(sal_Bool _bShow)
{
    m_aHRuler.Show(_bShow);

    m_aViewsWindow.showRuler(_bShow);
}
//------------------------------------------------------------------------------
sal_Int32 OReportWindow::getMaxMarkerWidth(sal_Bool _bWithEnd) const
{
    Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH));
    aStartWidth *= m_aViewsWindow.GetMapMode().GetScaleX();
    if ( _bWithEnd )
        aStartWidth += Fraction(long(REPORT_ENDMARKER_WIDTH));
    return sal_Int32((long)aStartWidth);
}
//------------------------------------------------------------------------------
sal_Int32 OReportWindow::GetTotalWidth() const
{
    sal_Int32 nWidth = 0;
    if ( !m_aViewsWindow.empty() )
    {
        Fraction aStartWidth(long(REPORT_ENDMARKER_WIDTH + REPORT_STARTMARKER_WIDTH ));
        const Fraction aZoom(m_pView->getController().getZoomValue(),100);
        aStartWidth *= aZoom;
        const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(m_pView->getController().getReportDefinition(),PROPERTY_PAPERSIZE).Width;
        Fraction aPaperWidth(nPaperWidth,1);
        aPaperWidth *= aZoom;
        const Size aPageSize = LogicToPixel(Size(aPaperWidth,0));
        nWidth = aPageSize.Width() + long(aStartWidth);
    }
    return nWidth;
}
//------------------------------------------------------------------------------
void OReportWindow::Resize()
{
    Window::Resize();
    if ( !m_aViewsWindow.empty() )
    {
        const Size aTotalOutputSize = GetOutputSizePixel();
        Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH)*m_pView->getController().getZoomValue(),100);

        const Point aOffset = LogicToPixel( Point( SECTION_OFFSET, 0 ), MAP_APPFONT );
        Point aStartPoint((long)aStartWidth + aOffset.X(),0);
        uno::Reference<report::XReportDefinition> xReportDefinition = getReportView()->getController().getReportDefinition();
        const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width;
        sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
        sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
        Size aPageSize  = m_aViewsWindow.LogicToPixel(Size(nPaperWidth ,0));
        nLeftMargin     = m_aViewsWindow.LogicToPixel(Size(nLeftMargin,0)).Width();
        nRightMargin    = m_aViewsWindow.LogicToPixel(Size(nRightMargin,0)).Width();

        aPageSize.Height() = m_aHRuler.GetSizePixel().Height();

        const long nTermp(m_aViewsWindow.getTotalHeight() + aPageSize.Height());
        long nSectionsHeight = ::std::max<long>(nTermp,aTotalOutputSize.Height());

        m_aHRuler.SetPosSizePixel(aStartPoint,aPageSize);
        m_aHRuler.SetNullOffset(nLeftMargin);
        m_aHRuler.SetMargin1(0);
        m_aHRuler.SetMargin2(aPageSize.Width() - nLeftMargin - nRightMargin);

        aStartPoint.Y() += aPageSize.Height();
        nSectionsHeight -= aStartPoint.Y();

        aStartPoint.X() = aOffset.X();

        m_aViewsWindow.SetPosSizePixel(aStartPoint,Size(aTotalOutputSize.Width(),nSectionsHeight));
    }
}
//------------------------------------------------------------------------------
Point OReportWindow::getThumbPos() const
{
    return m_pParent->getThumbPos();
}
//------------------------------------------------------------------------------
void OReportWindow::ImplInitSettings()
{
    SetBackground( );
}
//-----------------------------------------------------------------------------
void OReportWindow::DataChanged( const DataChangedEvent& rDCEvt )
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
sal_Int32 OReportWindow::GetTotalHeight() const
{
    return m_aViewsWindow.getTotalHeight();
}
//------------------------------------------------------------------------------
void OReportWindow::ScrollChildren(const Point& _aThumbPos)
{
    MapMode aMap = m_aHRuler.GetMapMode();
    Point aOrg( aMap.GetOrigin() );
    if ( aOrg.X() != (-_aThumbPos.X()) )
    {
        aMap.SetOrigin( Point(- _aThumbPos.X(), aOrg.Y()));
        m_aHRuler.SetMapMode( aMap );
        m_aHRuler.Scroll(-(aOrg.X() + _aThumbPos.X()),0);
    }

    m_aViewsWindow.scrollChildren(_aThumbPos);
}
//----------------------------------------------------------------------------
sal_uInt16 OReportWindow::getSectionCount() const
{
    return m_aViewsWindow.getSectionCount();
}
//----------------------------------------------------------------------------
void OReportWindow::notifySizeChanged()
{
    m_pParent->setTotalSize(GetTotalWidth(),GetTotalHeight());
}
//----------------------------------------------------------------------------
sal_Bool OReportWindow::HasSelection() const
{
    return m_aViewsWindow.HasSelection();
}
//----------------------------------------------------------------------------
void OReportWindow::Delete()
{

    m_aViewsWindow.Delete();
}
//----------------------------------------------------------------------------
void OReportWindow::Copy()
{

    m_aViewsWindow.Copy();
}
//----------------------------------------------------------------------------
void OReportWindow::Paste()
{

    m_aViewsWindow.Paste();
}
//----------------------------------------------------------------------------
sal_Bool OReportWindow::IsPasteAllowed() const
{
    return m_aViewsWindow.IsPasteAllowed();
}
//-----------------------------------------------------------------------------
void OReportWindow::SelectAll(const sal_uInt16 _nObjectType)
{

    m_aViewsWindow.SelectAll(_nObjectType);
}
//-----------------------------------------------------------------------------
void OReportWindow::unmarkAllObjects(OSectionView* _pSectionView)
{

    m_aViewsWindow.unmarkAllObjects(_pSectionView);
}
//-----------------------------------------------------------------------------
void OReportWindow::showProperties(const uno::Reference< report::XSection>& _xReportComponent)
{
    ::boost::shared_ptr<OSectionWindow> pSectionWindow = m_aViewsWindow.getSectionWindow( _xReportComponent );
    m_pView->UpdatePropertyBrowserDelayed(pSectionWindow->getReportSection().getSectionView());
}
//------------------------------------------------------------------------------
sal_Bool OReportWindow::handleKeyEvent(const KeyEvent& _rEvent)
{
    return m_aViewsWindow.handleKeyEvent(_rEvent);
}
//------------------------------------------------------------------------
void OReportWindow::setMarked(OSectionView* _pSectionView,sal_Bool _bMark)
{
    if ( _pSectionView )
        m_aViewsWindow.setMarked(_pSectionView,_bMark);
}
//------------------------------------------------------------------------
void OReportWindow::setMarked(const uno::Reference< report::XSection>& _xSection,sal_Bool _bMark)
{

    m_aViewsWindow.setMarked(_xSection,_bMark);
}
//------------------------------------------------------------------------
void OReportWindow::setMarked(const uno::Sequence< uno::Reference< report::XReportComponent> >& _xShape,sal_Bool _bMark)
{

    m_aViewsWindow.setMarked(_xShape,_bMark);
}
//-----------------------------------------------------------------------------
::boost::shared_ptr<OSectionWindow> OReportWindow::getMarkedSection(NearSectionAccess nsa) const
{
    return  m_aViewsWindow.getMarkedSection(nsa);
}
//-----------------------------------------------------------------------------
::boost::shared_ptr<OSectionWindow> OReportWindow::getSectionWindow(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) const
{
    return  m_aViewsWindow.getSectionWindow(_xSection);
}
// -------------------------------------------------------------------------
void OReportWindow::markSection(const sal_uInt16 _nPos)
{

    m_aViewsWindow.markSection(_nPos);
}
// -----------------------------------------------------------------------------
void OReportWindow::fillCollapsedSections(::std::vector<sal_uInt16>& _rCollapsedPositions) const
{

    m_aViewsWindow.fillCollapsedSections(_rCollapsedPositions);
}
// -----------------------------------------------------------------------------
void OReportWindow::collapseSections(const uno::Sequence< ::com::sun::star::beans::PropertyValue>& _aCollpasedSections)
{

    m_aViewsWindow.collapseSections(_aCollpasedSections);
}
// -----------------------------------------------------------------------------
void OReportWindow::alignMarkedObjects(sal_Int32 _nControlModification,bool _bAlignAtSection, bool bBoundRects)
{

    m_aViewsWindow.alignMarkedObjects(_nControlModification, _bAlignAtSection, bBoundRects);
}
// -----------------------------------------------------------------------------
void OReportWindow::setGridSnap(sal_Bool bOn)
{

    m_aViewsWindow.setGridSnap(bOn);
}
// -----------------------------------------------------------------------------
void OReportWindow::setDragStripes(sal_Bool bOn)
{
    m_aViewsWindow.setDragStripes(bOn);
}
// -----------------------------------------------------------------------------
sal_uInt32 OReportWindow::getMarkedObjectCount() const
{
    return m_aViewsWindow.getMarkedObjectCount();
}
// -----------------------------------------------------------------------------
void OReportWindow::zoom(const Fraction& _aZoom)
{
    m_aHRuler.SetZoom(_aZoom);
    m_aHRuler.Invalidate();

    m_aViewsWindow.zoom(_aZoom);

    notifySizeChanged();
    const Point aNewThumbPos( m_pParent->getThumbPos() );

    ScrollChildren( aNewThumbPos );
    Resize();

    Invalidate(INVALIDATE_NOERASE | INVALIDATE_NOCHILDREN | INVALIDATE_TRANSPARENT);
}
// -----------------------------------------------------------------------------
void OReportWindow::fillControlModelSelection(::std::vector< uno::Reference< uno::XInterface > >& _rSelection) const
{
    m_aViewsWindow.fillControlModelSelection(_rSelection);
}
// -----------------------------------------------------------------------------
sal_Int32 OReportWindow::impl_getRealPixelWidth() const
{
    const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(m_pView->getController().getReportDefinition(),PROPERTY_PAPERSIZE).Width;
    MapMode aMap( MAP_100TH_MM );
    const Size aPageSize = LogicToPixel(Size(nPaperWidth,0),aMap);
    return aPageSize.Width() + REPORT_ENDMARKER_WIDTH + REPORT_STARTMARKER_WIDTH + SECTION_OFFSET;
}
// -----------------------------------------------------------------------------
sal_uInt16 OReportWindow::getZoomFactor(SvxZoomType _eType) const
{
    sal_uInt16 nZoom(100);
    const Size aSize( GetSizePixel() );
    switch( _eType)
    {
        case SVX_ZOOM_PERCENT:
            nZoom = m_pView->getController().getZoomValue();
            break;
        case SVX_ZOOM_OPTIMAL:
            break;
        case SVX_ZOOM_WHOLEPAGE:
            {
                nZoom = (sal_uInt16)(long)Fraction(aSize.Width()*100,impl_getRealPixelWidth());
                MapMode aMap( MAP_100TH_MM );
                const Size aHeight = m_aViewsWindow.LogicToPixel(m_aViewsWindow.PixelToLogic(Size(0,GetTotalHeight() + m_aHRuler.GetSizePixel().Height())),aMap);
                nZoom = ::std::min(nZoom,(sal_uInt16)(long)Fraction(aSize.Height()*100,aHeight.Height()));
            }
            break;
        case SVX_ZOOM_PAGEWIDTH:
            nZoom = (sal_uInt16)(long)Fraction(aSize.Width()*100,impl_getRealPixelWidth());
            break;
        default:
            break;
    }

    return nZoom;
}
// -----------------------------------------------------------------------------
void OReportWindow::_propertyChanged(const beans::PropertyChangeEvent& _rEvent) throw( uno::RuntimeException)
{
    (void)_rEvent;
    Resize();
    m_aViewsWindow.Resize();
    static sal_Int32 nIn = INVALIDATE_TRANSPARENT;
    Invalidate(nIn);
}
//==================================================================
}   //rptui
//==================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
