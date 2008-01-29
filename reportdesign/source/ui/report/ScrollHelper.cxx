/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScrollHelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:51:31 $
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

#ifndef RPTUI_SCROLLHELPER_HXX
#include "ScrollHelper.hxx"
#endif
#ifndef RPTUI_DESIGNVIEW_HXX
#include "DesignView.hxx"
#endif
#ifndef RPTUI_REPORTCONTROLLER_HXX
#include "ReportController.hxx"
#endif
#ifndef RPTUI_REPORT_WINDOW_HXX
#include "ReportWindow.hxx"
#endif
#include "UITools.hxx"
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

namespace rptui
{
#define LINE_SIZE           50
#define SECTION_OFFSET      3
#define SCR_LINE_SIZE       10
using namespace ::com::sun::star;

// -----------------------------------------------------------------------------
void lcl_setScrollBar(sal_Int32 _nNewValue,sal_Int32 _nOffSet,const Point& _aPos,const Size& _aSize,ScrollBar& _rScrollBar)
{
    _rScrollBar.SetPosSizePixel(_aPos,_aSize);
    _rScrollBar.SetPageSize( _nNewValue );
    _rScrollBar.SetVisibleSize( _nNewValue );
    (void)_nOffSet;
    //_rScrollBar.SetThumbPos( -_nOffSet );
}

// -----------------------------------------------------------------------------
DBG_NAME( rpt_OScrollWindowHelper );
OScrollWindowHelper::OScrollWindowHelper( ODesignView* _pDesignView)
    : OScrollWindowHelper_BASE( _pDesignView,WB_DIALOGCONTROL)
    ,OPropertyChangeListener(m_aMutex)
    ,m_aHScroll( this, WB_HSCROLL|WB_REPEAT|WB_DRAG )
    ,m_aVScroll( this, WB_VSCROLL|WB_REPEAT|WB_DRAG )
    ,m_aCornerWin( this )
    ,m_pParent(_pDesignView)
    ,m_pChild(NULL)
    ,m_pReportDefintionMultiPlexer(NULL)
{
    DBG_CTOR( rpt_OScrollWindowHelper,NULL);
    SetMapMode( MapMode( MAP_100TH_MM ) );

    impl_initScrollBar( m_aHScroll );
    impl_initScrollBar( m_aVScroll );

    m_pChild = new OReportWindow(this,m_pParent);
    m_pChild->SetMapMode( MapMode( MAP_100TH_MM ) );
    m_pChild->Show();

    // normally we should be SCROLL_PANE
    SetAccessibleRole(accessibility::AccessibleRole::SCROLL_PANE);
    ImplInitSettings();
}

// -----------------------------------------------------------------------------
OScrollWindowHelper::~OScrollWindowHelper()
{
    DBG_DTOR( rpt_OScrollWindowHelper,NULL);
    if ( m_pReportDefintionMultiPlexer.is() )
        m_pReportDefintionMultiPlexer->dispose();
    {
        ::std::auto_ptr<OReportWindow> aTemp2(m_pChild);
        m_pChild = NULL;
    }
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
    uno::Reference<report::XReportDefinition> xReportDefinition = m_pParent->getController()->getReportDefinition();
    m_pReportDefintionMultiPlexer = addStyleListener(xReportDefinition,this);
    if ( m_pChild )
        m_pChild->initialize();
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::setTotalSize(sal_Int32 _nWidth ,sal_Int32 _nHeight)
{
    m_aTotalPixelSize.Width() = _nWidth;
    m_aTotalPixelSize.Height() = _nHeight;
    m_aHScroll.SetRangeMax( m_aTotalPixelSize.Width() );
    m_aVScroll.SetRangeMax( m_aTotalPixelSize.Height() );
    /*Point aNewPixOffset(-m_aHScroll.GetThumbPos(),-m_aVScroll.GetThumbPos());
    if ( m_aPixOffset != aNewPixOffset )
    {
        const long nX = aNewPixOffset.X() - m_aPixOffset.X();
        const long nY = aNewPixOffset.Y() - m_aPixOffset.Y();
        impl_scrollContent( -nX, -nY );
        m_aPixOffset = aNewPixOffset;
    }*/

    Resize();
}
//------------------------------------------------------------------------------
Size OScrollWindowHelper::ResizeScrollBars()
{
    // get the new output-size in pixel
    Size aOutPixSz = GetOutputSizePixel();
    if ( aOutPixSz.Width() == 0 || aOutPixSz.Height() == 0 )
        return aOutPixSz;

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

    const Point aOldPixOffset( m_aPixOffset );

    // show or hide scrollbars
    m_aVScroll.Show( bVVisible );
    m_aHScroll.Show( bHVisible );

    // disable painting in the corner between the scrollbars
    if ( bVVisible && bHVisible )
    {
        m_aCornerWin.SetPosSizePixel(*((Point*) &aOutPixSz), Size(nScrSize, nScrSize) );
        m_aCornerWin.Show();
    }
    else
        m_aCornerWin.Hide();

    const Point aOffset = LogicToPixel( Point( SECTION_OFFSET, SECTION_OFFSET ), MAP_APPFONT );
    // resize scrollbars and set their ranges
    {
        const sal_Int32 nOldThumbPos = m_aHScroll.GetThumbPos();
        const sal_Int32 nNewWidth = aOutPixSz.Width() - aOffset.X();
        lcl_setScrollBar(nNewWidth,m_aPixOffset.X(),Point( REPORT_STARTMARKER_WIDTH, aOutPixSz.Height() ),Size( aOutPixSz.Width() - REPORT_STARTMARKER_WIDTH, nScrSize ),m_aHScroll);
        m_aPixOffset.X() += nOldThumbPos - m_aHScroll.GetThumbPos();
    }
    {
        const sal_Int32 nOldThumbPos = m_aVScroll.GetThumbPos();
        const sal_Int32 nNewHeight = aOutPixSz.Height() - m_pChild->getRulerHeight();
        lcl_setScrollBar(nNewHeight,m_aPixOffset.Y(),Point( aOutPixSz.Width(), m_pChild->getRulerHeight() ),Size( nScrSize,nNewHeight),m_aVScroll);
        m_aPixOffset.Y() += nOldThumbPos - m_aVScroll.GetThumbPos();
    }

    // select the shifted map-mode
    if ( m_aPixOffset != aOldPixOffset )
    {
        const long nX = m_aPixOffset.X() - aOldPixOffset.X();
        const long nY = m_aPixOffset.Y() - aOldPixOffset.Y();
        impl_scrollContent( -nX, -nY );
    }
    return aOutPixSz;
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::EndScroll( long nDeltaX, long nDeltaY )
{
    m_aHScroll.SetThumbPos(m_aHScroll.GetThumbPos()+nDeltaX);
    impl_scrollContent( nDeltaX, nDeltaY );
    Resize();

}
//------------------------------------------------------------------------------
void OScrollWindowHelper::impl_scrollContent( long nDeltaX, long nDeltaY )
{
    if ( m_pChild )
        m_pChild->ScrollChildren(nDeltaX,nDeltaY);
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::Resize()
{
    OScrollWindowHelper_BASE::Resize();
 //   Size aSize = GetOutputSizePixel();
 //   const Size aMinSize(getMaxMarkerWidth(sal_False),0);
 //   bool bResize = false;
    //if( aSize.Width() < aMinSize.Width() )
    //{
    //  aSize.setWidth( aMinSize.Width() );
    //  bResize = true;
    //}
    //if( aSize.Height() < aMinSize.Height() )
    //{
    //  aSize.setHeight( aMinSize.Height() );
    //  bResize = true;
    //}
    //if( bResize )
 //   {
 //       //static_cast<SplitWindow*>(GetParent())->SetItemSize(2,aSize.getWidth());
    //  SetOutputSizePixel( aSize );
 //   }
    const Size aTotalOutputSize = ResizeScrollBars();

    OSL_ENSURE(m_pChild,"ReportWindow is NULL");
    if ( m_pChild )
        m_pChild->SetPosSizePixel(Point( 0, 0 ),Size( aTotalOutputSize.Width(), aTotalOutputSize.Height()));
}
//------------------------------------------------------------------------------
IMPL_LINK( OScrollWindowHelper, ScrollHdl, ScrollBar*, /*pScroll*/ )
{
    impl_scrollContent( m_aHScroll.GetDelta(), m_aVScroll.GetDelta() );
    return 0;
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::addSection(const uno::Reference< report::XSection >& _xSection
                                   ,const ::rtl::OUString& _sColorEntry
                                   ,USHORT _nPosition)
{
    m_pChild->addSection(_xSection,_sColorEntry,_nPosition);
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::removeSection(USHORT _nPosition)
{
    m_pChild->removeSection(_nPosition);
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::toggleGrid(sal_Bool _bVisible)
{
    m_pChild->toggleGrid(_bVisible);
}
//------------------------------------------------------------------------------
USHORT OScrollWindowHelper::getSectionCount() const
{
    return m_pChild->getSectionCount();
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::SetInsertObj( USHORT eObj,const ::rtl::OUString& _sShapeType )
{
    m_pChild->SetInsertObj(eObj,_sShapeType);
}
//----------------------------------------------------------------------------
rtl::OUString OScrollWindowHelper::GetInsertObjString() const
{
    return m_pChild->GetInsertObjString();
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::SetMode( DlgEdMode _eNewMode )
{
    m_pChild->SetMode(_eNewMode);
}
//------------------------------------------------------------------------------
BOOL OScrollWindowHelper::HasSelection()
{
    return m_pChild && m_pChild->HasSelection();
}
//----------------------------------------------------------------------------
void OScrollWindowHelper::Delete()
{
    if ( m_pChild )
        m_pChild->Delete();
}
//----------------------------------------------------------------------------
void OScrollWindowHelper::Copy()
{
    if ( m_pChild )
        m_pChild->Copy();
}
//----------------------------------------------------------------------------
void OScrollWindowHelper::Paste()
{
    if ( m_pChild )
        m_pChild->Paste();
}
//----------------------------------------------------------------------------
BOOL OScrollWindowHelper::IsPasteAllowed()
{
    return m_pChild && m_pChild->IsPasteAllowed();
}
//-----------------------------------------------------------------------------
void OScrollWindowHelper::SelectAll()
{
    m_pChild->SelectAll();
}
//----------------------------------------------------------------------------
void OScrollWindowHelper::unmarkAllObjects(OSectionView* _pSectionView)
{
    m_pChild->unmarkAllObjects(_pSectionView);
}
//------------------------------------------------------------------------------
sal_Int32 OScrollWindowHelper::getMaxMarkerWidth(sal_Bool _bWithEnd) const
{
    return m_pChild->getMaxMarkerWidth(_bWithEnd);
}
//----------------------------------------------------------------------------
void OScrollWindowHelper::showRuler(sal_Bool _bShow)
{
    m_pChild->showRuler(_bShow);
}
//------------------------------------------------------------------------------
sal_Bool OScrollWindowHelper::handleKeyEvent(const KeyEvent& _rEvent)
{
    return m_pChild ? m_pChild->handleKeyEvent(_rEvent) : sal_False;
}
//------------------------------------------------------------------------
void OScrollWindowHelper::setMarked(OSectionView* _pSectionView,sal_Bool _bMark)
{
    if ( m_pChild )
        m_pChild->setMarked(_pSectionView,_bMark);
}
//------------------------------------------------------------------------
void OScrollWindowHelper::setMarked(const uno::Reference< report::XSection>& _xSection,sal_Bool _bMark)
{
    if ( m_pChild )
        m_pChild->setMarked(_xSection,_bMark);
}
//------------------------------------------------------------------------
void OScrollWindowHelper::setMarked(const uno::Sequence< uno::Reference< report::XReportComponent> >& _xShape,sal_Bool _bMark)
{
    if ( m_pChild )
        m_pChild->setMarked(_xShape,_bMark);
}
// -------------------------------------------------------------------------
::boost::shared_ptr<OReportSection> OScrollWindowHelper::getMarkedSection(NearSectionAccess nsa) const
{
    return m_pChild ? m_pChild->getMarkedSection(nsa) : ::boost::shared_ptr<OReportSection>();
}
// -------------------------------------------------------------------------
void OScrollWindowHelper::markSection(const sal_uInt16 _nPos)
{
    if ( m_pChild )
        m_pChild->markSection(_nPos);
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::fillCollapsedSections(::std::vector<sal_uInt16>& _rCollapsedPositions) const
{
    if ( m_pChild )
        m_pChild->fillCollapsedSections(_rCollapsedPositions);
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::collapseSections(const uno::Sequence< ::com::sun::star::beans::PropertyValue>& _aCollpasedSections)
{
    if ( m_pChild )
        m_pChild->collapseSections(_aCollpasedSections);
}
//------------------------------------------------------------------------------
long OScrollWindowHelper::Notify( NotifyEvent& rNEvt )
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
            return 1L;
    }
    return OScrollWindowHelper_BASE::Notify(rNEvt);
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::alignMarkedObjects(sal_Int32 _nControlModification,bool _bAlignAtSection, bool bBoundRects)
{
    if ( m_pChild )
        m_pChild->alignMarkedObjects(_nControlModification, _bAlignAtSection, bBoundRects);
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::ImplInitSettings()
{
//#if OSL_DEBUG_LEVEL > 0
//    SetBackground( Wallpaper( COL_BLUE ));
//#else
    //SetBackground( );
    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
//#endif
    SetFillColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    SetTextFillColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
//#endif
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
    if ( m_pChild )
        m_pChild->notifyHeightChanged();
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::setGridSnap(BOOL bOn)
{
    if ( m_pChild )
        m_pChild->setGridSnap(bOn);
}
// -----------------------------------------------------------------------------
void OScrollWindowHelper::setDragStripes(BOOL bOn)
{
    if ( m_pChild )
        m_pChild->setDragStripes(bOn);
}
// -----------------------------------------------------------------------------
BOOL OScrollWindowHelper::isDragStripes() const
{
    return m_pChild ? m_pChild->isDragStripes() : FALSE;
}
// -----------------------------------------------------------------------------
sal_uInt32 OScrollWindowHelper::getMarkedObjectCount() const
{
    return m_pChild ? m_pChild->getMarkedObjectCount() : 0;
}
// -----------------------------------------------------------------------------
//==============================================================================
} // rptui
//==============================================================================
