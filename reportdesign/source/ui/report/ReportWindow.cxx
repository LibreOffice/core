/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportWindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:51:20 $
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

#ifndef RPTUI_REPORT_WINDOW_HXX
#include "ReportWindow.hxx"
#endif
#ifndef REPORT_REPORTSECTION_HXX
#include "ReportSection.hxx"
#endif
#ifndef _REPORT_SECTIONVIEW_HXX
#include "SectionView.hxx"
#endif
#ifndef RPTUI_SECTIONSWINDOW_HXX
#include "SectionsWindow.hxx"
#endif
#ifndef RPTUI_VIEWSWINDOW_HXX
#include "ViewsWindow.hxx"
#endif
#ifndef RPTUI_RULER_HXX
#include "ReportRuler.hxx"
#endif
#ifndef RPTUI_DESIGNVIEW_HXX
#include "DesignView.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include <RptDef.hxx>
#endif
#ifndef _REPORT_RPTUIFUNC_HXX
#include "dlgedfunc.hxx"
#endif

#ifndef REPORT_RPTMODEL_HXX
#include <RptModel.hxx>
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#ifndef _REPORT_RPTUIPAGE_HXX
#include "RptPage.hxx"
#endif
#ifndef RPTUI_REPORTCONTROLLER_HXX
#include "ReportController.hxx"
#endif
#ifndef RPTUI_ENDMARKER_HXX
#include "EndMarker.hxx"
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef RPTUI_COLORCHANGER_HXX
#include "ColorChanger.hxx"
#endif
#ifndef RPTUI_SCROLLHELPER_HXX
#include "ScrollHelper.hxx"
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
#ifndef _REPORT_RPTUIFAC_HXX
#include "dlgedfac.hxx"
#endif
#include <boost/bind.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/syslocale.hxx>

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
,m_aHRuler(this)
,m_pView(_pView)
,m_pParent(_pParent)
{
    DBG_CTOR( rpt_OReportWindow,NULL);
    SetHelpId(UID_RPT_REPORTWINDOW);
    SetMapMode( MapMode( MAP_100TH_MM ) );

    m_pSections = new OSectionsWindow(this,this);
    m_pSections->Show();
    m_pViews = new OViewsWindow(this,this);
    m_pViews->Show();

    m_pSections->SetZOrder(m_pViews,WINDOW_ZORDER_BEFOR);

    m_aHRuler.Show();
    m_aHRuler.Activate();
    m_aHRuler.SetPagePos(0);
    m_aHRuler.SetBorders();
    m_aHRuler.SetIndents();
    m_aHRuler.SetMargin1();
    m_aHRuler.SetMargin2();
    const MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    m_aHRuler.SetUnit(MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH);

    m_pObjFac = new DlgEdFactory();

    ImplInitSettings();
}
//------------------------------------------------------------------------------
OReportWindow::~OReportWindow()
{
    DBG_DTOR( rpt_OReportWindow,NULL);
    {
        ::std::auto_ptr<Window> aTemp(m_pSections);
        m_pSections = NULL;
    }
    {
        ::std::auto_ptr<Window> aTemp(m_pViews);
        m_pViews = NULL;
    }
    delete m_pObjFac;
}
// -----------------------------------------------------------------------------
void OReportWindow::initialize()
{
    if ( m_pViews )
        m_pViews->initialize();
}
//----------------------------------------------------------------------------
void OReportWindow::SetInsertObj( USHORT eObj,const ::rtl::OUString& _sShapeType )
{
    if ( m_pViews )
        m_pViews->SetInsertObj( eObj,_sShapeType);
}

//----------------------------------------------------------------------------
rtl::OUString OReportWindow::GetInsertObjString() const
{
    if ( m_pViews )
        return m_pViews->GetInsertObjString();
    return rtl::OUString();
}
//------------------------------------------------------------------------------
void OReportWindow::SetMode( DlgEdMode eNewMode )
{
    if ( m_pViews )
        m_pViews->SetMode(eNewMode);
}
//----------------------------------------------------------------------------
void OReportWindow::removeSection(USHORT _nPosition)
{
    m_pViews->removeSection(_nPosition);
    m_pSections->removeSection(_nPosition);
    m_pParent->setTotalSize(GetTotalWidth(),GetTotalHeight());
    m_pSections->Invalidate(INVALIDATE_NOCHILDREN|INVALIDATE_TRANSPARENT);
    m_pViews->Invalidate(INVALIDATE_NOCHILDREN);
    //Resize();
}
//----------------------------------------------------------------------------
void OReportWindow::addSection(const uno::Reference< report::XSection >& _xSection,const ::rtl::OUString& _sColorEntry,USHORT _nPosition)
{
    if ( !_xSection.is() )
        return;

    m_pSections->addSection(_xSection,_sColorEntry,_nPosition);
    m_pViews->addSection(_xSection,_sColorEntry,_nPosition);

    Resize();
    m_pParent->setTotalSize(GetTotalWidth(),GetTotalHeight());
}
//------------------------------------------------------------------------------
IMPL_LINK( OReportWindow, Collapsed, OStartMarker *, _pMarker )
{
    if ( _pMarker && m_pSections && m_pViews )
    {
        m_pViews->showView(m_pSections->find(_pMarker),!_pMarker->isCollapsed());

        m_pViews->Resize();
        m_pViews->Invalidate(INVALIDATE_NOCHILDREN);
        m_pSections->Resize();
        _pMarker->Invalidate();
        m_pSections->Invalidate(INVALIDATE_NOCHILDREN|INVALIDATE_TRANSPARENT);
    }
    return 0L;
}
//------------------------------------------------------------------------------
void OReportWindow::toggleGrid(sal_Bool _bVisible)
{
    if ( m_pViews )
        m_pViews->toggleGrid(_bVisible);
}
//----------------------------------------------------------------------------
void OReportWindow::showRuler(sal_Bool _bShow)
{
    m_aHRuler.Show(_bShow);
    if ( m_pSections )
        m_pSections->showRuler(_bShow);
}
//------------------------------------------------------------------------------
sal_Int32 OReportWindow::getMaxMarkerWidth(sal_Bool _bWithEnd) const
{
    return REPORT_STARTMARKER_WIDTH + ( _bWithEnd ? REPORT_ENDMARKER_WIDTH : 0 );
}
//------------------------------------------------------------------------------
sal_Int32 OReportWindow::GetTotalWidth() const
{
    sal_Int32 nWidth = 0;
    if ( m_pViews && !m_pViews->empty() )
    {
        const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(m_pView->getController()->getReportDefinition(),PROPERTY_PAPERSIZE).Width;
        const Size aPageSize = LogicToPixel(Size(nPaperWidth,0),MAP_100TH_MM);
        nWidth = aPageSize.Width()
                + REPORT_ENDMARKER_WIDTH + REPORT_STARTMARKER_WIDTH;
    }
    return nWidth;
}
//------------------------------------------------------------------------------
void OReportWindow::Resize()
{
    Window::Resize();
    if ( m_pView->getController() && m_pViews && !m_pViews->empty() && m_pSections )
    {
        const Size aTotalOutputSize = GetOutputSizePixel();

        Point aStartPoint(REPORT_STARTMARKER_WIDTH,0);
        uno::Reference<report::XReportDefinition> xReportDefinition = getReportView()->getController()->getReportDefinition();
        const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width;
        sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
        sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
        Size aPageSize = LogicToPixel(Size(nPaperWidth,0),MAP_100TH_MM);
        nLeftMargin = LogicToPixel(Size(nLeftMargin,0),MAP_100TH_MM).Width();
        nRightMargin = LogicToPixel(Size(nRightMargin,0),MAP_100TH_MM).Width();

        aPageSize.Height() = m_aHRuler.GetSizePixel().Height();

        const long nTermp(m_pViews->getTotalHeight() + aPageSize.Height());
        long nSectionsHeight = ::std::max<long>(nTermp,aTotalOutputSize.getHeight());
        m_pSections->SetPosSizePixel(Point(0,aPageSize.Height()),Size(REPORT_STARTMARKER_WIDTH,nSectionsHeight));

        m_aHRuler.SetPosSizePixel(aStartPoint,aPageSize);
        m_aHRuler.SetNullOffset(nLeftMargin);
        m_aHRuler.SetMargin1(0);
        m_aHRuler.SetMargin2(aPageSize.Width() - nLeftMargin - nRightMargin);

        aStartPoint.Y() += aPageSize.Height();
        nSectionsHeight -= aStartPoint.Y();

        //m_pViews->SetPosSizePixel(aStartPoint,Size(GetTotalWidth() - aStartPoint.X(),nSectionsHeight));
        m_pViews->SetPosSizePixel(aStartPoint,Size(GetTotalWidth(),nSectionsHeight));
    }
}
//------------------------------------------------------------------------------
Point OReportWindow::getScrollOffset() const
{
    return m_pParent->getScrollOffset();
}
//------------------------------------------------------------------------------
void OReportWindow::ImplInitSettings()
{
//#if OSL_DEBUG_LEVEL > 0
//    SetBackground( Wallpaper( COL_YELLOW ));
//#else
    SetBackground( );
//#endif
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
sal_Int32 OReportWindow::getSplitterHeight() const
{
    sal_Int32 nRet = 0;
    if ( m_pViews )
        nRet = m_pViews->getSplitterHeight();
    return nRet;
}
//------------------------------------------------------------------------------
void OReportWindow::setTotalSize()
{
}
//------------------------------------------------------------------------------
sal_Int32 OReportWindow::GetTotalHeight() const
{
    sal_Int32 nHeight = m_aHRuler.GetSizePixel().Height();
    if ( m_pViews )
        nHeight += m_pViews->getTotalHeight();
    return nHeight;
}
//------------------------------------------------------------------------------
namespace
{
    class OWindowPositionCorrector
    {
        ::std::vector< ::std::pair<Window*,Point> > m_aChildren;
        long m_nDeltaX;
        long m_nDeltaY;
    public:
        OWindowPositionCorrector(Window* _pWindow,long _nDeltaX, long _nDeltaY) :m_nDeltaX(_nDeltaX), m_nDeltaY(_nDeltaY)
        {
            USHORT nCount = _pWindow->GetChildCount();
            m_aChildren.reserve(nCount);
            while( nCount )
            {
                Window* pChild = _pWindow->GetChild(--nCount);
                m_aChildren.push_back(::std::pair<Window*,Point>(pChild,pChild->GetPosPixel()));
            }
        }
        ~OWindowPositionCorrector()
        {
            ::std::vector< ::std::pair<Window*,Point> >::iterator aIter = m_aChildren.begin();
            ::std::vector< ::std::pair<Window*,Point> >::iterator aEnd = m_aChildren.end();
            for (; aIter != aEnd; ++aIter)
            {
                const Point aPos = aIter->first->GetPosPixel();
                if ( aPos == aIter->second )
                    aIter->first->SetPosPixel(Point(m_nDeltaX,m_nDeltaY) + aPos);
            }
        }
    };
}
//----------------------------------------------------------------------------
void OReportWindow::ScrollChildren(long nDeltaX, long nDeltaY)
{
    MapMode aMap = m_aHRuler.GetMapMode();
    Point aOrg( aMap.GetOrigin() );
    aMap.SetOrigin( Point(aOrg.X() - nDeltaX, aOrg.Y()));
    m_aHRuler.SetMapMode( aMap );
    m_aHRuler.Scroll(-nDeltaX,0);

    if ( m_pViews )
    {
        aMap = m_pViews->GetMapMode();
        aOrg = aMap.GetOrigin();
        aMap.SetOrigin( Point(aOrg.X() - nDeltaX, aOrg.Y() - nDeltaY));
        m_pViews->SetMapMode( aMap );
        const Size aDelta( PixelToLogic(Size(nDeltaX,nDeltaY)) );
        OWindowPositionCorrector aCorrector(m_pViews,-nDeltaX,-nDeltaY);
        m_pViews->Scroll(-aDelta.Width(),-aDelta.Height(),SCROLL_CHILDREN);
    } // if ( m_pViews )
    if ( m_pSections && nDeltaY )
    {
        aMap = m_pSections->GetMapMode();
        aOrg  = aMap.GetOrigin();
        aMap.SetOrigin( Point(aOrg.X(), aOrg.Y() - nDeltaY));
        m_pSections->SetMapMode( aMap );
        const Size aDelta( PixelToLogic(Size(0,nDeltaY)) );
        OWindowPositionCorrector aCorrector(m_pSections,0,-nDeltaY);
        m_pSections->Scroll(0,-aDelta.Height(),SCROLL_CHILDREN);
        m_pSections->Invalidate(INVALIDATE_NOCHILDREN|INVALIDATE_TRANSPARENT);
    } // if ( m_pSections )
}
//----------------------------------------------------------------------------
USHORT OReportWindow::getSectionCount() const
{
    USHORT nRet = 0;
    if ( m_pViews )
        nRet = m_pViews->getSectionCount();
    return nRet;
}
//----------------------------------------------------------------------------
sal_Int32 OReportWindow::getMinHeight(USHORT _nPos) const
{
    return m_pSections ? m_pSections->getMinHeight(_nPos) : sal_Int32(0);
}
//----------------------------------------------------------------------------
void OReportWindow::notifyHeightChanged()
{
    m_pParent->setTotalSize(GetTotalWidth(),GetTotalHeight());
    Resize();
    Invalidate(INVALIDATE_TRANSPARENT);
}
//----------------------------------------------------------------------------
BOOL OReportWindow::HasSelection()
{
    return m_pViews && m_pViews->HasSelection();
}
//----------------------------------------------------------------------------
void OReportWindow::Delete()
{
    if ( m_pViews )
        m_pViews->Delete();
}
//----------------------------------------------------------------------------
void OReportWindow::Copy()
{
    if ( m_pViews )
        m_pViews->Copy();
}
//----------------------------------------------------------------------------
void OReportWindow::Paste()
{
    if ( m_pViews )
        m_pViews->Paste();
}
//----------------------------------------------------------------------------
BOOL OReportWindow::IsPasteAllowed()
{
    return m_pViews && m_pViews->IsPasteAllowed();
}
//-----------------------------------------------------------------------------
void OReportWindow::SelectAll()
{
    if ( m_pViews )
        m_pViews->SelectAll();
}
//-----------------------------------------------------------------------------
void OReportWindow::unmarkAllObjects(OSectionView* _pSectionView)
{
    if ( m_pViews )
        m_pViews->unmarkAllObjects(_pSectionView);
}
//-----------------------------------------------------------------------------
void OReportWindow::showProperties(const uno::Reference< report::XSection>& _xReportComponent)
{
    ::boost::shared_ptr<OReportSection> pReportSection = m_pViews->getReportSection( _xReportComponent );
    m_pView->UpdatePropertyBrowserDelayed(pReportSection->getView());
}
//------------------------------------------------------------------------------
sal_Bool OReportWindow::handleKeyEvent(const KeyEvent& _rEvent)
{
    return m_pViews ? m_pViews->handleKeyEvent(_rEvent) : sal_False;
}
//------------------------------------------------------------------------
void OReportWindow::setMarked(OSectionView* _pSectionView,sal_Bool _bMark)
{
    if ( m_pViews && _pSectionView )
        m_pViews->setMarked(_pSectionView,_bMark);
    if ( m_pSections )
    {
        m_pSections->setMarked(_pSectionView->getSectionWindow()->getSection(),_bMark);
    }
}
//------------------------------------------------------------------------
void OReportWindow::setMarked(const uno::Reference< report::XSection>& _xSection,sal_Bool _bMark)
{
    if ( m_pViews )
        m_pViews->setMarked(_xSection,_bMark);
    if ( m_pSections )
        m_pSections->setMarked(_xSection,_bMark);
}
//------------------------------------------------------------------------
void OReportWindow::setMarked(const uno::Sequence< uno::Reference< report::XReportComponent> >& _xShape,sal_Bool _bMark)
{
    if ( m_pViews )
        m_pViews->setMarked(_xShape,_bMark);
}
//-----------------------------------------------------------------------------
::boost::shared_ptr<OReportSection> OReportWindow::getMarkedSection(NearSectionAccess nsa) const
{
    return  m_pViews ? m_pViews->getMarkedSection(nsa) : ::boost::shared_ptr<OReportSection>();
}
// -------------------------------------------------------------------------
void OReportWindow::markSection(const sal_uInt16 _nPos)
{
    if ( m_pViews )
        m_pViews->markSection(_nPos);
}
// -----------------------------------------------------------------------------
void OReportWindow::fillCollapsedSections(::std::vector<sal_uInt16>& _rCollapsedPositions) const
{
    if ( m_pSections )
        m_pSections->fillCollapsedSections(_rCollapsedPositions);
}
// -----------------------------------------------------------------------------
void OReportWindow::collapseSections(const uno::Sequence< ::com::sun::star::beans::PropertyValue>& _aCollpasedSections)
{
    if ( m_pSections )
        m_pSections->collapseSections(_aCollpasedSections);
}
// -----------------------------------------------------------------------------
void OReportWindow::alignMarkedObjects(sal_Int32 _nControlModification,bool _bAlignAtSection, bool bBoundRects)
{
    if ( m_pViews )
        m_pViews->alignMarkedObjects(_nControlModification, _bAlignAtSection, bBoundRects);
}
// -----------------------------------------------------------------------------
void OReportWindow::setGridSnap(BOOL bOn)
{
    if ( m_pViews )
        m_pViews->setGridSnap(bOn);
}
// -----------------------------------------------------------------------------
void OReportWindow::setDragStripes(BOOL bOn)
{
    if ( m_pViews )
        m_pViews->setDragStripes(bOn);
}
// -----------------------------------------------------------------------------
BOOL OReportWindow::isDragStripes() const
{
    return m_pViews ? m_pViews->isDragStripes() : FALSE;
}
// -----------------------------------------------------------------------------
sal_uInt32 OReportWindow::getMarkedObjectCount() const
{
    return m_pViews ? m_pViews->getMarkedObjectCount() : 0;
}

//==================================================================
}   //rptui
//==================================================================

