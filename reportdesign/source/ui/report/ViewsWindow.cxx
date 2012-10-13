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

#include "ViewsWindow.hxx"
#include "ScrollHelper.hxx"
#include "UndoActions.hxx"
#include "ReportWindow.hxx"
#include "DesignView.hxx"
#include <svtools/colorcfg.hxx>
#include "ReportController.hxx"
#include "UITools.hxx"
#include "RptDef.hxx"
#include "RptResId.hrc"
#include "SectionView.hxx"
#include "ReportSection.hxx"
#include "uistrings.hrc"
#include "rptui_slotid.hrc"
#include "dlgedclip.hxx"
#include "ColorChanger.hxx"
#include "RptObject.hxx"
#include "RptObject.hxx"
#include "ModuleHelper.hxx"
#include "EndMarker.hxx"
#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <vcl/svapp.hxx>
#include <boost/bind.hpp>

#include "helpids.hrc"
#include <svx/svdundo.hxx>
#include <toolkit/helper/convert.hxx>
#include <algorithm>
#include <numeric>
#include <o3tl/compat_functional.hxx>

namespace rptui
{
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
            if (dynamic_cast<OUnoObject*>(_pObj) != NULL || dynamic_cast<OOle2Obj*>(_pObj) != NULL)
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
OViewsWindow::OViewsWindow( OReportWindow* _pReportWindow)
: Window( _pReportWindow,WB_DIALOGCONTROL)
,m_pParent(_pReportWindow)
,m_bInUnmark(sal_False)
{
    DBG_CTOR( rpt_OViewsWindow,NULL);
    SetPaintTransparent(sal_True);
    SetUniqueId(UID_RPT_VIEWSWINDOW);
    SetMapMode( MapMode( MAP_100TH_MM ) );
    m_aColorConfig.AddListener(this);
    ImplInitSettings();
}
// -----------------------------------------------------------------------------
OViewsWindow::~OViewsWindow()
{
    m_aColorConfig.RemoveListener(this);
    m_aSections.clear();

    DBG_DTOR( rpt_OViewsWindow,NULL);
}
// -----------------------------------------------------------------------------
void OViewsWindow::initialize()
{

}
// -----------------------------------------------------------------------------
void OViewsWindow::impl_resizeSectionWindow(OSectionWindow& _rSectionWindow,Point& _rStartPoint,bool _bSet)
{
    const uno::Reference< report::XSection> xSection = _rSectionWindow.getReportSection().getSection();

    Size aSectionSize = _rSectionWindow.LogicToPixel( Size( 0,xSection->getHeight() ) );
    aSectionSize.Width() = getView()->GetTotalWidth();

    const sal_Int32 nMinHeight = _rSectionWindow.getStartMarker().getMinHeight();
    if ( _rSectionWindow.getStartMarker().isCollapsed() || nMinHeight > aSectionSize.Height() )
    {
        aSectionSize.Height() = nMinHeight;
    }
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    aSectionSize.Height() += (long)(rSettings.GetSplitSize() * (double)_rSectionWindow.GetMapMode().GetScaleY());

    if ( _bSet )
        _rSectionWindow.SetPosSizePixel(_rStartPoint,aSectionSize);

    _rStartPoint.Y() += aSectionSize.Height();
}

// -----------------------------------------------------------------------------
void OViewsWindow::resize(const OSectionWindow& _rSectionWindow)
{
    bool bSet = false;
    Point aStartPoint;
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (;aIter != aEnd ; ++aIter)
    {
        const ::boost::shared_ptr<OSectionWindow> pSectionWindow = (*aIter);
        if ( pSectionWindow.get() == &_rSectionWindow )
        {
            aStartPoint = pSectionWindow->GetPosPixel();
            bSet = true;
        }

        if ( bSet )
        {
            impl_resizeSectionWindow(*pSectionWindow.get(),aStartPoint,bSet);
            static sal_Int32 nIn = INVALIDATE_UPDATE | INVALIDATE_TRANSPARENT;
            pSectionWindow->getStartMarker().Invalidate( nIn ); // INVALIDATE_NOERASE |INVALIDATE_NOCHILDREN| INVALIDATE_TRANSPARENT
            pSectionWindow->getEndMarker().Invalidate( nIn );
        }
    }
    Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH));
    aStartWidth *= GetMapMode().GetScaleX();
    Size aOut = GetOutputSizePixel();
    aOut.Width() = aStartWidth;
    aOut = PixelToLogic(aOut);
    m_pParent->notifySizeChanged();

    Rectangle aRect(PixelToLogic(Point(0,0)),aOut);
}
//------------------------------------------------------------------------------
void OViewsWindow::Resize()
{
    Window::Resize();
    if ( !m_aSections.empty() )
    {
        const Point aOffset(m_pParent->getThumbPos());
        Point aStartPoint(0,-aOffset.Y());
        TSectionsMap::iterator aIter = m_aSections.begin();
        TSectionsMap::iterator aEnd = m_aSections.end();
        for (;aIter != aEnd ; ++aIter)
        {
            const ::boost::shared_ptr<OSectionWindow> pSectionWindow = (*aIter);
            impl_resizeSectionWindow(*pSectionWindow.get(),aStartPoint,true);
        }
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::Paint( const Rectangle& rRect )
{
    Window::Paint( rRect );

    Size aOut = GetOutputSizePixel();
    Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH));
    aStartWidth *= GetMapMode().GetScaleX();

    aOut.Width() -= (long)aStartWidth;
    aOut = PixelToLogic(aOut);

    Rectangle aRect(PixelToLogic(Point(aStartWidth,0)),aOut);
    Wallpaper aWall( m_aColorConfig.GetColorValue(::svtools::APPBACKGROUND).nColor );
    DrawWallpaper(aRect,aWall);
}
//------------------------------------------------------------------------------
void OViewsWindow::ImplInitSettings()
{
    EnableChildTransparentMode( sal_True );
    SetBackground( );
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
void OViewsWindow::addSection(const uno::Reference< report::XSection >& _xSection,const ::rtl::OUString& _sColorEntry,sal_uInt16 _nPosition)
{
    ::boost::shared_ptr<OSectionWindow> pSectionWindow( new OSectionWindow(this,_xSection,_sColorEntry) );
    m_aSections.insert(getIteratorAtPos(_nPosition) , TSectionsMap::value_type(pSectionWindow));
    m_pParent->setMarked(&pSectionWindow->getReportSection().getSectionView(),m_aSections.size() == 1);
    Resize();
}
//----------------------------------------------------------------------------
void OViewsWindow::removeSection(sal_uInt16 _nPosition)
{
    if ( _nPosition < m_aSections.size() )
    {
        TSectionsMap::iterator aPos = getIteratorAtPos(_nPosition);
        TSectionsMap::iterator aNew = getIteratorAtPos(_nPosition == 0 ? _nPosition+1: _nPosition - 1);

        m_pParent->getReportView()->UpdatePropertyBrowserDelayed((*aNew)->getReportSection().getSectionView());

        m_aSections.erase(aPos);
        Resize();
    }
}
//------------------------------------------------------------------------------
void OViewsWindow::toggleGrid(sal_Bool _bVisible)
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::o3tl::compose1(::boost::bind(&OReportSection::SetGridVisible,_1,_bVisible),TReportPairHelper()));
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::o3tl::compose1(::boost::bind(&OReportSection::Window::Invalidate,_1,INVALIDATE_NOERASE),TReportPairHelper()));
}
//------------------------------------------------------------------------------
sal_Int32 OViewsWindow::getTotalHeight() const
{
    sal_Int32 nHeight = 0;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (;aIter != aEnd ; ++aIter)
    {
        nHeight += (*aIter)->GetSizePixel().Height();
    }
    return nHeight;
}
//----------------------------------------------------------------------------
sal_uInt16 OViewsWindow::getSectionCount() const
{
    return static_cast<sal_uInt16>(m_aSections.size());
}
//----------------------------------------------------------------------------
void OViewsWindow::SetInsertObj( sal_uInt16 eObj,const ::rtl::OUString& _sShapeType )
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (;aIter != aEnd ; ++aIter)
        (*aIter)->getReportSection().getSectionView().SetCurrentObj( eObj, ReportInventor );

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
        ::o3tl::compose1(::boost::bind(&OReportSection::SetMode,_1,eNewMode),TReportPairHelper()));
}
//----------------------------------------------------------------------------
sal_Bool OViewsWindow::HasSelection() const
{
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (;aIter != aEnd && !(*aIter)->getReportSection().getSectionView().AreObjectsMarked(); ++aIter)
        ;
    return aIter != aEnd;
}
//----------------------------------------------------------------------------
void OViewsWindow::Delete()
{
    m_bInUnmark = sal_True;
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::o3tl::compose1(::boost::mem_fn(&OReportSection::Delete),TReportPairHelper()));
    m_bInUnmark = sal_False;
}
//----------------------------------------------------------------------------
void OViewsWindow::Copy()
{
    uno::Sequence< beans::NamedValue > aAllreadyCopiedObjects;
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::o3tl::compose1(::boost::bind(&OReportSection::Copy,_1,::boost::ref(aAllreadyCopiedObjects)),TReportPairHelper()));

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
            ::o3tl::compose1(::boost::bind(&OReportSection::Paste,_1,aCopies,false),TReportPairHelper()));
    else
    {
        ::boost::shared_ptr<OSectionWindow> pMarkedSection = getMarkedSection();
        if ( pMarkedSection )
            pMarkedSection->getReportSection().Paste(aCopies,true);
    }
}
// ---------------------------------------------------------------------------
::boost::shared_ptr<OSectionWindow> OViewsWindow::getSectionWindow(const uno::Reference< report::XSection>& _xSection) const
{
    OSL_ENSURE(_xSection.is(),"Section is NULL!");

    ::boost::shared_ptr<OSectionWindow> pSectionWindow;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (; aIter != aEnd ; ++aIter)
    {
        if ((*aIter)->getReportSection().getSection() == _xSection)
        {
            pSectionWindow = (*aIter);
            break;
        }
    }

    return pSectionWindow;
}

//----------------------------------------------------------------------------
::boost::shared_ptr<OSectionWindow> OViewsWindow::getMarkedSection(NearSectionAccess nsa) const
{
    ::boost::shared_ptr<OSectionWindow> pRet;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    sal_uInt32 nCurrentPosition = 0;
    for (; aIter != aEnd ; ++aIter)
    {
        if ( (*aIter)->getStartMarker().isMarked() )
        {
            if (nsa == CURRENT)
            {
                pRet = (*aIter);
                break;
            }
            else if ( nsa == PREVIOUS )
            {
                if (nCurrentPosition > 0)
                {
                    pRet = (*(--aIter));
                    if (pRet == NULL)
                    {
                        pRet = (*m_aSections.begin());
                    }
                }
                else
                {
                    // if we are out of bounds return the first one
                    pRet = (*m_aSections.begin());
                }
                break;
            }
            else if ( nsa == POST )
            {
                sal_uInt32 nSize = m_aSections.size();
                if ((nCurrentPosition + 1) < nSize)
                {
                    pRet = *(++aIter);
                    if (pRet == NULL)
                    {
                        pRet = (*(--aEnd));
                    }
                }
                else
                {
                    // if we are out of bounds return the last one
                    pRet = (*(--aEnd));
                }
                break;
            }
        }
        ++nCurrentPosition;
    }

    return pRet;
}
// -------------------------------------------------------------------------
void OViewsWindow::markSection(const sal_uInt16 _nPos)
{
    if ( _nPos < m_aSections.size() )
        m_pParent->setMarked(m_aSections[_nPos]->getReportSection().getSection(),sal_True);
}
//----------------------------------------------------------------------------
sal_Bool OViewsWindow::IsPasteAllowed() const
{
    TransferableDataHelper aTransferData( TransferableDataHelper::CreateFromSystemClipboard( const_cast< OViewsWindow* >( this ) ) );
    return aTransferData.HasFormat(OReportExchange::getDescriptorFormatId());
}
//-----------------------------------------------------------------------------
void OViewsWindow::SelectAll(const sal_uInt16 _nObjectType)
{
    m_bInUnmark = sal_True;
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::o3tl::compose1(::boost::bind(::boost::mem_fn(&OReportSection::SelectAll),_1,_nObjectType),TReportPairHelper()));
    m_bInUnmark = sal_False;
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
            if ( &(*aIter)->getReportSection().getSectionView() != _pSectionView )
            {
                (*aIter)->getReportSection().deactivateOle();
                (*aIter)->getReportSection().getSectionView().UnmarkAllObj();
            }
        }
        m_bInUnmark = sal_False;
    }
}
// -----------------------------------------------------------------------
void OViewsWindow::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32)
{
    ImplInitSettings();
    Invalidate();
}
// -----------------------------------------------------------------------------
void OViewsWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        GrabFocus();
        const uno::Sequence< beans::PropertyValue> aArgs;
        getView()->getReportView()->getController().executeChecked(SID_SELECT_REPORT,aArgs);
    }
    Window::MouseButtonDown(rMEvt);
}
//----------------------------------------------------------------------------
void OViewsWindow::showRuler(sal_Bool _bShow)
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::o3tl::compose1(::boost::bind(&OStartMarker::showRuler,_1,_bShow),TStartMarkerHelper()));
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::o3tl::compose1(::boost::bind(&OStartMarker::Window::Invalidate, _1, sal_uInt16(INVALIDATE_NOERASE)), TStartMarkerHelper()));
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
            if ( (*aIter)->getReportSection().getSectionView().AreObjectsMarked() )
            {
                (*aIter)->getReportSection().MouseButtonUp(rMEvt);
                break;
            }
        }

        // remove special insert mode
        for (aIter = m_aSections.begin();aIter != aEnd ; ++aIter)
        {
            (*aIter)->getReportSection().getPage()->resetSpecialMode();
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
        if ( (*aIter)->getStartMarker().isMarked() )
        {
            bRet = (*aIter)->getReportSection().handleKeyEvent(_rEvent);
        }
    }
    return bRet;
}
//----------------------------------------------------------------------------
OViewsWindow::TSectionsMap::iterator OViewsWindow::getIteratorAtPos(sal_uInt16 _nPos)
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
        setMarked(_pSectionView->getReportSection()->getSection(),_bMark);
}
//------------------------------------------------------------------------
void OViewsWindow::setMarked(const uno::Reference< report::XSection>& _xSection,sal_Bool _bMark)
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd ; ++aIter)
    {
        if ( (*aIter)->getReportSection().getSection() != _xSection )
        {
            (*aIter)->setMarked(sal_False);
        }
        else if ( (*aIter)->getStartMarker().isMarked() != _bMark )
        {
            (*aIter)->setMarked(_bMark);
        }
    }
}
//------------------------------------------------------------------------
void OViewsWindow::setMarked(const uno::Sequence< uno::Reference< report::XReportComponent> >& _aShapes,sal_Bool _bMark)
{
    bool bFirst = true;
    const uno::Reference< report::XReportComponent>* pIter = _aShapes.getConstArray();
    const uno::Reference< report::XReportComponent>* pEnd  = pIter + _aShapes.getLength();
    for(;pIter != pEnd;++pIter)
    {
        const uno::Reference< report::XSection> xSection = (*pIter)->getSection();
        if ( xSection.is() )
        {
            if ( bFirst )
            {
                bFirst = false;
                m_pParent->setMarked(xSection,_bMark);
            }
            ::boost::shared_ptr<OSectionWindow> pSectionWindow = getSectionWindow(xSection);
            if ( pSectionWindow )
            {
                SvxShape* pShape = SvxShape::getImplementation( *pIter );
                SdrObject* pObject = pShape ? pShape->GetSdrObject() : NULL;
                OSL_ENSURE( pObject, "OViewsWindow::setMarked: no SdrObject for the shape!" );
                if ( pObject )
                    pSectionWindow->getReportSection().getSectionView().MarkObj( pObject, pSectionWindow->getReportSection().getSectionView().GetSdrPageView(), !_bMark );
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::collectRectangles(TRectangleMap& _rSortRectangles,  bool _bBoundRects)
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (aIter = m_aSections.begin();aIter != aEnd ; ++aIter)
    {
        OSectionView& rView = (*aIter)->getReportSection().getSectionView();
        if ( rView.AreObjectsMarked() )
        {
            rView.SortMarkedObjects();
            const sal_uInt32 nCount = rView.GetMarkedObjectCount();
            for (sal_uInt32 i=0; i < nCount; ++i)
            {
                const SdrMark* pM = rView.GetSdrMarkByIndex(i);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                Rectangle aObjRect(_bBoundRects ? pObj->GetCurrentBoundRect() : pObj->GetSnapRect());
                _rSortRectangles.insert(TRectangleMap::value_type(aObjRect,TRectangleMap::mapped_type(pObj,&rView)));
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
        sal_Bool bHasFixed = !aInfo.bMoveAllowed || pObj->IsMoveProtect();
        if ( bHasFixed )
            _rBound.Union(aObjRect);
        else
        {
            if ( _bAlignAtSection || _rSortRectangles.size() == 1 )
            { // einzelnes Obj an der Seite ausrichten
                if ( ! bOnlyOnce )
                {
                    bOnlyOnce = true;
                    OReportSection* pReportSection = aRectIter->second.second->getReportSection();
                    const uno::Reference< report::XSection> xSection = pReportSection->getSection();
                    try
                    {
                        uno::Reference<report::XReportDefinition> xReportDefinition = xSection->getReportDefinition();
                        _rBound.Union(Rectangle(getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN),0,
                                            getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width  - getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN),
                                            xSection->getHeight()));
                    }
                    catch(const uno::Exception &){}
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
                uno::Reference<report::XSection> xSection = (*m_aSections.begin())->getReportSection().getSection();
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
                    if ( pView == aInterSectRectIter->second.second && (dynamic_cast<OUnoObject*>(aInterSectRectIter->second.first) || dynamic_cast<OOle2Obj*>(aInterSectRectIter->second.first)))
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
                                else if ( _nControlModification == ControlModification::HEIGHT_GREATEST || _nControlModification == ControlModification::HEIGHT_SMALLEST )
                                    pObjBase->getReportComponent()->setSize(awt::Size(aObjRect.getWidth(),nYMov));
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
    ::boost::shared_ptr<OSectionWindow> pMarkedSection = getMarkedSection();
    if ( pMarkedSection )
        pMarkedSection->getReportSection().createDefault(m_sShapeType);
}
// -----------------------------------------------------------------------------
void OViewsWindow::setGridSnap(sal_Bool bOn)
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd ; ++aIter)
    {
        (*aIter)->getReportSection().getSectionView().SetGridSnap(bOn);
        static sal_Int32 nIn = 0;
        (*aIter)->getReportSection().Invalidate(nIn);
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::setDragStripes(sal_Bool bOn)
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd ; ++aIter)
        (*aIter)->getReportSection().getSectionView().SetDragStripes(bOn);
}
// -----------------------------------------------------------------------------
sal_uInt16 OViewsWindow::getPosition(const OSectionWindow* _pSectionWindow) const
{
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    sal_uInt16 nPosition = 0;
    for (; aIter != aEnd ; ++aIter)
    {
        if ( _pSectionWindow == (*aIter).get() )
        {
            break;
        }
        ++nPosition;
    }
    return nPosition;
}
// -----------------------------------------------------------------------------
::boost::shared_ptr<OSectionWindow> OViewsWindow::getSectionWindow(const sal_uInt16 _nPos) const
{
    ::boost::shared_ptr<OSectionWindow> aReturn;

    if ( _nPos < m_aSections.size() )
        aReturn = m_aSections[_nPos];

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
            OSectionView& rView( _rhs->getReportSection().getSectionView() );
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
void OViewsWindow::BegDragObj_createInvisibleObjectAtPosition(const Rectangle& _aRect, const OSectionView& _rSection)
{
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    Point aNewPos(0,0);

    for (; aIter != aEnd; ++aIter)
    {
        OReportSection& rReportSection = (*aIter)->getReportSection();
        rReportSection.getPage()->setSpecialMode();
        OSectionView& rView = rReportSection.getSectionView();

        if ( &rView != &_rSection )
        {
            SdrObject *pNewObj = new SdrUnoObj(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FixedText")));
            if (pNewObj)
            {
                pNewObj->SetLogicRect(_aRect);

                pNewObj->Move(Size(0, aNewPos.Y()));
                sal_Bool bChanged = rView.GetModel()->IsChanged();
                rReportSection.getPage()->InsertObject(pNewObj);
                rView.GetModel()->SetChanged(bChanged);
                m_aBegDragTempList.push_back(pNewObj);

                rView.MarkObj( pNewObj, rView.GetSdrPageView() );
            }
        }
        const long nSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
        aNewPos.Y() -= nSectionHeight;
    }
}
// -----------------------------------------------------------------------------
bool OViewsWindow::isObjectInMyTempList(SdrObject *_pObj)
{
    return ::std::find(m_aBegDragTempList.begin(),m_aBegDragTempList.end(),_pObj) != m_aBegDragTempList.end();
}

// -----------------------------------------------------------------------------
void OViewsWindow::BegDragObj(const Point& _aPnt, SdrHdl* _pHdl,const OSectionView* _pSection)
{
    OSL_TRACE("BegDragObj Clickpoint X:%d Y:%d", _aPnt.X(), _aPnt.Y() );

    m_aBegDragTempList.clear();

    // Calculate the absolute clickpoint in the views
    Point aAbsolutePnt = _aPnt;
    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (; aIter != aEnd; ++aIter)
    {
        OReportSection& rReportSection = (*aIter)->getReportSection();
        OSectionView* pView = &rReportSection.getSectionView();
        if (pView == _pSection)
            break;
        const long nSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
        aAbsolutePnt.Y() +=  nSectionHeight;
    }
    m_aDragDelta = Point(SAL_MAX_INT32, SAL_MAX_INT32);
    OSL_TRACE("BegDragObj Absolute X:%d Y:%d", aAbsolutePnt.X(), aAbsolutePnt.Y() );

    // Create drag lines over all viewable Views
    // Therefore we need to identify the marked objects
    // and create temporary objects on all other views at the same position
    // relative to its occurrence.

    OSL_TRACE("BegDragObj createInvisible Objects" );
    int nViewCount = 0;
    Point aNewObjPos(0,0);
    Point aLeftTop = Point(SAL_MAX_INT32, SAL_MAX_INT32);
    for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    {
        OReportSection& rReportSection = (*aIter)->getReportSection();

        OSectionView& rView = rReportSection.getSectionView();

        if ( rView.AreObjectsMarked() )
        {
            const sal_uInt32 nCount = rView.GetMarkedObjectCount();
            for (sal_uInt32 i=0; i < nCount; ++i)
            {
                const SdrMark* pM = rView.GetSdrMarkByIndex(i);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                if (!isObjectInMyTempList(pObj))
                {
                    Rectangle aRect( pObj->GetCurrentBoundRect() );
                    aRect.Move(0, aNewObjPos.Y());

                    aLeftTop.X() = ::std::min( aRect.Left(), aLeftTop.X() );
                    aLeftTop.Y() = ::std::min( aRect.Top(), aLeftTop.Y() );

                    OSL_TRACE("BegDragObj createInvisible X:%d Y:%d on View #%d", aRect.Left(), aRect.Top(), nViewCount );

                    BegDragObj_createInvisibleObjectAtPosition(aRect, rView);
                }
            }
        }
        ++nViewCount;
        Rectangle aClipRect = rView.GetWorkArea();
        aClipRect.Top() = -aNewObjPos.Y();
        rView.SetWorkArea( aClipRect );

        const long nSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
        aNewObjPos.Y() += nSectionHeight;
    }

    const sal_Int32 nDeltaX = abs(aLeftTop.X() - aAbsolutePnt.X());
    const sal_Int32 nDeltaY = abs(aLeftTop.Y() - aAbsolutePnt.Y());
    m_aDragDelta.X() = nDeltaX;
    m_aDragDelta.Y() = nDeltaY;

    Point aNewPos = aAbsolutePnt;

    const short nDrgLog = static_cast<short>(PixelToLogic(Size(3,0)).Width());
    nViewCount = 0;
    for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    {
        OReportSection& rReportSection = (*aIter)->getReportSection();

        SdrHdl* pHdl = _pHdl;
        if ( pHdl )
        {
            if ( &rReportSection.getSectionView() != _pSection )
            {
                const SdrHdlList& rHdlList = rReportSection.getSectionView().GetHdlList();
                pHdl = rHdlList.GetHdl(_pHdl->GetKind());
            }
        }
        OSL_TRACE("BegDragObj X:%d Y:%d on View#%d", aNewPos.X(), aNewPos.Y(), nViewCount++ );
        rReportSection.getSectionView().BegDragObj(aNewPos, (OutputDevice*)NULL, pHdl, nDrgLog, NULL);

        const long nSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
        aNewPos.Y() -= nSectionHeight;
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
        OReportSection& rReportSection = (*aIter)->getReportSection();
        if ( &rReportSection.getSectionView() == _pSection )
        {
            bAdd = false;
            aNewPos = _aPnt; // 2,2
        }
        else if ( bAdd )
        {
            const long nSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
            aNewPos.Y() += nSectionHeight;
        }
        else
        {
            aNewPos.Y() -= nLastSectionHeight;
        }
        rReportSection.getSectionView().BegMarkObj ( aNewPos );
        nLastSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
    }
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
        OReportSection& rReportSection = (*aIter)->getReportSection();
        if ( &rReportSection.getSectionView() == _pSection)
            break;
    }
    OSL_ENSURE(aIter != aEnd,"This can never happen!");
    if ( _rPnt.Y() < 0 )
    {
        if ( nCount )
            --aIter;
        for (; nCount && (_rPnt.Y() < 0); --nCount)
        {
            OReportSection& rReportSection = (*aIter)->getReportSection();
            const sal_Int32 nHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
            _rPnt.Y() += nHeight;
            if ( (nCount -1) > 0 && (_rPnt.Y() < 0) )
                --aIter;
        }
        if ( nCount == 0 )
            pSection = &(*m_aSections.begin())->getReportSection().getSectionView();
        else
            pSection = &(*aIter)->getReportSection().getSectionView();
    }
    else
    {
        for (; aIter != aEnd; ++aIter)
        {
            OReportSection& rReportSection = (*aIter)->getReportSection();
            const long nHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
            if ( (_rPnt.Y() - nHeight) < 0  )
                break;
            _rPnt.Y() -= nHeight;
        }
        if ( aIter != aEnd )
            pSection = &(*aIter)->getReportSection().getSectionView();
        else
            pSection = &(*(aEnd-1))->getReportSection().getSectionView();
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
        OReportSection& rReportSection = (*aIter)->getReportSection();
        rReportSection.getPage()->resetSpecialMode();
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::EndDragObj(sal_Bool _bControlKeyPressed, const OSectionView* _pSection,const Point& _aPnt)
{
    const String sUndoAction = String((ModuleRes(RID_STR_UNDO_CHANGEPOSITION)));
    const UndoContext aUndoContext( getView()->getReportView()->getController().getUndoManager(), sUndoAction );

    Point aNewPos = _aPnt;
    OSectionView* pInSection = getSectionRelativeToPosition(_pSection, aNewPos);
    if (!_bControlKeyPressed &&
        (_pSection && ( _pSection->IsDragResize() == false ) ) && /* Not in resize mode */
        _pSection != pInSection)
    {
        EndDragObj_removeInvisibleObjects();

        // we need to manipulate the current clickpoint, we substract the old delta from BeginDrag
        aNewPos -= m_aDragDelta;

        uno::Sequence< beans::NamedValue > aAllreadyCopiedObjects;
        TSectionsMap::iterator aIter = m_aSections.begin();
        const TSectionsMap::iterator aEnd = m_aSections.end();
        for (; aIter != aEnd; ++aIter)
        {
            OReportSection& rReportSection = (*aIter)->getReportSection();
            if ( pInSection != &rReportSection.getSectionView() )
            {
                rReportSection.getSectionView().BrkAction();
                rReportSection.Copy(aAllreadyCopiedObjects,true);
            }
            else
                pInSection->EndDragObj(sal_False);
        }

        if ( aAllreadyCopiedObjects.getLength() )
        {
            beans::NamedValue* pIter = aAllreadyCopiedObjects.getArray();
            const beans::NamedValue* pEnd = pIter + aAllreadyCopiedObjects.getLength();
            try
            {
                uno::Reference<report::XReportDefinition> xReportDefinition = getView()->getReportView()->getController().getReportDefinition();
                const sal_Int32 nLeftMargin  = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
                const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
                const sal_Int32 nPaperWidth  = getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width;

                if ( aNewPos.X() < nLeftMargin )
                    aNewPos.X() = nLeftMargin;
                if ( aNewPos.Y() < 0 )
                    aNewPos.Y() = 0;

                Point aPrevious;
                for (; pIter != pEnd; ++pIter)
                {
                    uno::Sequence< uno::Reference<report::XReportComponent> > aClones;
                    pIter->Value >>= aClones;
                    uno::Reference<report::XReportComponent>* pColIter = aClones.getArray();
                    const uno::Reference<report::XReportComponent>* pColEnd = pColIter + aClones.getLength();

                    // move the cloned Components to new positions
                    for (; pColIter != pColEnd; ++pColIter)
                    {
                        uno::Reference< report::XReportComponent> xRC(*pColIter);
                        aPrevious = VCLPoint(xRC->getPosition());
                        awt::Size aSize = xRC->getSize();

                        if ( aNewPos.X() < nLeftMargin )
                        {
                            aNewPos.X() = nLeftMargin;
                        }
                        else if ( (aNewPos.X() + aSize.Width) > (nPaperWidth - nRightMargin) )
                        {
                            aNewPos.X() = nPaperWidth - nRightMargin - aSize.Width;
                        }
                        if ( aNewPos.Y() < 0 )
                        {
                            aNewPos.Y() = 0;
                        }
                        if ( aNewPos.X() < 0 )
                        {
                            aSize.Width += aNewPos.X();
                            aNewPos.X()= 0;
                            xRC->setSize(aSize);
                        }
                        xRC->setPosition(AWTPoint(aNewPos));
                        if ( (pColIter+1) != pColEnd )
                        {
                            // bring aNewPos to the position of the next object
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
            pInSection->getReportSection()->Paste(aAllreadyCopiedObjects,true);
        }
    }
    else
    {
        ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction( sal_False ) );
        EndDragObj_removeInvisibleObjects();
    }
    m_aDragDelta = Point(SAL_MAX_INT32, SAL_MAX_INT32);
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
    OSL_TRACE("MovAction X:%d Y:%d", aRealMousePos.X(), aRealMousePos.Y() );

    Point aHdlPos;
    SdrHdl* pHdl = _pSection->GetDragHdl();
    if ( pHdl )
    {
        aHdlPos = pHdl->GetPos();
    }

    TSectionsMap::iterator aIter;
    TSectionsMap::iterator aEnd = m_aSections.end();

    for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    {
        OReportSection& rReportSection = (*aIter)->getReportSection();
        if ( &rReportSection.getSectionView() == _pSection )
            break;
        const long nSectionHeight = (*aIter)->PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
        aCurrentSectionPos.Y() += nSectionHeight;
    }
    aRealMousePos += aCurrentSectionPos;

    // If control key is pressed the work area is limited to the section with the current selection.
    Point aPosForWorkArea(0,0);
    for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    {
        OReportSection& rReportSection = (*aIter)->getReportSection();
        OSectionView& rView = rReportSection.getSectionView();
        const long nSectionHeight = (*aIter)->PixelToLogic((*aIter)->GetOutputSizePixel()).Height();

        if (_bControlKeySet)
        {
            Rectangle aClipRect = rView.GetWorkArea();
            aClipRect.Top() = aCurrentSectionPos.Y() - aPosForWorkArea.Y();
            aClipRect.Bottom() = aClipRect.Top() + nSectionHeight;
            rView.SetWorkArea( aClipRect );
        }
        else
        {
            Rectangle aClipRect = rView.GetWorkArea();
            aClipRect.Top() = -aPosForWorkArea.Y();
            rView.SetWorkArea( aClipRect );
        }
        aPosForWorkArea.Y() += nSectionHeight;
    }


    for (aIter = m_aSections.begin(); aIter != aEnd; ++aIter)
    {
        OReportSection& rReportSection = (*aIter)->getReportSection();
        SdrHdl* pCurrentHdl = rReportSection.getSectionView().GetDragHdl();
        if ( pCurrentHdl )
        {
            if ( aRealMousePos.Y() > 0 )
                aRealMousePos = _aPnt + pCurrentHdl->GetPos() - aHdlPos;
        }
        rReportSection.getSectionView().MovAction ( aRealMousePos );
        const long nSectionHeight = (*aIter)->PixelToLogic((*aIter)->GetOutputSizePixel()).Height();
        aRealMousePos.Y() -= nSectionHeight;
    }
}
// -----------------------------------------------------------------------------
sal_Bool OViewsWindow::IsAction() const
{
    sal_Bool bAction = sal_False;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (; !bAction && aIter != aEnd; ++aIter)
        bAction = (*aIter)->getReportSection().getSectionView().IsAction();
    return bAction;
}
// -----------------------------------------------------------------------------
sal_Bool OViewsWindow::IsDragObj() const
{
    sal_Bool bAction = sal_False;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (; !bAction && aIter != aEnd; ++aIter)
        bAction = (*aIter)->getReportSection().getSectionView().IsAction();
    return bAction;
}
// -----------------------------------------------------------------------------
sal_uInt32 OViewsWindow::getMarkedObjectCount() const
{
    sal_uInt32 nCount = 0;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (; aIter != aEnd; ++aIter)
        nCount += (*aIter)->getReportSection().getSectionView().GetMarkedObjectCount();
    return nCount;
}
// -----------------------------------------------------------------------------
void OViewsWindow::handleKey(const KeyCode& _rCode)
{
    const sal_uInt16 nCode = _rCode.GetCode();
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
        OReportSection& rReportSection = (*aIter)->getReportSection();
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

        if ( rReportSection.getSectionView().AreObjectsMarked() )
        {
            if ( _rCode.IsMod2() )
            {
                // move in 1 pixel distance
                const Size aPixelSize = rReportSection.PixelToLogic( Size( 1, 1 ) );
                nX *= aPixelSize.Width();
                nY *= aPixelSize.Height();
            }
            else
            {
                // move in 1 mm distance
                nX *= DEFAUL_MOVE_SIZE;
                nY *= DEFAUL_MOVE_SIZE;
            }

            OSectionView& rView = rReportSection.getSectionView();
            const SdrHdlList& rHdlList = rView.GetHdlList();
            SdrHdl* pHdl = rHdlList.GetFocusHdl();

            if ( pHdl == 0 )
            {
                // no handle selected
                if ( rView.IsMoveAllowed() )
                {
                    // restrict movement to work area
                    Rectangle rWorkArea = rView.GetWorkArea();
                    rWorkArea.Right()++;

                    if ( !rWorkArea.IsEmpty() )
                    {
                        if ( rWorkArea.Top() < 0 )
                            rWorkArea.Top() = 0;
                        Rectangle aMarkRect( rView.GetMarkedObjRect() );
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
                        const SdrMarkList& rMarkList = rView.GetMarkedObjectList();
                        for (sal_uInt32 i =  0; !bCheck && i < rMarkList.GetMarkCount();++i )
                        {
                            SdrMark* pMark = rMarkList.GetMark(i);
                            bCheck = dynamic_cast<OUnoObject*>(pMark->GetMarkedSdrObj()) != NULL|| dynamic_cast<OOle2Obj*>(pMark->GetMarkedSdrObj());
                        }


                        if ( bCheck )
                        {
                            SdrObject* pOverlapped = isOver(aMarkRect,*rReportSection.getPage(),rView);
                            if ( pOverlapped )
                            {
                                do
                                {
                                    Rectangle aOver = pOverlapped->GetLastBoundRect();
                                    Point aPos;
                                    if ( nCode == KEY_UP )
                                    {
                                        aPos.X() = aMarkRect.Left();
                                        aPos.Y() = aOver.Top() - aMarkRect.getHeight();
                                        nY += (aPos.Y() - aMarkRect.Top());
                                    }
                                    else if ( nCode == KEY_DOWN )
                                    {
                                        aPos.X() = aMarkRect.Left();
                                        aPos.Y() = aOver.Bottom();
                                        nY += (aPos.Y() - aMarkRect.Top());
                                    }
                                    else if ( nCode == KEY_LEFT )
                                    {
                                        aPos.X() = aOver.Left() - aMarkRect.getWidth();
                                        aPos.Y() = aMarkRect.Top();
                                        nX += (aPos.X() - aMarkRect.Left());
                                    }
                                    else if ( nCode == KEY_RIGHT )
                                    {
                                        aPos.X() = aOver.Right();
                                        aPos.Y() = aMarkRect.Top();
                                        nX += (aPos.X() - aMarkRect.Left());
                                    }

                                    aMarkRect.SetPos(aPos);
                                    if ( !rWorkArea.IsInside( aMarkRect ) )
                                    {
                                        break;
                                    }
                                    pOverlapped = isOver(aMarkRect,*rReportSection.getPage(),rView);
                                }
                                while(pOverlapped != NULL);
                                if (pOverlapped != NULL)
                                    break;
                            }
                        }
                    }

                    if ( nX != 0 || nY != 0 )
                    {
                        rView.MoveAllMarked( Size( nX, nY ) );
                        rView.MakeVisible( rView.GetAllMarkedRect(), rReportSection);
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
                    const SdrDragStat& rDragStat = rView.GetDragStat();

                    // start dragging
                    rView.BegDragObj( aStartPoint, 0, pHdl, 0 );

                    if ( rView.IsDragObj() )
                    {
                        const bool bWasNoSnap = rDragStat.IsNoSnap();
                        const sal_Bool bWasSnapEnabled = rView.IsSnapEnabled();

                        // switch snapping off
                        if ( !bWasNoSnap )
                            ((SdrDragStat&)rDragStat).SetNoSnap( sal_True );
                        if ( bWasSnapEnabled )
                            rView.SetSnapEnabled( sal_False );

                        Rectangle aNewRect;
                        bool bCheck = false;
                        const SdrMarkList& rMarkList = rView.GetMarkedObjectList();
                        for (sal_uInt32 i =  0; !bCheck && i < rMarkList.GetMarkCount();++i )
                        {
                            SdrMark* pMark = rMarkList.GetMark(i);
                            bCheck = dynamic_cast<OUnoObject*>(pMark->GetMarkedSdrObj()) != NULL || dynamic_cast<OOle2Obj*>(pMark->GetMarkedSdrObj()) != NULL;
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
                        if ( !(bCheck && isOver(aNewRect,*rReportSection.getPage(),rView)) )
                            rView.MovAction(aEndPoint);
                        rView.EndDragObj();

                        // restore snap
                        if ( !bWasNoSnap )
                            ((SdrDragStat&)rDragStat).SetNoSnap( bWasNoSnap );
                        if ( bWasSnapEnabled )
                            rView.SetSnapEnabled( bWasSnapEnabled );
                    }

                    // make moved handle visible
                    const Rectangle aVisRect( aEndPoint - Point( DEFAUL_MOVE_SIZE, DEFAUL_MOVE_SIZE ), Size( 200, 200 ) );
                    rView.MakeVisible( aVisRect, rReportSection);
                }
            }
            rView.AdjustMarkHdl();
        }
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::stopScrollTimer()
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        ::o3tl::compose1(::boost::mem_fn(&OReportSection::stopScrollTimer),TReportPairHelper()));
}
// -----------------------------------------------------------------------------
void OViewsWindow::fillCollapsedSections(::std::vector<sal_uInt16>& _rCollapsedPositions) const
{
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for (sal_uInt16 i = 0;aIter != aEnd ; ++aIter,++i)
    {
        if ( (*aIter)->getStartMarker().isCollapsed() )
            _rCollapsedPositions.push_back(i);
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::collapseSections(const uno::Sequence< beans::PropertyValue>& _aCollpasedSections)
{
    const beans::PropertyValue* pIter = _aCollpasedSections.getConstArray();
    const beans::PropertyValue* pEnd = pIter + _aCollpasedSections.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        sal_uInt16 nPos = sal_uInt16(-1);
        if ( (pIter->Value >>= nPos) && nPos < m_aSections.size() )
        {
            m_aSections[nPos]->setCollapsed(sal_True);
        }
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::zoom(const Fraction& _aZoom)
{
    const MapMode& aMapMode = GetMapMode();

    Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH));
    if ( _aZoom < aMapMode.GetScaleX() )
        aStartWidth *= aMapMode.GetScaleX();
    else
        aStartWidth *= _aZoom;

    setZoomFactor(_aZoom,*this);

    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (;aIter != aEnd ; ++aIter)
    {
        (*aIter)->zoom(_aZoom);
    }

    Resize();

    Size aOut = GetOutputSizePixel();
    aOut.Width() = aStartWidth;
    aOut = PixelToLogic(aOut);

    Rectangle aRect(PixelToLogic(Point(0,0)),aOut);
    static sal_Int32 nIn = INVALIDATE_NOCHILDREN;
    Invalidate(aRect,nIn);
}
//----------------------------------------------------------------------------
void OViewsWindow::scrollChildren(const Point& _aThumbPos)
{
    const Point aPos(PixelToLogic(_aThumbPos));
    {
        MapMode aMapMode = GetMapMode();
        const Point aOld = aMapMode.GetOrigin();
        aMapMode.SetOrigin(m_pParent->GetMapMode().GetOrigin());

        const Point aPosY(m_pParent->PixelToLogic(_aThumbPos,aMapMode));

        aMapMode.SetOrigin( Point(aOld.X() , - aPosY.Y()));
        SetMapMode( aMapMode );
        Scroll(0, -( aOld.Y() + aPosY.Y()),SCROLL_CHILDREN);
    }

    TSectionsMap::iterator aIter = m_aSections.begin();
    TSectionsMap::iterator aEnd = m_aSections.end();
    for (;aIter != aEnd ; ++aIter)
    {
        (*aIter)->scrollChildren(aPos.X());
    }
}
// -----------------------------------------------------------------------------
void OViewsWindow::fillControlModelSelection(::std::vector< uno::Reference< uno::XInterface > >& _rSelection) const
{
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    for(;aIter != aEnd; ++aIter)
    {
        (*aIter)->getReportSection().fillControlModelSelection(_rSelection);
    }
}
//==============================================================================
} // rptui
//==============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
