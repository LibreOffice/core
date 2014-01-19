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
#include "ScrollHelper.hxx"
#include "DesignView.hxx"
#include "ReportController.hxx"
#include "ReportWindow.hxx"
#include "UITools.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>

namespace rptui
{
#define SECTION_OFFSET      3
#define SCR_LINE_SIZE       10
using namespace ::com::sun::star;

// -----------------------------------------------------------------------------
void lcl_setScrollBar(sal_Int32 _nNewValue,const Point& _aPos,const Size& _aSize,ScrollBar& _rScrollBar)
{
    _rScrollBar.SetPosSizePixel(_aPos,_aSize);
    _rScrollBar.SetPageSize( _nNewValue );
    _rScrollBar.SetVisibleSize( _nNewValue );
}

// -----------------------------------------------------------------------------
OScrollWindowHelper::OScrollWindowHelper( ODesignView* _pDesignView)
    : OScrollWindowHelper_BASE( _pDesignView,WB_DIALOGCONTROL)
    ,OPropertyChangeListener(m_aMutex)
    ,m_aHScroll( this, WB_HSCROLL|WB_REPEAT|WB_DRAG )
    ,m_aVScroll( this, WB_VSCROLL|WB_REPEAT|WB_DRAG )
    ,m_aCornerWin( this )
    ,m_pParent(_pDesignView)
    ,m_aReportWindow(this,m_pParent)
    ,m_pReportDefintionMultiPlexer(NULL)
{
    SetMapMode( MapMode( MAP_100TH_MM ) );

    impl_initScrollBar( m_aHScroll );
    impl_initScrollBar( m_aVScroll );

    m_aReportWindow.SetMapMode( MapMode( MAP_100TH_MM ) );
    m_aReportWindow.Show();

    // normally we should be SCROLL_PANE
    SetAccessibleRole(accessibility::AccessibleRole::SCROLL_PANE);
    ImplInitSettings();
}

// -----------------------------------------------------------------------------
OScrollWindowHelper::~OScrollWindowHelper()
{
    if ( m_pReportDefintionMultiPlexer.is() )
        m_pReportDefintionMultiPlexer->dispose();
}

// -----------------------------------------------------------------------------
void OScrollWindowHelper::impl_initScrollBar( ScrollBar& _rScrollBar ) const
{
    AllSettings aSettings( _rScrollBar.GetSettings() );
    StyleSettings aStyle( aSettings.GetStyleSettings() );
    aStyle.SetDragFullOptions( aStyle.GetDragFullOptions() | DRAGFULL_OPTION_SCROLL ); // live scrolling
    aSettings.SetStyleSettings( aStyle );
    _rScrollBar.SetSettings( aSettings );

    _rScrollBar.SetScrollHdl( LINK( this, OScrollWindowHelper, ScrollHdl ) );
    _rScrollBar.SetLineSize( SCR_LINE_SIZE );
}

// -----------------------------------------------------------------------------
void OScrollWindowHelper::initialize()
{
    uno::Reference<report::XReportDefinition> xReportDefinition = m_pParent->getController().getReportDefinition();
    m_pReportDefintionMultiPlexer = addStyleListener(xReportDefinition,this);

    m_aReportWindow.initialize();
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::setTotalSize(sal_Int32 _nWidth ,sal_Int32 _nHeight)
{
    m_aTotalPixelSize.Width() = _nWidth;
    m_aTotalPixelSize.Height() = _nHeight;

    // now set the ranges without start marker
    Fraction aStartWidth(REPORT_STARTMARKER_WIDTH * m_pParent->getController().getZoomValue(),100);
    long nWidth = long(_nWidth - (double)aStartWidth);
    m_aHScroll.SetRangeMax( nWidth );
    m_aVScroll.SetRangeMax( m_aTotalPixelSize.Height() );

    Resize();
}
//------------------------------------------------------------------------------
Size OScrollWindowHelper::ResizeScrollBars()
{
    // get the new output-size in pixel
    Size aOutPixSz = GetOutputSizePixel();
    if ( aOutPixSz.Width() == 0 || aOutPixSz.Height() == 0 )
        return aOutPixSz;

    aOutPixSz.Height() -= m_aReportWindow.getRulerHeight();
    // determine the size of the output-area and if we need scrollbars
    const long nScrSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    bool bVVisible = false; // by default no vertical-ScrollBar
    bool bHVisible = false; // by default no horizontal-ScrollBar
    bool bChanged;          // determines if a visiblility was changed
    do
    {
        bChanged = false;

        // does we need a vertical ScrollBar
        if ( aOutPixSz.Width() < m_aTotalPixelSize.Width() && !bHVisible )
        {
            bHVisible = true;
            aOutPixSz.Height() -= nScrSize;
            bChanged = true;
        }

        // does we need a horizontal ScrollBar
        if ( aOutPixSz.Height() < m_aTotalPixelSize.Height() && !bVVisible )
        {
            bVVisible = true;
            aOutPixSz.Width() -= nScrSize;
            bChanged = true;
        }

    }
    while ( bChanged );   // until no visibility has changed

    aOutPixSz.Height() += m_aReportWindow.getRulerHeight();

    // show or hide scrollbars
    m_aVScroll.Show( bVVisible );
    m_aHScroll.Show( bHVisible );

    // disable painting in the corner between the scrollbars
    if ( bVVisible && bHVisible )
    {
        m_aCornerWin.SetPosSizePixel(Point(aOutPixSz.Width(), aOutPixSz.Height()), Size(nScrSize, nScrSize) );
        m_aCornerWin.Show();
    }
    else
        m_aCornerWin.Hide();

    const Point aOffset = LogicToPixel( Point( SECTION_OFFSET, SECTION_OFFSET ), MAP_APPFONT );
    // resize scrollbars and set their ranges
    {
        Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH*m_pParent->getController().getZoomValue()),100);
        const sal_Int32 nNewWidth = aOutPixSz.Width() - aOffset.X() - (long)aStartWidth;
        lcl_setScrollBar(nNewWidth,Point( (long)aStartWidth + aOffset.X(), aOutPixSz.Height() ),Size( nNewWidth, nScrSize ),m_aHScroll);
    }
    {
        const sal_Int32 nNewHeight = aOutPixSz.Height() - m_aReportWindow.getRulerHeight();
        lcl_setScrollBar(nNewHeight,Point( aOutPixSz.Width(), m_aReportWindow.getRulerHeight() ),Size( nScrSize,nNewHeight),m_aVScroll);
    }

    return aOutPixSz;
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::Resize()
{
    OScrollWindowHelper_BASE::Resize();
    const Size aTotalOutputSize = ResizeScrollBars();

    m_aReportWindow.SetPosSizePixel(Point( 0, 0 ),aTotalOutputSize);
}
//------------------------------------------------------------------------------
IMPL_LINK( OScrollWindowHelper, ScrollHdl, ScrollBar*, /*pScroll*/ )
{
    m_aReportWindow.ScrollChildren( getThumbPos() );
    return 0;
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::addSection(const uno::Reference< report::XSection >& _xSection
                                   ,const OUString& _sColorEntry
                                   ,sal_uInt16 _nPosition)
{
    m_aReportWindow.addSection(_xSection,_sColorEntry,_nPosition);
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::removeSection(sal_uInt16 _nPosition)
{
    m_aReportWindow.removeSection(_nPosition);
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::toggleGrid(sal_Bool _bVisible)
{
    m_aReportWindow.toggleGrid(_bVisible);
}
//------------------------------------------------------------------------------
sal_uInt16 OScrollWindowHelper::getSectionCount() const
{
    return m_aReportWindow.getSectionCount();
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::SetInsertObj( sal_uInt16 eObj,const OUString& _sShapeType )
{
    m_aReportWindow.SetInsertObj(eObj,_sShapeType);
}
//----------------------------------------------------------------------------
OUString OScrollWindowHelper::GetInsertObjString() const
{
    return m_aReportWindow.GetInsertObjString();
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::SetMode( DlgEdMode _eNewMode )
{
    m_aReportWindow.SetMode(_eNewMode);
}
//------------------------------------------------------------------------------
sal_Bool OScrollWindowHelper::HasSelection() const
{
    return m_aReportWindow.HasSelection();
}
//----------------------------------------------------------------------------
void OScrollWindowHelper::Delete()
{
    m_aReportWindow.Delete();
}
//----------------------------------------------------------------------------
void OScrollWindowHelper::Copy()
{
    m_aReportWindow.Copy();
}
//----------------------------------------------------------------------------
void OScrollWindowHelper::Paste()
{
    m_aReportWindow.Paste();
}
//----------------------------------------------------------------------------
sal_Bool OScrollWindowHelper::IsPasteAllowed() const
{
    return m_aReportWindow.IsPasteAllowed();
}
//-----------------------------------------------------------------------------
void OScrollWindowHelper::SelectAll(const sal_uInt16 _nObjectType)
{
    m_aReportWindow.SelectAll(_nObjectType);
}
//----------------------------------------------------------------------------
void OScrollWindowHelper::unmarkAllObjects(OSectionView* _pSectionView)
{
    m_aReportWindow.unmarkAllObjects(_pSectionView);
}
//------------------------------------------------------------------------------
sal_Int32 OScrollWindowHelper::getMaxMarkerWidth(sal_Bool _bWithEnd) const
{
    return m_aReportWindow.getMaxMarkerWidth(_bWithEnd);
}
//----------------------------------------------------------------------------
void OScrollWindowHelper::showRuler(sal_Bool _bShow)
{
    m_aReportWindow.showRuler(_bShow);
}
//------------------------------------------------------------------------------
sal_Bool OScrollWindowHelper::handleKeyEvent(const KeyEvent& _rEvent)
{
    return m_aReportWindow.handleKeyEvent(_rEvent);
}
//------------------------------------------------------------------------
void OScrollWindowHelper::setMarked(OSectionView* _pSectionView,sal_Bool _bMark)
{
    m_aReportWindow.setMarked(_pSectionView,_bMark);
}
//------------------------------------------------------------------------
void OScrollWindowHelper::setMarked(const uno::Reference< report::XSection>& _xSection,sal_Bool _bMark)
{
    m_aReportWindow.setMarked(_xSection,_bMark);
}
//------------------------------------------------------------------------
void OScrollWindowHelper::setMarked(const uno::Sequence< uno::Reference< report::XReportComponent> >& _xShape,sal_Bool _bMark)
{
    m_aReportWindow.setMarked(_xShape,_bMark);
}
// -------------------------------------------------------------------------
::boost::shared_ptr<OSectionWindow> OScrollWindowHelper::getMarkedSection(NearSectionAccess nsa) const
{
    return m_aReportWindow.getMarkedSection(nsa);
}
// -------------------------------------------------------------------------
::boost::shared_ptr<OSectionWindow> OScrollWindowHelper::getSectionWindow(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) const
{
    return  m_aReportWindow.getSectionWindow(_xSection);
}
// -------------------------------------------------------------------------
void OScrollWindowHelper::markSection(const sal_uInt16 _nPos)
{
    m_aReportWindow.markSection(_nPos);
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::fillCollapsedSections(::std::vector<sal_uInt16>& _rCollapsedPositions) const
{
    m_aReportWindow.fillCollapsedSections(_rCollapsedPositions);
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::collapseSections(const uno::Sequence< ::com::sun::star::beans::PropertyValue>& _aCollpasedSections)
{
    m_aReportWindow.collapseSections(_aCollpasedSections);
}
//------------------------------------------------------------------------------
bool OScrollWindowHelper::Notify( NotifyEvent& rNEvt )
{
    const CommandEvent* pCommandEvent = rNEvt.GetCommandEvent();
    if ( pCommandEvent &&
        ( ((pCommandEvent->GetCommand() == COMMAND_WHEEL) ||
         (pCommandEvent->GetCommand() == COMMAND_STARTAUTOSCROLL) ||
         (pCommandEvent->GetCommand() == COMMAND_AUTOSCROLL))) )
    {
        ScrollBar* pHScrBar = NULL;
        ScrollBar* pVScrBar = NULL;
        if ( m_aHScroll.IsVisible() )
            pHScrBar = &m_aHScroll;

        if ( m_aVScroll.IsVisible() )
            pVScrBar = &m_aVScroll;

        if ( HandleScrollCommand( *pCommandEvent, pHScrBar, pVScrBar ) )
            return true;
    }
    return OScrollWindowHelper_BASE::Notify(rNEvt);
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::alignMarkedObjects(sal_Int32 _nControlModification,bool _bAlignAtSection, bool bBoundRects)
{
    m_aReportWindow.alignMarkedObjects(_nControlModification, _bAlignAtSection, bBoundRects);
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::ImplInitSettings()
{
    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
    SetFillColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    SetTextFillColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
}
//-----------------------------------------------------------------------------
void OScrollWindowHelper::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::_propertyChanged(const beans::PropertyChangeEvent& /*_rEvent*/) throw( uno::RuntimeException)
{
    m_aReportWindow.notifySizeChanged();
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::setGridSnap(sal_Bool bOn)
{
    m_aReportWindow.setGridSnap(bOn);
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::setDragStripes(sal_Bool bOn)
{
    m_aReportWindow.setDragStripes(bOn);
}
// -----------------------------------------------------------------------------
sal_uInt32 OScrollWindowHelper::getMarkedObjectCount() const
{
    return m_aReportWindow.getMarkedObjectCount();
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::zoom(const Fraction& _aZoom)
{
    m_aReportWindow.zoom(_aZoom);
    Resize();
    Invalidate(INVALIDATE_NOCHILDREN|INVALIDATE_TRANSPARENT);
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::fillControlModelSelection(::std::vector< uno::Reference< uno::XInterface > >& _rSelection) const
{
    m_aReportWindow.fillControlModelSelection(_rSelection);
}
// -----------------------------------------------------------------------------
sal_uInt16 OScrollWindowHelper::getZoomFactor(SvxZoomType _eType) const
{
    return m_aReportWindow.getZoomFactor(_eType);
}
//==============================================================================
} // rptui
//==============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
