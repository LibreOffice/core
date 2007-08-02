/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewsWindow.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:41:16 $
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
#include <algorithm>

namespace rptui
{
#define SECTION_OFFSET  3

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
    SetUniqueId(UID_VIEWSWINDOW);
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

    uno::Reference<report::XReportDefinition> xReportDefinition = getView()->getReportView()->getController()->getReportDefinition();
    if ( !xReportDefinition.is() )
        return;
    //sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
    sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
    const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width;
    const Size aOrignalSize = LogicToPixel(Size(nPaperWidth,0),MAP_100TH_MM);
    nRightMargin = LogicToPixel(Size(nRightMargin,0),MAP_100TH_MM).Width();

    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (USHORT nPos=0;aIter != aEnd ; ++aIter,++nPos)
    {
        ::boost::shared_ptr<OReportSection> pReportSection = (*aIter).first.first;
        uno::Reference< report::XSection> xSection = pReportSection->getSection();
        Size aSectionSize = LogicToPixel( Size( nPaperWidth,xSection->getHeight() ),MAP_100TH_MM );

        ::boost::shared_ptr<Splitter>       pSplitter       = (*aIter).second.second;
        ::boost::shared_ptr<OEndMarker>     pEndMarker      = (*aIter).second.first;

        const sal_Int32 nMinHeight = m_pParent->getMinHeight(nPos);
        if ( pReportSection->IsVisible() )
        {
            pReportSection->SetPosSizePixel(aStartPoint,aSectionSize);
            if ( !m_bInSplitHandler )
            {
                pSplitter->SetPosSizePixel(Point(aSplitterStartPoint.X(),aStartPoint.Y() + aSectionSize.Height()),Size(aOrignalSize.Width(),pSplitter->GetSizePixel().Height()));
                pSplitter->SetDragRectPixel( Rectangle(Point(aSplitterStartPoint.X(),aStartPoint.Y() - 1),Size(aOrignalSize.Width(),m_pParent->GetOutputSizePixel().Height())) );
            }

            if ( nMinHeight > aSectionSize.Height() )
            {
                pEndMarker->SetPosSizePixel(Point(aSplitterStartPoint.X() + aOrignalSize.Width(),aStartPoint.Y()),Size(REPORT_ENDMARKER_WIDTH,nMinHeight));
                aSectionSize.Height() = nMinHeight;
            }
            else
            {
                pEndMarker->SetPosSizePixel(Point(aSplitterStartPoint.X() + aOrignalSize.Width(),aStartPoint.Y()),Size(REPORT_ENDMARKER_WIDTH,aSectionSize.Height()));
            }
        }
        else
        {
            aSectionSize.Height() = nMinHeight;
            pEndMarker->SetPosSizePixel(Point(aSplitterStartPoint.X(),aStartPoint.Y()),Size(aOrignalSize.Width() + REPORT_ENDMARKER_WIDTH,aSectionSize.Height()));
        }
        aStartPoint.Y() += aSectionSize.Height() + pSplitter->GetSizePixel().Height();
    } // for (;aIter != aEnd ; ++aIter)
}
//------------------------------------------------------------------------------
void OViewsWindow::ImplInitSettings()
{
    SetBackground( Wallpaper( m_aColorConfig.GetColorValue(::svtools::APPBACKGROUND).nColor ) );
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
    const String sEmpty(String(ModuleRes(RID_STR_UNDO_CHANGE_SIZE)));
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
    _pSplitter->SetPosPixel( Point( aPos.X(),nSplitPos ));
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
//----------------------------------------------------------------------------
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
                _rSortRectangles.insert(TRectangleMap::value_type(aObjRect,TRectangleMap::data_type(pObj,pView)));
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::collectBoundResizeRect(const TRectangleMap& _rSortRectangles,sal_Int32 _nControlModification,bool _bAlignAtSection, bool _bBoundRects,Rectangle& _rBound,Rectangle& _rResize)
{
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
                    if ( pView == aInterSectRectIter->second.second )
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
                                    pObjBase->getReportComponent()->setSize(awt::Size(nXMov,aObjRect.getWidth()));
                                    //pObj->Resize(aObjRect.TopLeft(),Fraction(nXMov,aObjRect.getWidth()),Fraction(1,1));
                                else if ( _nControlModification == ControlModification::HEIGHT_GREATEST || _nControlModification == ControlModification::HEIGHT_SMALLEST )
                                    pObjBase->getReportComponent()->setSize(awt::Size(nYMov,aObjRect.getHeight()));
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
void OViewsWindow::breakAction()
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::std::compose1(::boost::bind(&OReportSection::breakAction,_1),TReportPairHelper()));
}
// -----------------------------------------------------------------------------
void OViewsWindow::BegDragObj(const Point& _aPnt, SdrHdl* _pHdl)
{
    const short nDrgLog = static_cast<short>(PixelToLogic(Size(3,0)).Width());
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd; ++aIter)
        aIter->first.first->getView()->BegDragObj(_aPnt, (OutputDevice*)NULL, _pHdl, nDrgLog,NULL);
}

// -----------------------------------------------------------------------------
namespace
{
    enum SectionViewAction
    {
        eEndDragObj,
        eEndAction,
        eMoveAction
    };
    struct ApplySectionViewAction : public ::std::unary_function< OViewsWindow::TSectionsMap::value_type, void >
    {
    private:
        SectionViewAction   m_eAction;
        sal_Bool            m_bCopy;
        Point               m_aPoint;

    public:
        ApplySectionViewAction( sal_Bool _bCopy ) : m_eAction( eEndDragObj ), m_bCopy( _bCopy ) { }
        ApplySectionViewAction() : m_eAction( eEndAction ) { }
        ApplySectionViewAction( const Point& _rPoint ) : m_eAction( eMoveAction ), m_bCopy( sal_False ), m_aPoint( _rPoint ) { }

        void operator() ( const OViewsWindow::TSectionsMap::value_type& _rhs )
        {
            OSectionView& rView( *_rhs.first.first->getView() );
            switch ( m_eAction )
            {
            case eEndDragObj:   rView.EndDragObj( m_bCopy  ); break;
            case eEndAction:    rView.EndAction (          ); break;
            case eMoveAction:   rView.MovAction ( m_aPoint ); break;
            }
        }
    };
}

// -----------------------------------------------------------------------------
void OViewsWindow::EndDragObj(BOOL _bCopy)
{
    ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction( _bCopy ) );
}
// -----------------------------------------------------------------------------
void OViewsWindow::EndAction()
{
    ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction() );
}
// -----------------------------------------------------------------------------
void OViewsWindow::MovAction(const Point& rPnt)
{
    ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction( rPnt ) );
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
//==============================================================================
} // rptui
//==============================================================================

