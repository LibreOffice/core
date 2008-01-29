/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewsWindow.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:52:21 $
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

#ifndef RPTUI_VIEWSWINDOW_HXX
#include "ViewsWindow.hxx"
#endif
#include "ScrollHelper.hxx"
#include "UndoActions.hxx"
#ifndef RPTUI_REPORT_WINDOW_HXX
#include "ReportWindow.hxx"
#endif
#ifndef RPTUI_DESIGNVIEW_HXX
#include "DesignView.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef RPTUI_REPORTCONTROLLER_HXX
#include "ReportController.hxx"
#endif
#ifndef RPTUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef RPTUI_ENDMARKER_HXX
#include "EndMarker.hxx"
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef _REPORT_SECTIONVIEW_HXX
#include "SectionView.hxx"
#endif
#ifndef REPORT_REPORTSECTION_HXX
#include "ReportSection.hxx"
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#ifndef _RPTUI_SLOTID_HRC_
#include "rptui_slotid.hrc"
#endif
#ifndef _REPORT_RPTUICLIP_HXX
#include "dlgedclip.hxx"
#endif
#include "RptObject.hxx"
#include "RptObject.hxx"
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
#include <vcl/svapp.hxx>
#include <boost/bind.hpp>

#include "helpids.hrc"
#include <svx/svdundo.hxx>
#include <toolkit/helper/convert.hxx>
#include <algorithm>

namespace rptui
{
#define SECTION_OFFSET  3
#define DEFAUL_MOVE_SIZE    100

using namespace ::com::sun::star;
using namespace ::comphelper;
// -----------------------------------------------------------------------------
bool lcl_getNewRectSize(const Rectangle& _aObjRect,long& _nXMov, long& _nYMov,SdrObject* _pObj,SdrView* _pView,sal_Int32 _nControlModification, bool _bBoundRects)
{
    bool bMoveAllowed = _nXMov != 0 || _nYMov != 0;
    if ( bMoveAllowed )
    {
        Rectangle aNewRect = _aObjRect;
        SdrObject* pOverlappedObj = NULL;
        do
        {
            aNewRect = _aObjRect;
            switch(_nControlModification)
            {
                case ControlModification::HEIGHT_GREATEST:
                case ControlModification::WIDTH_GREATEST:
                    aNewRect.setWidth(_nXMov);
                    aNewRect.setHeight(_nYMov);
                    break;
                default:
                    aNewRect.Move(_nXMov,_nYMov);
                    break;
            }
            if ( dynamic_cast<OUnoObject*>(_pObj) )
            {
                pOverlappedObj = isOver(aNewRect,*_pObj->GetPage(),*_pView,true,_pObj);
                if ( pOverlappedObj && _pObj != pOverlappedObj )
                {
                    Rectangle aOverlappingRect = (_bBoundRects ? pOverlappedObj->GetCurrentBoundRect() : pOverlappedObj->GetSnapRect());
                    sal_Int32 nXTemp = _nXMov;
                    sal_Int32 nYTemp = _nYMov;
                    switch(_nControlModification)
                    {
                        case ControlModification::LEFT:
                            nXTemp += aOverlappingRect.Right() - aNewRect.Left();
                            bMoveAllowed = _nXMov != nXTemp;
                            break;
                        case ControlModification::RIGHT:
                            nXTemp += aOverlappingRect.Left() - aNewRect.Right();
                            bMoveAllowed = _nXMov != nXTemp;
                            break;
                        case ControlModification::TOP:
                            nYTemp += aOverlappingRect.Bottom() - aNewRect.Top();
                            bMoveAllowed = _nYMov != nYTemp;
                            break;
                        case ControlModification::BOTTOM:
                            nYTemp += aOverlappingRect.Top() - aNewRect.Bottom();
                            bMoveAllowed = _nYMov != nYTemp;
                            break;
                        case ControlModification::CENTER_HORIZONTAL:
                            if ( _aObjRect.Left() < aOverlappingRect.Left() )
                                nXTemp += aOverlappingRect.Left() - aNewRect.Left() - aNewRect.getWidth();
                            else
                                nXTemp += aOverlappingRect.Right() - aNewRect.Left();
                            bMoveAllowed = _nXMov != nXTemp;
                            break;
                        case ControlModification::CENTER_VERTICAL:
                            if ( _aObjRect.Top() < aOverlappingRect.Top() )
                                nYTemp += aOverlappingRect.Top() - aNewRect.Top() - aNewRect.getHeight();
                            else
                                nYTemp += aOverlappingRect.Bottom() - aNewRect.Top();
                            bMoveAllowed = _nYMov != nYTemp;
                            break;
                        case ControlModification::HEIGHT_GREATEST:
                        case ControlModification::WIDTH_GREATEST:
                            {
                                Rectangle aIntersectionRect = aNewRect.GetIntersection(aOverlappingRect);
                                if ( !aIntersectionRect.IsEmpty() )
                                {
                                    if ( _nControlModification == ControlModification::WIDTH_GREATEST )
                                    {
                                        if ( aNewRect.Left() < aIntersectionRect.Left() )
                                        {
                                            aNewRect.Right() = aIntersectionRect.Left();
                                        }
                                        else if ( aNewRect.Left() < aIntersectionRect.Right() )
                                        {
                                            aNewRect.Left() = aIntersectionRect.Right();
                                        }
                                    }
                                    else if ( _nControlModification == ControlModification::HEIGHT_GREATEST )
                                    {
                                        if ( aNewRect.Top() < aIntersectionRect.Top() )
                                        {
                                            aNewRect.Bottom() = aIntersectionRect.Top();
                                        }
                                        else if ( aNewRect.Top() < aIntersectionRect.Bottom() )
                                        {
                                            aNewRect.Top() = aIntersectionRect.Bottom();
                                        }
                                    }
                                    nYTemp = aNewRect.getHeight();
                                    bMoveAllowed = _nYMov != nYTemp;
                                    nXTemp = aNewRect.getWidth();
                                    bMoveAllowed = bMoveAllowed && _nXMov != nXTemp;
                                }
                            }
                            break;
                        default:
                            break;
                    }

                    _nXMov = nXTemp;
                    _nYMov = nYTemp;
                }
                else
                    pOverlappedObj = NULL;
            }
        }
        while ( pOverlappedObj && bMoveAllowed );
    }
    return bMoveAllowed;
}
// -----------------------------------------------------------------------------

DBG_NAME( rpt_OViewsWindow );
OViewsWindow::OViewsWindow( Window* _pParent,OReportWindow* _pReportWindow)
: Window( _pParent,WB_DIALOGCONTROL)
,OPropertyChangeListener(m_aMutex)
,m_pParent(_pReportWindow)
,m_bInSplitHandler(sal_False)
,m_bInUnmark(sal_False)
{
    DBG_CTOR( rpt_OViewsWindow,NULL);
    SetUniqueId(UID_RPT_VIEWSWINDOW);
    SetMapMode( MapMode( MAP_100TH_MM ) );
    StartListening(m_aColorConfig);
    ImplInitSettings();
}
// -----------------------------------------------------------------------------
OViewsWindow::~OViewsWindow()
{
    EndListening(m_aColorConfig);
    try
    {
        TSectionsMap::iterator aIter = m_aSections.begin();
        TSectionsMap::iterator aEnd = m_aSections.end();
        for (;aIter != aEnd ; ++aIter)
            aIter->first.second->dispose();
    }
    catch (uno::Exception&)
    {
    }
    m_aSections.clear();

    DBG_DTOR( rpt_OViewsWindow,NULL);
}
// -----------------------------------------------------------------------------
void OViewsWindow::initialize()
{

}
//------------------------------------------------------------------------------
void OViewsWindow::Resize()
{
    Window::Resize();
    if ( m_aSections.empty() )
        return;
    Point aStartPoint(0,0);

    aStartPoint -= m_pParent->getScrollOffset();
    Point aSplitterStartPoint(aStartPoint.X(),0);
    Size aOutputSize = GetOutputSizePixel();
    aOutputSize.Width() -= (REPORT_ENDMARKER_WIDTH + REPORT_STARTMARKER_WIDTH);

    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (USHORT nPos=0;aIter != aEnd ; ++aIter,++nPos)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = (*aIter).first.first;
        uno::Reference< report::XSection> xSection = pReportSection->getSection();
        Size aSectionSize = LogicToPixel( Size( 0,xSection->getHeight() ),MAP_100TH_MM );
        aSectionSize.Width() = aOutputSize.Width();

        ::boost::shared_ptr<Splitter>       pSplitter       = (*aIter).second.second;
        ::boost::shared_ptr<OEndMarker>     pEndMarker      = (*aIter).second.first;

        const sal_Int32 nMinHeight = m_pParent->getMinHeight(nPos);
        if ( pReportSection->IsVisible() )
        {
            pReportSection->SetPosSizePixel(aStartPoint,aSectionSize);
            if ( !m_bInSplitHandler )
            {
                pSplitter->SetPosSizePixel(Point(aSplitterStartPoint.X(),aStartPoint.Y() + aSectionSize.Height()),Size(aOutputSize.Width(),pSplitter->GetSizePixel().Height()));
                pSplitter->SetDragRectPixel( Rectangle(Point(aSplitterStartPoint.X(),aStartPoint.Y()/*- 1*/),aOutputSize));
            }

            if ( nMinHeight > aSectionSize.Height() )
            {
                pEndMarker->SetPosSizePixel(Point(aSplitterStartPoint.X() + aOutputSize.Width(),aStartPoint.Y()),Size(REPORT_ENDMARKER_WIDTH,nMinHeight));
                aSectionSize.Height() = nMinHeight;
            }
            else
            {
                pEndMarker->SetPosSizePixel(Point(aSplitterStartPoint.X() + aOutputSize.Width(),aStartPoint.Y()),Size(REPORT_ENDMARKER_WIDTH,aSectionSize.Height()));
            }
        }
        else
        {
            aSectionSize.Height() = nMinHeight;
            pEndMarker->SetPosSizePixel(Point(aSplitterStartPoint.X(),aStartPoint.Y()),Size(aOutputSize.Width() + REPORT_ENDMARKER_WIDTH,aSectionSize.Height()));
        }
        aStartPoint.Y() += aSectionSize.Height() + pSplitter->GetSizePixel().Height();
    } // for (;aIter != aEnd ; ++aIter)
}
//------------------------------------------------------------------------------
void OViewsWindow::ImplInitSettings()
{
//#if OSL_DEBUG_LEVEL > 0
//    SetBackground( Wallpaper( COL_GREEN ));
//#else
    SetBackground( Wallpaper( m_aColorConfig.GetColorValue(::svtools::APPBACKGROUND).nColor ) );
//#endif
    SetFillColor( Application::GetSettings().GetStyleSettings().GetDialogColor() );
    SetTextFillColor( Application::GetSettings().GetStyleSettings().GetDialogColor() );
}
//-----------------------------------------------------------------------------
void OViewsWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}
//----------------------------------------------------------------------------
void OViewsWindow::addSection(const uno::Reference< report::XSection >& _xSection,const ::rtl::OUString& _sColorEntry,USHORT _nPosition)
{
    ::boost::shared_ptr<OReportSection> pReportSection( new OReportSection(this,_xSection) );
    pReportSection->Show();

    ::boost::shared_ptr<OEndMarker> pEndMarker( new OEndMarker(this,_sColorEntry) );
    pEndMarker->Show();

    ::boost::shared_ptr<Splitter> pSplitter(new Splitter(this));
    pSplitter->SetStartSplitHdl(LINK(this, OViewsWindow,StartSplitHdl));
    pSplitter->SetSplitHdl(LINK(this, OViewsWindow,SplitHdl));
    pSplitter->SetEndSplitHdl(LINK(this, OViewsWindow,EndSplitHdl));
    pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
    pSplitter->SetSplitPosPixel(LogicToPixel(Size(0,getTotalHeight() + _xSection->getHeight()),MAP_100TH_MM).Height());
    //pSplitter->SetSizePixel(Size(pSplitter->GetSizePixel().Width(),1));
    pSplitter->Show();

    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer> pMulti = new OPropertyChangeMultiplexer(this,_xSection.get());
    pMulti->addProperty(PROPERTY_HEIGHT);

    m_aSections.insert(getIteratorAtPos(_nPosition) , TSectionsMap::value_type(TReportPair(pReportSection,pMulti),TSplitterPair(pEndMarker,pSplitter)));
    m_pParent->setMarked(pReportSection->getView(),m_aSections.size() == 1);

    Resize();
}
//----------------------------------------------------------------------------
void OViewsWindow::removeSection(USHORT _nPosition)
{
    if ( _nPosition < m_aSections.size() )
    {
        TSectionsMap::iterator aPos = getIteratorAtPos(_nPosition);
        TSectionsMap::iterator aNew = getIteratorAtPos(_nPosition == 0 ? _nPosition+1: _nPosition - 1);

        m_pParent->getReportView()->UpdatePropertyBrowserDelayed(aNew->first.first->getView());

        aPos->first.second->dispose();
        aPos->first.second = NULL;
        m_aSections.erase(aPos);
        Resize();
    } // if ( _nPosition < m_aSections.size() )
}
//----------------------------------------------------------------------------
void OViewsWindow::showView(USHORT _nPos,BOOL _bShow)
{
    if ( _nPos < m_aSections.size() )
    {
        TSectionsMap::iterator aPos = getIteratorAtPos(_nPos);
        aPos->first.first->Show(_bShow);
        aPos->second.first->setCollapsed(_bShow);
        aPos->second.second->Show(_bShow);
    }
}
//------------------------------------------------------------------------------
void OViewsWindow::toggleGrid(sal_Bool _bVisible)
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::std::compose1(::boost::bind(&OReportSection::SetGridVisible,_1,_bVisible),TReportPairHelper()));
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::std::compose1(::boost::bind(&OReportSection::Window::Invalidate,_1,INVALIDATE_NOERASE),TReportPairHelper()));
}
//------------------------------------------------------------------------------
sal_Int32 OViewsWindow::getTotalHeight(const OReportSection* _pSection) const
{
    sal_Int32 nHeight = 0;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (;aIter != aEnd && _pSection != (*aIter).first.first.get() ; ++aIter)
    {
        const ::boost::shared_ptr<OReportSection>   pReportSection  = (*aIter).first.first;
        const ::boost::shared_ptr<Splitter>     pSplitter       = (*aIter).second.second;

        uno::Reference< report::XSection> xSection = pReportSection->getSection();
        const Size aSectionSize = LogicToPixel( Size( 0 ,xSection->getHeight() ) ,MAP_100TH_MM);
        nHeight += aSectionSize.Height() + pSplitter->GetSizePixel().Height();
    }

    return nHeight;
}
//----------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
sal_Int32 OViewsWindow::getSplitterHeight() const
{
    sal_Int32 nRet = 0;
    if ( !m_aSections.empty() )
        nRet = m_aSections.begin()->second.second->GetSizePixel().Height();
    return nRet;
}
//-----------------------------------------------------------------------------
IMPL_LINK( OViewsWindow, StartSplitHdl, Splitter*,  )
{
    const String sEmpty(ModuleRes(RID_STR_UNDO_CHANGE_SIZE));
    getView()->getReportView()->getController()->getUndoMgr()->EnterListAction(sEmpty,String());
    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK( OViewsWindow, EndSplitHdl, Splitter*,  )
{
    getView()->getReportView()->getController()->getUndoMgr()->LeaveListAction();
    Resize();
    return 0L;
}
//-----------------------------------------------------------------------------
IMPL_LINK( OViewsWindow, SplitHdl, Splitter*, _pSplitter )
{
    if ( !getView()->getReportView()->getController()->isEditable() )
    {
        m_bInSplitHandler = sal_False;
        return 0L;
    }

    //m_bInSplitHandler = sal_True;
    sal_Int32 nSplitPos = _pSplitter->GetSplitPosPixel();
    const Point aPos = _pSplitter->GetPosPixel();
    _pSplitter->SetPosPixel( Point( 0,nSplitPos ));
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (;aIter != aEnd ; ++aIter)
    {
        if ( (*aIter).second.second.get() == _pSplitter )
        {
            uno::Reference< report::XSection> xSection = (*aIter).first.first->getSection();
            nSplitPos = xSection->getHeight() + PixelToLogic(Size(0,nSplitPos-aPos.Y()),MAP_100TH_MM).Height();

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
            break;
        }
    }
    m_bInSplitHandler = sal_False;
    return 0L;
}
// -----------------------------------------------------------------------------
void OViewsWindow::_propertyChanged(const beans::PropertyChangeEvent& _rEvent) throw( uno::RuntimeException)
{
    uno::Reference< report::XSection > xSection(_rEvent.Source,uno::UNO_QUERY);
    if ( xSection.is() )
    {
        if ( _rEvent.PropertyName.equals(PROPERTY_HEIGHT) )
        {
            TSectionsMap::iterator aIter = m_aSections.begin();
            TSectionsMap::iterator aEnd = m_aSections.end();
            for (;aIter != aEnd ; ++aIter)
            {
                ::boost::shared_ptr<OReportSection> pReportSection = (*aIter).first.first;
                uno::Reference< report::XSection> xCurrentSection = pReportSection->getSection();
                if ( xCurrentSection == xSection )
                {
                    Resize();
                    pReportSection->Invalidate();
                    // end marker
                    (*aIter).second.first->Invalidate();
                    break;
                }
            } // for (;aIter != aEnd ; ++aIter)
        }
    } // if ( xSection.is() )
}
//----------------------------------------------------------------------------
USHORT OViewsWindow::getSectionCount() const
{
    return static_cast<USHORT>(m_aSections.size());
}
//----------------------------------------------------------------------------
void OViewsWindow::SetInsertObj( USHORT eObj,const ::rtl::OUString& _sShapeType )
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (;aIter != aEnd ; ++aIter)
        (*aIter).first.first->getView()->SetCurrentObj( eObj, ReportInventor );

    m_sShapeType = _sShapeType;
}
//----------------------------------------------------------------------------
rtl::OUString OViewsWindow::GetInsertObjString() const
{
    return m_sShapeType;
}

//------------------------------------------------------------------------------
void OViewsWindow::SetMode( DlgEdMode eNewMode )
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::std::compose1(::boost::bind(&OReportSection::SetMode,_1,eNewMode),TReportPairHelper()));
}
//----------------------------------------------------------------------------
BOOL OViewsWindow::HasSelection()
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (;aIter != aEnd && !(*aIter).first.first->getView()->AreObjectsMarked(); ++aIter)
        ;
    return aIter != aEnd;
}
//----------------------------------------------------------------------------
void OViewsWindow::Delete()
{
    m_bInUnmark = sal_True;
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::std::compose1(::boost::mem_fn(&OReportSection::Delete),TReportPairHelper()));
    m_bInUnmark = sal_False;
}
//----------------------------------------------------------------------------
void OViewsWindow::Copy()
{
    uno::Sequence< beans::NamedValue > aAllreadyCopiedObjects;
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd; ++aIter)
        aIter->first.first->Copy(aAllreadyCopiedObjects);
    OReportExchange* pCopy = new OReportExchange(aAllreadyCopiedObjects);
    uno::Reference< datatransfer::XTransferable> aEnsureDelete = pCopy;
    pCopy->CopyToClipboard(this);
}
//----------------------------------------------------------------------------
void OViewsWindow::Paste()
{
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(this));
    OReportExchange::TSectionElements aCopies = OReportExchange::extractCopies(aTransferData);
    if ( aCopies.getLength() > 1 )
        ::std::for_each(m_aSections.begin(),m_aSections.end(),
            ::std::compose1(::boost::bind(&OReportSection::Paste,_1,aCopies,false),TReportPairHelper()));
    else
    {
        ::boost::shared_ptr<OReportSection> pMarkedSection = getMarkedSection();
        if ( pMarkedSection )
            pMarkedSection->Paste(aCopies,true);
    }
}
//----------------------------------------------------------------------------
::boost::shared_ptr<OReportSection> OViewsWindow::getMarkedSection(NearSectionAccess nsa) const
{
    ::boost::shared_ptr<OReportSection> pRet;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    sal_uInt32 nCurrentPosition = 0;
    for (; aIter != aEnd ; ++aIter)
    {
        if ( (*aIter).second.first->isMarked() )
        {
            if (nsa == CURRENT)
            {
                pRet = (*aIter).first.first;
                break;
            }
            else if ( nsa == PREVIOUS )
            {
                if (nCurrentPosition > 0)
                {
                    const TSectionPair aPair = (*(--aIter));
                    pRet = aPair.first.first;
                    if (pRet == NULL)
                    {
                        pRet = (*m_aSections.begin()).first.first;
                    }
                }
                else
                {
                    // if we are out of bounds return the first one
                    pRet = (*m_aSections.begin()).first.first;
                }
                break;
            }
            else if ( nsa == POST )
            {
                sal_uInt32 nSize = m_aSections.size();
                if ((nCurrentPosition + 1) < nSize)
                {
                    const TSectionPair aPair = (*(++aIter));
                    pRet = aPair.first.first;
                    if (pRet == NULL)
                    {
                        pRet = (*(--aEnd)).first.first;
                    }
                }
                else
                {
                    // if we are out of bounds return the last one
                    pRet = (*(--aEnd)).first.first;
                }
                break;
            }
        } // ( (*aIter).second.first->isMarked() )
        nCurrentPosition ++;
    } // for (; aIter != aEnd ; ++aIter)

    return pRet;
}
// -------------------------------------------------------------------------
void OViewsWindow::markSection(const sal_uInt16 _nPos)
{
    if ( _nPos < m_aSections.size() )
        m_pParent->setMarked(getIteratorAtPos(_nPos)->first.first->getSection(),sal_True);
}
//----------------------------------------------------------------------------
BOOL OViewsWindow::IsPasteAllowed()
{
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(this));
    return aTransferData.HasFormat(OReportExchange::getDescriptorFormatId());
}
//-----------------------------------------------------------------------------
void OViewsWindow::SelectAll()
{
    m_bInUnmark = sal_True;
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::std::compose1(::boost::mem_fn(&OReportSection::SelectAll),TReportPairHelper()));
    m_bInUnmark = sal_False;
}
//----------------------------------------------------------------------------
void OViewsWindow::SectionHasFocus(OReportSection* /*_pSection*/,BOOL /*_bHasFocus*/)
{
/* LLA!: this function does nothing!
TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (USHORT i = 0 ; aIter != aEnd ; ++aIter,++i)
    {
        if ( aIter->first.first.get() == _pSection )
        {

        }
    }
*/
}
//-----------------------------------------------------------------------------
void OViewsWindow::unmarkAllObjects(OSectionView* _pSectionView)
{
    if ( !m_bInUnmark )
    {
        m_bInUnmark = sal_True;
        TSectionsMap::iterator aIter = m_aSections.begin();
        TSectionsMap::iterator aEnd = m_aSections.end();
        for (; aIter != aEnd ; ++aIter)
        {
            if ( (*aIter).first.first->getView() != _pSectionView )
                (*aIter).first.first->getView()->UnmarkAllObj();
        } // for (; aIter != aEnd ; ++aIter)
        m_bInUnmark = sal_False;
    }
}
//-----------------------------------------------------------------------------
::boost::shared_ptr<OReportSection> OViewsWindow::getReportSection(const uno::Reference< report::XSection >& _xSection)
{
    OSL_ENSURE(_xSection.is(),"Section is NULL!");
    ::boost::shared_ptr<OReportSection> pRet;
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd ; ++aIter)
    {
        if ( (*aIter).first.first->getSection() == _xSection )
        {
            pRet = (*aIter).first.first;
            break;
        } // if ( (*aIter).first.first->getSection() == _xSection )
    } // for (; aIter != aEnd ; ++aIter)
    return pRet;
}
// -----------------------------------------------------------------------
void OViewsWindow::Notify(SfxBroadcaster & /*rBc*/, SfxHint const & rHint)
{
    if (rHint.ISA(SfxSimpleHint)
        && (static_cast< SfxSimpleHint const & >(rHint).GetId()
            == SFX_HINT_COLORS_CHANGED))
    {
        ImplInitSettings();
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        GrabFocus();
        const uno::Sequence< beans::PropertyValue> aArgs;
        getView()->getReportView()->getController()->executeChecked(SID_SELECT_REPORT,aArgs);
    }
    Window::MouseButtonDown(rMEvt);
}
// -----------------------------------------------------------------------------
void OViewsWindow::showProperties(const OEndMarker* _pEndMarker)
{
    OSL_PRECOND(_pEndMarker,"End marker is null!");
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (;aIter != aEnd && (*aIter).second.first.get() != _pEndMarker; ++aIter)
        ;
    if ( aIter != aEnd )
    {
        uno::Reference< report::XSection> xSection = (*aIter).first.first->getSection();
        m_pParent->showProperties( xSection.get() );
    }
}
//----------------------------------------------------------------------------
void OViewsWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        TSectionsMap::iterator aIter = m_aSections.begin();
        TSectionsMap::iterator aEnd = m_aSections.end();
        for (;aIter != aEnd ; ++aIter)
        {
            if ( (*aIter).first.first->getView()->AreObjectsMarked() )
            {
                (*aIter).first.first->MouseButtonUp(rMEvt);
                break;
            }
        }

        // remove special insert mode
        for (aIter = m_aSections.begin();aIter != aEnd ; ++aIter)
        {
            ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
            pReportSection->getPage()->resetSpecialMode();
        }
    }
}
//------------------------------------------------------------------------------
sal_Bool OViewsWindow::handleKeyEvent(const KeyEvent& _rEvent)
{
    sal_Bool bRet = sal_False;
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (;aIter != aEnd ; ++aIter)
    {
        //if ( (*aIter).first.first->getView()->AreObjectsMarked() )
        if ( (*aIter).second.first->isMarked() )
        {
            bRet = (*aIter).first.first->handleKeyEvent(_rEvent);
        }
    }
    return bRet;
}
//----------------------------------------------------------------------------
OViewsWindow::TSectionsMap::iterator OViewsWindow::getIteratorAtPos(USHORT _nPos)
{
    TSectionsMap::iterator aRet = m_aSections.end();
    if ( _nPos < m_aSections.size() )
        aRet = m_aSections.begin() + _nPos;
    return aRet;
}
//------------------------------------------------------------------------
void OViewsWindow::setMarked(OSectionView* _pSectionView,sal_Bool _bMark)
{
    OSL_ENSURE(_pSectionView != NULL,"SectionView is NULL!");
    if ( _pSectionView )
        setMarked(_pSectionView->getSectionWindow()->getSection(),_bMark);
}
//------------------------------------------------------------------------
void OViewsWindow::setMarked(const uno::Reference< report::XSection>& _xSection,sal_Bool _bMark)
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd ; ++aIter)
    {
        if ( (*aIter).first.first->getSection() != _xSection )
        {
            (*aIter).second.first->setMarked(sal_False);
            (*aIter).second.first->Invalidate(INVALIDATE_NOCHILDREN|INVALIDATE_NOERASE);
        }
        else if ( (*aIter).second.first->isMarked() != _bMark )
        {
            (*aIter).second.first->setMarked(_bMark);
            (*aIter).second.first->Invalidate(INVALIDATE_NOCHILDREN|INVALIDATE_NOERASE);
        }
    }
}
//------------------------------------------------------------------------
void OViewsWindow::setMarked(const uno::Sequence< uno::Reference< report::XReportComponent> >& _aShapes,sal_Bool _bMark)
{
    bool bFirst = true;
    const uno::Reference< report::XReportComponent>* pIter = _aShapes.getConstArray();
    const uno::Reference< report::XReportComponent>* pEnd     = pIter + _aShapes.getLength();
    for(;pIter != pEnd;++pIter)
    {
        uno::Reference< report::XSection> xSection = (*pIter)->getSection();
        if ( xSection.is() )
        {
            if ( bFirst )
            {
                bFirst = false;
                m_pParent->setMarked(xSection,_bMark);
            }
            ::boost::shared_ptr<OReportSection> pSection = getReportSection(xSection);
            if ( pSection )
            {
                SvxShape* pShape = SvxShape::getImplementation( *pIter );
                SdrObject* pObject = pShape ? pShape->GetSdrObject() : NULL;
                OSL_ENSURE( pObject, "OViewsWindow::setMarked: no SdrObject for the shape!" );
                if ( pObject )
                    pSection->getView()->MarkObj( pObject, pSection->getView()->GetSdrPageView(), !_bMark );
            }
        }
    }
}
//------------------------------------------------------------------------
::boost::shared_ptr<OReportSection> OViewsWindow::getSection(const Point& _aPosition)
{
    ::boost::shared_ptr<OReportSection> pRet;
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd ; ++aIter)
    {
        Rectangle aWorkArea((*aIter).first.first->ScreenToOutputPixel((*aIter).first.first->GetPosPixel()),(*aIter).first.first->GetOutputSizePixel());
        aWorkArea = (*aIter).first.first->PixelToLogic( aWorkArea );
        if ( aWorkArea.IsInside(_aPosition) )
        {
            pRet = (*aIter).first.first;
        }
    }
    return pRet;
}
// -----------------------------------------------------------------------------
void OViewsWindow::collectRectangles(TRectangleMap& _rSortRectangles,  bool _bBoundRects)
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (aIter = m_aSections.begin();aIter != aEnd ; ++aIter)
    {
        OSectionView* pView = (*aIter).first.first->getView();
        if ( pView->AreObjectsMarked() )
        {
            pView->SortMarkedObjects();
            const sal_uInt32 nCount = pView->GetMarkedObjectCount();
            for (sal_uInt32 i=0; i < nCount; ++i)
            {
                const SdrMark* pM = pView->GetSdrMarkByIndex(i);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                Rectangle aObjRect(_bBoundRects ? pObj->GetCurrentBoundRect() : pObj->GetSnapRect());
                _rSortRectangles.insert(TRectangleMap::value_type(aObjRect,TRectangleMap::mapped_type(pObj,pView)));
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::collectBoundResizeRect(const TRectangleMap& _rSortRectangles,sal_Int32 _nControlModification,bool _bAlignAtSection, bool _bBoundRects,Rectangle& _rBound,Rectangle& _rResize)
{
    bool bOnlyOnce = false;
    TRectangleMap::const_iterator aRectIter = _rSortRectangles.begin();
    TRectangleMap::const_iterator aRectEnd = _rSortRectangles.end();
    for (;aRectIter != aRectEnd ; ++aRectIter)
    {
        Rectangle aObjRect = aRectIter->first;
        if ( _rResize.IsEmpty() )
            _rResize = aObjRect;
        switch(_nControlModification)
        {
            case ControlModification::WIDTH_SMALLEST:
                if ( _rResize.getWidth() > aObjRect.getWidth() )
                    _rResize = aObjRect;
                break;
            case ControlModification::HEIGHT_SMALLEST:
                if ( _rResize.getHeight() > aObjRect.getHeight() )
                    _rResize = aObjRect;
                break;
            case ControlModification::WIDTH_GREATEST:
                if ( _rResize.getWidth() < aObjRect.getWidth() )
                    _rResize = aObjRect;
                break;
            case ControlModification::HEIGHT_GREATEST:
                if ( _rResize.getHeight() < aObjRect.getHeight() )
                    _rResize = aObjRect;
                break;
        }

        SdrObjTransformInfoRec aInfo;
        const SdrObject* pObj =  aRectIter->second.first;
        pObj->TakeObjInfo(aInfo);
        BOOL bHasFixed = !aInfo.bMoveAllowed || pObj->IsMoveProtect();
        if ( bHasFixed )
            _rBound.Union(aObjRect);
        else
        {
            if ( _bAlignAtSection || _rSortRectangles.size() == 1 )
            { // einzelnes Obj an der Seite ausrichten
                if ( ! bOnlyOnce )
                {
                    bOnlyOnce = true;
                    OReportSection* pReportSection = aRectIter->second.second->getSectionWindow();
                    uno::Reference< report::XSection> xSection = pReportSection->getSection();
                    try
                    {
                        uno::Reference<report::XReportDefinition> xReportDefinition = xSection->getReportDefinition();
                        _rBound.Union(Rectangle(getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN),0,
                                            getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width  - getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN),
                                            xSection->getHeight()));
                    }
                    catch(uno::Exception){}
                }
            }
            else
            {
                if (_bBoundRects)
                    _rBound.Union(aRectIter->second.second->GetMarkedObjBoundRect());
                else
                    _rBound.Union(aRectIter->second.second->GetMarkedObjRect());
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::alignMarkedObjects(sal_Int32 _nControlModification,bool _bAlignAtSection, bool _bBoundRects)
{
    if ( _nControlModification == ControlModification::NONE )
        return;

    Point aRefPoint;
    RectangleLess::CompareMode eCompareMode = RectangleLess::POS_LEFT;
    switch (_nControlModification)
    {
        case ControlModification::TOP   : eCompareMode = RectangleLess::POS_UPPER; break;
        case ControlModification::BOTTOM: eCompareMode = RectangleLess::POS_DOWN; break;
        case ControlModification::LEFT  : eCompareMode = RectangleLess::POS_LEFT; break;
        case ControlModification::RIGHT : eCompareMode = RectangleLess::POS_RIGHT; break;
        case ControlModification::CENTER_HORIZONTAL :
        case ControlModification::CENTER_VERTICAL :
            {
                eCompareMode = (ControlModification::CENTER_VERTICAL == _nControlModification) ?  RectangleLess::POS_CENTER_VERTICAL :  RectangleLess::POS_CENTER_HORIZONTAL;
                uno::Reference<report::XSection> xSection = m_aSections.begin()->first.first->getSection();
                uno::Reference<report::XReportDefinition> xReportDefinition = xSection->getReportDefinition();
                aRefPoint = Rectangle(getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN),0,
                                        getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width  - getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN),
                                        xSection->getHeight()).Center();
            }
            break;
        default: break;
    }
    RectangleLess aCompare(eCompareMode,aRefPoint);
    TRectangleMap aSortRectangles(aCompare);
    collectRectangles(aSortRectangles,_bBoundRects);

    Rectangle aBound;
    Rectangle aResize;
    collectBoundResizeRect(aSortRectangles,_nControlModification,_bAlignAtSection,_bBoundRects,aBound,aResize);

    bool bMove = true;

    ::std::mem_fun_t<long&,Rectangle> aGetFun       = ::std::mem_fun<long&,Rectangle>(&Rectangle::Bottom);
    ::std::mem_fun_t<long&,Rectangle> aRefFun       = ::std::mem_fun<long&,Rectangle>(&Rectangle::Top);
    TRectangleMap::iterator aRectIter = aSortRectangles.begin();
    TRectangleMap::iterator aRectEnd = aSortRectangles.end();
    for (;aRectIter != aRectEnd ; ++aRectIter)
    {
        Rectangle aObjRect = aRectIter->first;
        SdrObject* pObj = aRectIter->second.first;
        SdrView* pView = aRectIter->second.second;
        Point aCenter(aBound.Center());
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo(aInfo);
        if (aInfo.bMoveAllowed && !pObj->IsMoveProtect())
        {
            long nXMov = 0;
            long nYMov = 0;
            long* pValue = &nXMov;
            switch(_nControlModification)
            {
                case ControlModification::TOP   :
                    aGetFun  = ::std::mem_fun<long&,Rectangle>(&Rectangle::Top);
                    aRefFun  = ::std::mem_fun<long&,Rectangle>(&Rectangle::Bottom);
                    pValue = &nYMov;
                    break;
                case ControlModification::BOTTOM:
                    // defaults are already set
                    pValue = &nYMov;
                    break;
                case ControlModification::CENTER_VERTICAL:
                    nYMov = aCenter.Y() - aObjRect.Center().Y();
                    pValue = &nYMov;
                    bMove = false;
                    break;
                case ControlModification::RIGHT :
                    aGetFun  = ::std::mem_fun<long&,Rectangle>(&Rectangle::Right);
                    aRefFun  = ::std::mem_fun<long&,Rectangle>(&Rectangle::Left);
                    break;
                case ControlModification::CENTER_HORIZONTAL:
                    nXMov = aCenter.X() - aObjRect.Center().X();
                    bMove = false;
                    break;
                case ControlModification::LEFT  :
                    aGetFun  = ::std::mem_fun<long&,Rectangle>(&Rectangle::Left);
                    aRefFun  = ::std::mem_fun<long&,Rectangle>(&Rectangle::Right);
                    break;
                default:
                    bMove = false;
                    break;
            }
            if ( bMove )
            {
                Rectangle aTest = aObjRect;
                aGetFun(&aTest) = aGetFun(&aBound);
                TRectangleMap::iterator aInterSectRectIter = aSortRectangles.begin();
                for (; aInterSectRectIter != aRectIter; ++aInterSectRectIter)
                {
                    if ( pView == aInterSectRectIter->second.second && dynamic_cast<OUnoObject*>(aInterSectRectIter->second.first) )
                    {
                        SdrObject* pPreviousObj = aInterSectRectIter->second.first;
                        Rectangle aIntersectRect = aTest.GetIntersection(_bBoundRects ? pPreviousObj->GetCurrentBoundRect() : pPreviousObj->GetSnapRect());
                        if ( !aIntersectRect.IsEmpty() && (aIntersectRect.Left() != aIntersectRect.Right() && aIntersectRect.Top() != aIntersectRect.Bottom() ) )
                        {
                            *pValue = aRefFun(&aIntersectRect) - aGetFun(&aObjRect);
                            break;
                        }
                    }
                }
                if ( aInterSectRectIter == aRectIter )
                    *pValue = aGetFun(&aBound) - aGetFun(&aObjRect);
            }

            if ( lcl_getNewRectSize(aObjRect,nXMov,nYMov,pObj,pView,_nControlModification,_bBoundRects) )
            {
                const Size aSize(nXMov,nYMov);
                pView->AddUndo(pView->GetModel()->GetSdrUndoFactory().CreateUndoMoveObject(*pObj,aSize));
                pObj->Move(aSize);
                aObjRect = (_bBoundRects ? pObj->GetCurrentBoundRect() : pObj->GetSnapRect());
            }

            // resizing control
            if ( !aResize.IsEmpty() && aObjRect != aResize )
            {
                nXMov = aResize.getWidth();
                nYMov = aResize.getHeight();
                switch(_nControlModification)
                {
                    case ControlModification::WIDTH_GREATEST:
                    case ControlModification::HEIGHT_GREATEST:
                        if ( _nControlModification == ControlModification::HEIGHT_GREATEST )
                            nXMov = aObjRect.getWidth();
                        else if ( _nControlModification == ControlModification::WIDTH_GREATEST )
                            nYMov = aObjRect.getHeight();
                        lcl_getNewRectSize(aObjRect,nXMov,nYMov,pObj,pView,_nControlModification,_bBoundRects);
                        // run through
                    case ControlModification::WIDTH_SMALLEST:
                    case ControlModification::HEIGHT_SMALLEST:
                        pView->AddUndo( pView->GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                        {
                            OObjectBase* pObjBase = dynamic_cast<OObjectBase*>(pObj);
                            OSL_ENSURE(pObjBase,"Where comes this object from?");
                            if ( pObjBase )
                            {
                                if ( _nControlModification == ControlModification::WIDTH_SMALLEST || _nControlModification == ControlModification::WIDTH_GREATEST )
                                    pObjBase->getReportComponent()->setSize(awt::Size(nXMov,aObjRect.getHeight()));
                                    //pObj->Resize(aObjRect.TopLeft(),Fraction(nXMov,aObjRect.getWidth()),Fraction(1,1));
                                else if ( _nControlModification == ControlModification::HEIGHT_GREATEST || _nControlModification == ControlModification::HEIGHT_SMALLEST )
                                    pObjBase->getReportComponent()->setSize(awt::Size(aObjRect.getWidth(),nYMov));
                                    //pObj->Resize(aObjRect.TopLeft(),Fraction(1,1),Fraction(nYMov,aObjRect.getHeight()));
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        pView->AdjustMarkHdl();
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::createDefault()
{
    ::boost::shared_ptr<OReportSection> pSection = getMarkedSection();
    if ( pSection )
        pSection->createDefault(m_sShapeType);
}
// -----------------------------------------------------------------------------
void OViewsWindow::setGridSnap(BOOL bOn)
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd ; ++aIter)
    {
        (*aIter).first.first->getView()->SetGridSnap(bOn);
        (*aIter).first.first->Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::setDragStripes(BOOL bOn)
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd ; ++aIter)
        (*aIter).first.first->getView()->SetDragStripes(bOn);
}
// -----------------------------------------------------------------------------
BOOL OViewsWindow::isDragStripes() const
{
    if ( m_aSections.empty() )
        return FALSE;
    return m_aSections.begin()->first.first->getView()->IsDragStripes();
}
// -----------------------------------------------------------------------------

USHORT OViewsWindow::getPosition(const OReportSection* _pSection) const
{
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    USHORT nPosition = 0;
    for (; aIter != aEnd ; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = (*aIter).first.first;
        if (_pSection == pReportSection.get() )
        {
            break;
        }
        ++nPosition;
    }
    return nPosition;
}
// -----------------------------------------------------------------------------
::boost::shared_ptr<OReportSection> OViewsWindow::getSection(const USHORT _nPos) const
{
    ::boost::shared_ptr<OReportSection> aReturn;

    if ( _nPos < m_aSections.size() )
        aReturn = m_aSections[_nPos].first.first;

    return aReturn;
}
// -----------------------------------------------------------------------------
namespace
{
    enum SectionViewAction
    {
        eEndDragObj,
        eEndAction,
        eMoveAction,
        eMarkAction,
        eForceToAnotherPage,
        eBreakAction
    };
    class ApplySectionViewAction : public ::std::unary_function< OViewsWindow::TSectionsMap::value_type, void >
    {
    private:
        SectionViewAction   m_eAction;
        sal_Bool            m_bCopy;
        Point               m_aPoint;

    public:
        ApplySectionViewAction( sal_Bool _bCopy ) : m_eAction( eEndDragObj ), m_bCopy( _bCopy ) { }
        ApplySectionViewAction(SectionViewAction _eAction = eEndAction ) : m_eAction( _eAction ) { }
        ApplySectionViewAction( const Point& _rPoint, SectionViewAction _eAction = eMoveAction ) : m_eAction( _eAction ), m_bCopy( sal_False ), m_aPoint( _rPoint ) { }

        void operator() ( const OViewsWindow::TSectionsMap::value_type& _rhs )
        {
            OSectionView& rView( *_rhs.first.first->getView() );
            switch ( m_eAction )
            {
            case eEndDragObj:
                rView.EndDragObj( m_bCopy  );
                break;
            case eEndAction:
                if ( rView.IsAction() )
                    rView.EndAction (      );
                break;
            case eMoveAction:
                rView.MovAction ( m_aPoint );
                break;
            case eMarkAction:
                rView.BegMarkObj ( m_aPoint );
                break;
            case eForceToAnotherPage:
                rView.ForceMarkedToAnotherPage();
                break;
            case eBreakAction:
                if ( rView.IsAction() )
                    rView.BrkAction (      );
                break;
                // default:

            }
        }
    };
}
// -----------------------------------------------------------------------------
void OViewsWindow::BrkAction()
{
    EndDragObj_removeInvisibleObjects();
    ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction(eBreakAction) );
}
// -----------------------------------------------------------------------------
void OViewsWindow::BegDragObj_createInvisibleObjectAtPosition(const Rectangle& _aRect, const OSectionView* _pSection)
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    Point aNewPos(0,0);

    for (; aIter != aEnd; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
        pReportSection->getPage()->setSpecialMode();
        OSectionView* pView = pReportSection->getView();

        if ( pView != _pSection )
        {
//            SdrRectObj *pNewObj = new SdrRectObj(OBJ_RECT, _aRect);
//          SdrObject *pNewObj = new SdrUnoObj(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Temp Label")));
            SdrObject *pNewObj = new SdrUnoObj(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FixedText")));
            if (pNewObj)
            {
                pNewObj->SetLogicRect(_aRect);
                // pNewObj->SetSize(_aRect.GetSize());
                // pNewObj->Move(Size(_aRect.Left(), _aRect.Top()));

                pNewObj->Move(Size(0, aNewPos.Y()));
                pReportSection->getPage()->InsertObject(pNewObj);
                m_aBegDragTempList.push_back(pNewObj);
                Rectangle aRect = pNewObj->GetLogicRect();

                // pNewObj->SetText(String::CreateFromAscii("Drag helper"));
                pView->MarkObj( pNewObj, pView->GetSdrPageView() );
            }
        }
        const long nSectionHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
        aNewPos.Y() -= nSectionHeight;
//        aNewPos.Y() -= PixelToLogic(aIter->second.second->GetSizePixel()).Height();
    }
}
// -----------------------------------------------------------------------------
bool OViewsWindow::isObjectInMyTempList(SdrObject *_pObj)
{
    ::std::vector<SdrObject*>::iterator aIter = m_aBegDragTempList.begin();
    ::std::vector<SdrObject*>::iterator aEnd = m_aBegDragTempList.end();
    for (; aIter != aEnd; ++aIter)
    {
        if (*aIter == _pObj)
        {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
void OViewsWindow::BegDragObj(const Point& _aPnt, SdrHdl* _pHdl,const OSectionView* _pSection)
{
    OSL_TRACE("BegDragObj Clickpoint X:%d Y:%d\n", _aPnt.X(), _aPnt.Y() );

    m_aBegDragTempList.clear();

    // Calculate the absolute clickpoint in the views
    Point aAbsolutePnt = _aPnt;
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
        OSectionView* pView = pReportSection->getView();
        if (pView == _pSection)
            break;
        const long nSectionHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
        aAbsolutePnt.Y() +=  nSectionHeight;
    }
    m_aDragDelta = Point(SAL_MAX_INT32, SAL_MAX_INT32);
    OSL_TRACE("BegDragObj Absolute X:%d Y:%d\n", aAbsolutePnt.X(), aAbsolutePnt.Y() );

    // Create drag lines over all viewable Views
    // Therefore we need to identify the marked objects
    // and create temporary objects on all other views at the same position
    // relative to its occurance.

    OSL_TRACE("BegDragObj createInvisible Objects\n" );
    int nViewCount = 0;
    Point aNewObjPos(0,0);
    for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;

        OSectionView* pView = pReportSection->getView();

        if ( pView->AreObjectsMarked() )
        {
            const sal_uInt32 nCount = pView->GetMarkedObjectCount();
            for (sal_uInt32 i=0; i < nCount; ++i)
            {
                const SdrMark* pM = pView->GetSdrMarkByIndex(i);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                if (!isObjectInMyTempList(pObj))
                {
                    Rectangle aRect( pObj->GetCurrentBoundRect() );
                    aRect.Move(0, aNewObjPos.Y());
                    OSL_TRACE("BegDragObj createInvisible X:%d Y:%d on View #%d\n", aRect.Left(), aRect.Top(), nViewCount );

                    BegDragObj_createInvisibleObjectAtPosition(aRect, pView);

                    // calculate the clickpoint
                    sal_Int32 nDeltaX = abs(aRect.Left() - aAbsolutePnt.X());
                    sal_Int32 nDeltaY = abs(aRect.Top() - aAbsolutePnt.Y());
                    if (m_aDragDelta.X() > nDeltaX)
                        m_aDragDelta.X() = nDeltaX;
                    if (m_aDragDelta.Y() > nDeltaY)
                        m_aDragDelta.Y() = nDeltaY;
                }
            }
        }
        ++nViewCount;
        Rectangle aClipRect = pView->GetWorkArea();
        aClipRect.Top() = -aNewObjPos.Y();
        pView->SetWorkArea( aClipRect );

        const long nSectionHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
        aNewObjPos.Y() += nSectionHeight;

        // don't subtract the height of the lines between the views
        // aNewObjPos.Y() -= PixelToLogic(aIter->second.second->GetSizePixel()).Height();
    }

    Point aNewPos = aAbsolutePnt;
    // for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    // {
    //     ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
    //     if ( pReportSection->getView() == _pSection )
    //         break;
    //     aNewPos.Y() += pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
    // }

    const short nDrgLog = static_cast<short>(PixelToLogic(Size(3,0)).Width());
    // long nLastSectionHeight = 0;
    // bool bAdd = true;
    nViewCount = 0;
    for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;

        // if ( pReportSection->getView() == _pSection )
        // {
        //     bAdd = false;
        //     aNewPos = _aPnt;
        // }
        // else if ( bAdd )
        // {
        //     const long nSectionHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
        //     aNewPos.Y() += nSectionHeight;
        // }
        // else
        // {
        //     aNewPos.Y() -= nLastSectionHeight;
        // }

        //?
        SdrHdl* pHdl = _pHdl;
        if ( pHdl )
        {
            if ( pReportSection->getView() != _pSection )
            {
                const SdrHdlList& rHdlList = pReportSection->getView()->GetHdlList();
                pHdl = rHdlList.GetHdl(_pHdl->GetKind());
            }
        }
        OSL_TRACE("BegDragObj X:%d Y:%d on View#%d\n", aNewPos.X(), aNewPos.Y(), nViewCount++ );
        pReportSection->getView()->BegDragObj(aNewPos, (OutputDevice*)NULL, pHdl, nDrgLog, NULL);

        const long nSectionHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
        aNewPos.Y() -= nSectionHeight;
        // subtract the height between the views, because they are visible but not from interest here.
        aNewPos.Y() -= PixelToLogic(aIter->second.second->GetSizePixel()).Height();
    }
}

// -----------------------------------------------------------------------------
void OViewsWindow::ForceMarkedToAnotherPage()
{
    ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction(eForceToAnotherPage ) );
}
// -----------------------------------------------------------------------------
void OViewsWindow::BegMarkObj(const Point& _aPnt,const OSectionView* _pSection)
{
    bool bAdd = true;
    Point aNewPos = _aPnt;

    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    long nLastSectionHeight = 0;
    for (; aIter != aEnd; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
        if ( pReportSection->getView() == _pSection )
        {
            bAdd = false;
            aNewPos = _aPnt; // 2,2
        }
        else if ( bAdd )
        {
            const long nSectionHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
            aNewPos.Y() += nSectionHeight;
        }
        else
        {
            aNewPos.Y() -= nLastSectionHeight;
        }
        pReportSection->getView()->BegMarkObj ( aNewPos );
        nLastSectionHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();

        aNewPos.Y() -= PixelToLogic(aIter->second.second->GetSizePixel()).Height();
    }
    //::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction( _aPnt , eMarkAction) );
}
// -----------------------------------------------------------------------------
OSectionView* OViewsWindow::getSectionRelativeToPosition(const OSectionView* _pSection,Point& _rPnt)
{
    OSectionView* pSection = NULL;
    sal_Int32 nCount = 0;
    TSectionsMap::iterator aIter = m_aSections.begin();
    const TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd ; ++aIter,++nCount)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
        if ( pReportSection->getView() == _pSection)
            break;
    }
    OSL_ENSURE(aIter != aEnd,"This can never happen!");
    if ( _rPnt.Y() < 0 )
    {
        --aIter;
        for (; nCount && (_rPnt.Y() < 0); --nCount)
        {
            ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
            _rPnt.Y() += pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
            if ( (nCount -1) > 0 && (_rPnt.Y() < 0) )
                --aIter;
        }
        if ( nCount == 0 )
            pSection = m_aSections.begin()->first.first->getView();
        else
            pSection = aIter->first.first->getView();
    }
    else
    {
        for (; aIter != aEnd; ++aIter)
        {
            ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
            const long nHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
            if ( (_rPnt.Y() - nHeight) < 0  )
                break;
            _rPnt.Y() -= nHeight;
        }
        if ( aIter != aEnd )
            pSection = aIter->first.first->getView();
        else
            pSection = (aEnd-1)->first.first->getView();
    }

    return pSection;
}
// -----------------------------------------------------------------------------
void OViewsWindow::EndDragObj_removeInvisibleObjects()
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();

    for (; aIter != aEnd; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
        pReportSection->getPage()->resetSpecialMode();
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::EndDragObj(BOOL _bControlKeyPressed, const OSectionView* _pSection,const Point& _aPnt)
{
    const String sUndoAction = String((ModuleRes(RID_STR_UNDO_CHANGEPOSITION)));
    UndoManagerListAction aListAction(*getView()->getReportView()->getController()->getUndoMgr(),sUndoAction);

    Point aNewPos = _aPnt;
    OSectionView* pInSection = getSectionRelativeToPosition(_pSection, aNewPos);
    if (!_bControlKeyPressed && _pSection != pInSection)
    {
        EndDragObj_removeInvisibleObjects();

        // we need to manipulate the current clickpoint, we substract the old delta from BeginDrag
        // OSectionView* pInSection = getSectionRelativeToPosition(_pSection, aPnt);
        // aNewPos.X() -= m_aDragDelta.X();
        // aNewPos.Y() -= m_aDragDelta.Y();
        aNewPos -= m_aDragDelta;

        uno::Sequence< beans::NamedValue > aAllreadyCopiedObjects;
        TSectionsMap::iterator aIter = m_aSections.begin();
        const TSectionsMap::iterator aEnd = m_aSections.end();
        for (; aIter != aEnd; ++aIter)
        {
            ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
            if ( pInSection != pReportSection->getView() )
            {
                pReportSection->getView()->BrkAction();
                pReportSection->Copy(aAllreadyCopiedObjects,true);
            }
            else
                pInSection->EndDragObj(FALSE);
        }
        if ( aAllreadyCopiedObjects.getLength() )
        {
            beans::NamedValue* pIter = aAllreadyCopiedObjects.getArray();
            const beans::NamedValue* pEnd = pIter + aAllreadyCopiedObjects.getLength();
            try
            {
                uno::Reference<report::XReportDefinition> xReportDefinition = getView()->getReportView()->getController()->getReportDefinition();
                const sal_Int32 nLeftMargin  = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
                const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
                const sal_Int32 nPaperWidth  = getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width;

                Point aPrevious;
                for (; pIter != pEnd; ++pIter)
                {
                    uno::Sequence< uno::Reference<util::XCloneable> > aClones;
                    pIter->Value >>= aClones;
                    uno::Reference<util::XCloneable>* pColIter = aClones.getArray();
                    const uno::Reference<util::XCloneable>* pColEnd = pColIter + aClones.getLength();
                    for (; pColIter != pColEnd; ++pColIter)
                    {
                        uno::Reference< report::XReportComponent> xRC(*pColIter,uno::UNO_QUERY);
                        aPrevious = VCLPoint(xRC->getPosition());

                        awt::Size aSize = xRC->getSize();
                        if ( aNewPos.X() < nLeftMargin )
                            aNewPos.X() = nLeftMargin;
                        else if ( (aNewPos.X() + aSize.Width) > (nPaperWidth - nRightMargin) )
                            aNewPos.X() = nPaperWidth - nRightMargin - aSize.Width;
                        if ( aNewPos.Y() < 0 )
                            aNewPos.Y() = 0;
                        if ( aNewPos.X() < 0 )
                        {
                            aSize.Width += aNewPos.X();
                            aNewPos.X()= 0;
                            xRC->setSize(aSize);
                        }
                        xRC->setPosition(AWTPoint(aNewPos));
                        if ( (pColIter+1) != pColEnd )
                        {
                            uno::Reference< report::XReportComponent> xRCNext(*(pColIter + 1),uno::UNO_QUERY);
                            Point aNextPosition = VCLPoint(xRCNext->getPosition());
                            aNewPos += (aNextPosition - aPrevious);
                        }
                    }
                }
            }
            catch(uno::Exception&)
            {
            }
            pInSection->getSectionWindow()->Paste(aAllreadyCopiedObjects,true);
        }
        getView()->getReportView()->getController()->getUndoMgr()->LeaveListAction();
    }
    else
    {
        ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction( FALSE ) );
        EndDragObj_removeInvisibleObjects();
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::EndAction()
{
    ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction() );
}
// -----------------------------------------------------------------------------
void OViewsWindow::MovAction(const Point& _aPnt,const OSectionView* _pSection,bool _bMove, bool _bControlKeySet)
{
    (void)_bMove;

    Point aRealMousePos = _aPnt;
    Point aCurrentSectionPos;
    OSL_TRACE("MovAction X:%d Y:%d\n", aRealMousePos.X(), aRealMousePos.Y() );

    Point aHdlPos;
    SdrHdl* pHdl = _pSection->GetDragHdl();
    if ( pHdl )
    {
        aHdlPos = pHdl->GetPos();
    }
    TSectionsMap::iterator aIter/*  = m_aSections.begin() */;
    TSectionsMap::iterator aEnd = m_aSections.end();

    //if ( _bMove )
    //{
    for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
        if ( pReportSection->getView() == _pSection )
            break;
        const long nSectionHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
        aCurrentSectionPos.Y() += nSectionHeight;
        const long nSectionSeparator = PixelToLogic(aIter->second.second->GetSizePixel()).Height();
        aCurrentSectionPos.Y() += nSectionSeparator;
    }
    //}
    aRealMousePos += aCurrentSectionPos;

    // If control key is pressed the work area is limited to the section with the current selection.
    Point aPosForWorkArea(0,0);
    for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
        OSectionView* pView = pReportSection->getView();
        const long nSectionHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
        const long nSeparatorHeight = PixelToLogic(aIter->second.second->GetSizePixel()).Height();

        if (_bControlKeySet)
        {
            Rectangle aClipRect = pView->GetWorkArea();
            aClipRect.Top() = aCurrentSectionPos.Y() - aPosForWorkArea.Y() - nSeparatorHeight;
            // if (aClipRect.Top() < 0) aClipRect.Top() = 0;
            aClipRect.Bottom() = aClipRect.Top() + nSectionHeight;
            pView->SetWorkArea( aClipRect );
        }
        else
        {
            Rectangle aClipRect = pView->GetWorkArea();
            aClipRect.Top() = -aPosForWorkArea.Y();
            pView->SetWorkArea( aClipRect );
        }
        aPosForWorkArea.Y() += nSectionHeight;
        // aNewPos.Y() += PixelToLogic(aIter->second.second->GetSizePixel()).Height();
    }


    for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
        SdrHdl* pCurrentHdl = pReportSection->getView()->GetDragHdl();
        if ( pCurrentHdl )
        {
            aRealMousePos = _aPnt + pCurrentHdl->GetPos() - aHdlPos;
        }
        pReportSection->getView()->MovAction ( aRealMousePos );
        // if ( _bMove )
        // {
        const long nSectionHeight = pReportSection->PixelToLogic(pReportSection->GetOutputSizePixel()).Height();
        aRealMousePos.Y() -= nSectionHeight;
        aRealMousePos.Y() -= PixelToLogic(aIter->second.second->GetSizePixel()).Height();
        // }
    }
}
// -----------------------------------------------------------------------------
BOOL OViewsWindow::IsAction() const
{
    BOOL bAction = FALSE;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (; !bAction && aIter != aEnd; ++aIter)
        bAction = aIter->first.first->getView()->IsAction();
    return bAction;
}
// -----------------------------------------------------------------------------
BOOL OViewsWindow::IsDragObj() const
{
    BOOL bAction = FALSE;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (; !bAction && aIter != aEnd; ++aIter)
        bAction = aIter->first.first->getView()->IsAction();
    return bAction;
}
// -----------------------------------------------------------------------------
void OViewsWindow::setPoint(const Point& _aPnt)
{
    m_aPoint = _aPnt;
}
// -----------------------------------------------------------------------------
sal_uInt32 OViewsWindow::getMarkedObjectCount() const
{
    sal_uInt32 nCount = 0;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (; aIter != aEnd; ++aIter)
        nCount += aIter->first.first->getView()->GetMarkedObjectCount();
    return nCount;
}
// -----------------------------------------------------------------------------
void OViewsWindow::handleKey(const KeyCode& _rCode)
{
    const USHORT nCode = _rCode.GetCode();
    if ( _rCode.IsMod1() )
    {
        // scroll page
        OScrollWindowHelper* pScrollWindow = getView()->getScrollWindow();
        ScrollBar* pScrollBar = ( nCode == KEY_LEFT || nCode == KEY_RIGHT ) ? pScrollWindow->GetHScroll() : pScrollWindow->GetVScroll();
        if ( pScrollBar && pScrollBar->IsVisible() )
            pScrollBar->DoScrollAction(( nCode == KEY_RIGHT || nCode == KEY_UP ) ? SCROLL_LINEUP : SCROLL_LINEDOWN );
        return;
    }
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (; aIter != aEnd; ++aIter)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = aIter->first.first;
        long nX = 0;
        long nY = 0;

        if ( nCode == KEY_UP )
            nY = -1;
        else if ( nCode == KEY_DOWN )
            nY =  1;
        else if ( nCode == KEY_LEFT )
            nX = -1;
        else if ( nCode == KEY_RIGHT )
            nX =  1;

        if ( pReportSection->getView()->AreObjectsMarked() )
        {
            if ( _rCode.IsMod2() )
            {
                // move in 1 pixel distance
                const Size aPixelSize = pReportSection->PixelToLogic( Size( 1, 1 ) );
                nX *= aPixelSize.Width();
                nY *= aPixelSize.Height();
            }
            else
            {
                // move in 1 mm distance
                nX *= DEFAUL_MOVE_SIZE;
                nY *= DEFAUL_MOVE_SIZE;
            }

            OSectionView* pView = pReportSection->getView();
            const SdrHdlList& rHdlList = pView->GetHdlList();
            SdrHdl* pHdl = rHdlList.GetFocusHdl();

            if ( pHdl == 0 )
            {
                // no handle selected
                if ( pView->IsMoveAllowed() )
                {
                    // restrict movement to work area
                    const Rectangle& rWorkArea = pView->GetWorkArea();

                    if ( !rWorkArea.IsEmpty() )
                    {
                        Rectangle aMarkRect( pView->GetMarkedObjRect() );
                        aMarkRect.Move( nX, nY );

                        if ( !rWorkArea.IsInside( aMarkRect ) )
                        {
                            if ( aMarkRect.Left() < rWorkArea.Left() )
                                nX += rWorkArea.Left() - aMarkRect.Left();

                            if ( aMarkRect.Right() > rWorkArea.Right() )
                                nX -= aMarkRect.Right() - rWorkArea.Right();

                            if ( aMarkRect.Top() < rWorkArea.Top() )
                                nY += rWorkArea.Top() - aMarkRect.Top();

                            if ( aMarkRect.Bottom() > rWorkArea.Bottom() )
                                nY -= aMarkRect.Bottom() - rWorkArea.Bottom();
                        }
                        bool bCheck = false;
                        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                        for (sal_uInt32 i =  0; !bCheck && i < rMarkList.GetMarkCount();++i )
                        {
                            SdrMark* pMark = rMarkList.GetMark(i);
                            bCheck = dynamic_cast<OUnoObject*>(pMark->GetMarkedSdrObj()) != NULL;
                        }

                        if ( bCheck && isOver(aMarkRect,*pReportSection->getPage(),*pView) )
                            break;
                    }

                    if ( nX != 0 || nY != 0 )
                    {
                        pView->MoveAllMarked( Size( nX, nY ) );
                        pView->MakeVisible( pView->GetAllMarkedRect(), *pReportSection);
                    }
                }
            }
            else
            {
                // move the handle
                if ( pHdl && ( nX || nY ) )
                {
                    const Point aStartPoint( pHdl->GetPos() );
                    const Point aEndPoint( pHdl->GetPos() + Point( nX, nY ) );
                    const SdrDragStat& rDragStat = pView->GetDragStat();

                    // start dragging
                    pView->BegDragObj( aStartPoint, 0, pHdl, 0 );

                    if ( pView->IsDragObj() )
                    {
                        const FASTBOOL bWasNoSnap = rDragStat.IsNoSnap();
                        const BOOL bWasSnapEnabled = pView->IsSnapEnabled();

                        // switch snapping off
                        if ( !bWasNoSnap )
                            ((SdrDragStat&)rDragStat).SetNoSnap( TRUE );
                        if ( bWasSnapEnabled )
                            pView->SetSnapEnabled( FALSE );

                        Rectangle aNewRect;
                        bool bCheck = false;
                        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                        for (sal_uInt32 i =  0; !bCheck && i < rMarkList.GetMarkCount();++i )
                        {
                            SdrMark* pMark = rMarkList.GetMark(i);
                            bCheck = dynamic_cast<OUnoObject*>(pMark->GetMarkedSdrObj()) != NULL;
                            if ( bCheck )
                                aNewRect.Union(pMark->GetMarkedSdrObj()->GetLastBoundRect());
                        }

                        switch(pHdl->GetKind())
                        {
                            case HDL_LEFT:
                            case HDL_UPLFT:
                            case HDL_LWLFT:
                            case HDL_UPPER:
                                aNewRect.Left() += nX;
                                aNewRect.Top()  += nY;
                                break;
                            case HDL_UPRGT:
                            case HDL_RIGHT:
                            case HDL_LWRGT:
                            case HDL_LOWER:
                                aNewRect.setWidth(aNewRect.getWidth() + nX);
                                aNewRect.setHeight(aNewRect.getHeight() + nY);
                                break;
                            default:
                                break;
                        }
                        if ( !(bCheck && isOver(aNewRect,*pReportSection->getPage(),*pView)) )
                            pView->MovAction(aEndPoint);
                        pView->EndDragObj();

                        // restore snap
                        if ( !bWasNoSnap )
                            ((SdrDragStat&)rDragStat).SetNoSnap( bWasNoSnap );
                        if ( bWasSnapEnabled )
                            pView->SetSnapEnabled( bWasSnapEnabled );
                    }

                    // make moved handle visible
                    const Rectangle aVisRect( aEndPoint - Point( DEFAUL_MOVE_SIZE, DEFAUL_MOVE_SIZE ), Size( 200, 200 ) );
                    pView->MakeVisible( aVisRect, *pReportSection);
                }
            }
            pView->AdjustMarkHdl();
        }
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::stopScrollTimer()
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::std::compose1(::boost::mem_fn(&OReportSection::stopScrollTimer),TReportPairHelper()));
}
//==============================================================================
} // rptui
//==============================================================================

