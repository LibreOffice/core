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

#include <JoinTableView.hxx>
#include <osl/diagnose.h>
#include <querycontroller.hxx>
#include <JoinDesignView.hxx>
#include <TableWindow.hxx>
#include <TableWindowListBox.hxx>
#include <TableConnection.hxx>
#include <TableConnectionData.hxx>
#include <ConnectionLine.hxx>
#include <ConnectionLineData.hxx>
#include <browserids.hxx>
#include <svl/urlbmk.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "QueryMoveTabWinUndoAct.hxx"
#include "QuerySizeTabWinUndoAct.hxx"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>
#include <TableWindowData.hxx>
#include <JAccess.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <UITools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <tools/diagnose_ex.h>
#include <algorithm>
#include <functional>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

#define LINE_SIZE           50
// Constants for the window layout
#define TABWIN_SPACING_X    17
#define TABWIN_SPACING_Y    17

#define TABWIN_WIDTH_STD    120
#define TABWIN_HEIGHT_STD   120

OScrollWindowHelper::OScrollWindowHelper( vcl::Window* pParent) : Window( pParent)
    ,m_aHScrollBar( VclPtr<ScrollBar>::Create(this, WB_HSCROLL|WB_REPEAT|WB_DRAG) )
    ,m_aVScrollBar( VclPtr<ScrollBar>::Create(this, WB_VSCROLL|WB_REPEAT|WB_DRAG) )
    ,m_pCornerWindow(VclPtr<ScrollBarBox>::Create(this, WB_3DLOOK))
    ,m_pTableView(nullptr)
{

    // ScrollBars

    GetHScrollBar().SetRange( Range(0, 1000) );
    GetVScrollBar().SetRange( Range(0, 1000) );

    GetHScrollBar().SetLineSize( LINE_SIZE );
    GetVScrollBar().SetLineSize( LINE_SIZE );

    GetHScrollBar().Show();
    GetVScrollBar().Show();
    m_pCornerWindow->Show();

    // normally we should be SCROLL_PANE
    SetAccessibleRole(AccessibleRole::SCROLL_PANE);
}

OScrollWindowHelper::~OScrollWindowHelper()
{
    disposeOnce();
}

void OScrollWindowHelper::dispose()
{
    m_aHScrollBar.disposeAndClear();
    m_aVScrollBar.disposeAndClear();
    m_pCornerWindow.disposeAndClear();
    m_pTableView.clear();
    vcl::Window::dispose();
}

void OScrollWindowHelper::setTableView(OJoinTableView* _pTableView)
{
    m_pTableView = _pTableView;
    // ScrollBars
    GetHScrollBar().SetScrollHdl( LINK(m_pTableView, OJoinTableView, ScrollHdl) );
    GetVScrollBar().SetScrollHdl( LINK(m_pTableView, OJoinTableView, ScrollHdl) );
}

void OScrollWindowHelper::resetRange(const Point& _aSize)
{
    Point aPos = PixelToLogic(_aSize);
    GetHScrollBar().SetRange( Range(0, aPos.X() + TABWIN_SPACING_X) );
    GetVScrollBar().SetRange( Range(0, aPos.Y() + TABWIN_SPACING_Y) );
}

void OScrollWindowHelper::Resize()
{
    Window::Resize();

    Size aTotalOutputSize = GetOutputSizePixel();
    long nHScrollHeight = GetHScrollBar().GetSizePixel().Height();
    long nVScrollWidth = GetVScrollBar().GetSizePixel().Width();

    GetHScrollBar().SetPosSizePixel(
        Point( 0, aTotalOutputSize.Height()-nHScrollHeight ),
        Size( aTotalOutputSize.Width()-nVScrollWidth, nHScrollHeight )
        );

    GetVScrollBar().SetPosSizePixel(
        Point( aTotalOutputSize.Width()-nVScrollWidth, 0 ),
        Size( nVScrollWidth, aTotalOutputSize.Height()-nHScrollHeight )
        );

    m_pCornerWindow->SetPosSizePixel(
        Point( aTotalOutputSize.Width() - nVScrollWidth, aTotalOutputSize.Height() - nHScrollHeight),
        Size( nVScrollWidth, nHScrollHeight )
        );

    GetHScrollBar().SetPageSize( aTotalOutputSize.Width() );
    GetHScrollBar().SetVisibleSize( aTotalOutputSize.Width() );

    GetVScrollBar().SetPageSize( aTotalOutputSize.Height() );
    GetVScrollBar().SetVisibleSize( aTotalOutputSize.Height() );

    // adjust the ranges of the scrollbars if necessary
    long lRange = GetHScrollBar().GetRange().Max() - GetHScrollBar().GetRange().Min();
    if (m_pTableView->GetScrollOffset().X() + aTotalOutputSize.Width() > lRange)
        GetHScrollBar().SetRangeMax(m_pTableView->GetScrollOffset().X() + aTotalOutputSize.Width() + GetHScrollBar().GetRange().Min());

    lRange = GetVScrollBar().GetRange().Max() - GetVScrollBar().GetRange().Min();
    if (m_pTableView->GetScrollOffset().Y() + aTotalOutputSize.Height() > lRange)
        GetVScrollBar().SetRangeMax(m_pTableView->GetScrollOffset().Y() + aTotalOutputSize.Height() + GetVScrollBar().GetRange().Min());

    m_pTableView->SetPosSizePixel(Point( 0, 0 ),Size( aTotalOutputSize.Width()-nVScrollWidth, aTotalOutputSize.Height()-nHScrollHeight ));
}

// class OJoinTableView

OJoinTableView::OJoinTableView( vcl::Window* pParent, OJoinDesignView* pView )
    :Window( pParent,WB_BORDER )
    ,DropTargetHelper(this)
    ,m_aDragOffset( Point(0,0) )
    ,m_aScrollOffset( Point(0,0) )
    ,m_pDragWin( nullptr )
    ,m_pSizingWin( nullptr )
    ,m_pSelectedConn( nullptr )
    ,m_pLastFocusTabWin(nullptr)
    ,m_pView( pView )
    ,m_pAccessible(nullptr)
{
    SetSizePixel( Size(1000, 1000) );

    InitColors();

    m_aDragScrollIdle.SetInvokeHandler(LINK(this, OJoinTableView, OnDragScrollTimer));
}

OJoinTableView::~OJoinTableView()
{
    disposeOnce();
}

void OJoinTableView::dispose()
{
    if( m_pAccessible )
    {
        m_pAccessible->clearTableView();
        m_pAccessible = nullptr;
    }
    // delete lists
    clearLayoutInformation();
    m_pDragWin.clear();
    m_pSizingWin.clear();
    m_pSelectedConn.clear();
    m_pLastFocusTabWin.clear();
    m_pView.clear();
    m_vTableConnection.clear();
    vcl::Window::dispose();
}

IMPL_LINK( OJoinTableView, ScrollHdl, ScrollBar*, pScrollBar, void )
{
    // move all windows
    ScrollPane( pScrollBar->GetDelta(), (pScrollBar == &GetHScrollBar()), false );
}

void OJoinTableView::Resize()
{
    Window::Resize();
    m_aOutputSize = GetSizePixel();

    // tab win positions may not be up-to-date
    if (m_aTableMap.empty())
        // no tab wins ...
        return;

    // we have at least one table so resize it
    m_aScrollOffset.setX( GetHScrollBar().GetThumbPos() );
    m_aScrollOffset.setY( GetVScrollBar().GetThumbPos() );

    VclPtr<OTableWindow> pCheck = m_aTableMap.begin()->second;
    Point aRealPos = pCheck->GetPosPixel();
    Point aAssumedPos = pCheck->GetData()->GetPosition() - GetScrollOffset();

    if (aRealPos == aAssumedPos)
        // all ok
        return;

    for (auto const& elem : m_aTableMap)
    {
        OTableWindow* pCurrent = elem.second;
        Point aPos(pCurrent->GetData()->GetPosition() - GetScrollOffset());
        pCurrent->SetPosPixel(aPos);
    }
}

sal_uLong OJoinTableView::GetTabWinCount()
{
    return m_aTableMap.size();
}

bool OJoinTableView::RemoveConnection(VclPtr<OTableConnection>& rConn, bool _bDelete)
{
    VclPtr<OTableConnection> xConn(rConn);

    DeselectConn(xConn);

    // to force a redraw
    xConn->InvalidateConnection();

    m_pView->getController().removeConnectionData(xConn->GetData());

    m_vTableConnection.erase(std::find(m_vTableConnection.begin(), m_vTableConnection.end(), xConn));

    modified();
    if ( m_pAccessible )
        m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                makeAny(xConn->GetAccessible()),
                                                Any());
    if (_bDelete)
        xConn->disposeOnce();

    return true;
}

OTableWindow* OJoinTableView::GetTabWindow( const OUString& rName )
{
    OTableWindowMap::const_iterator aIter = m_aTableMap.find(rName);

    return aIter == m_aTableMap.end() ? nullptr : aIter->second;
}

TTableWindowData::value_type OJoinTableView::createTableWindowData(const OUString& _rComposedName
                                                                  ,const OUString& _sTableName
                                                                  ,const OUString& _rWinName)
{
    TTableWindowData::value_type pData( CreateImpl(_rComposedName, _sTableName,_rWinName) );
    OJoinDesignView* pParent = getDesignView();
    try
    {
        if ( !pData->init(pParent->getController().getConnection(),allowQueries()) )
        {
            if ( pData->isValid() )
                onNoColumns_throw();
            else
                pData.reset();
        }
    }
    catch ( const SQLException& )
    {
        ::dbtools::showError( ::dbtools::SQLExceptionInfo( ::cppu::getCaughtException() ),
            VCLUnoHelper::GetInterface(pParent), pParent->getController().getORB() );
    }
    catch( const WrappedTargetException& e )
    {
        SQLException aSql;
        if ( e.TargetException >>= aSql )
            ::dbtools::showError( ::dbtools::SQLExceptionInfo( aSql ), VCLUnoHelper::GetInterface(pParent), pParent->getController().getORB() );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return pData;
}

OTableWindowData* OJoinTableView::CreateImpl(const OUString& _rComposedName
                                             ,const OUString& _sTableName
                                             ,const OUString& _rWinName)
{
    return new OTableWindowData( nullptr,_rComposedName,_sTableName, _rWinName );
}

void OJoinTableView::AddTabWin(const OUString& _rComposedName, const OUString& rWinName, bool /*bNewTable*/)
{
    OSL_ENSURE(!_rComposedName.isEmpty(),"There must be a table name supplied!");

    TTableWindowData::value_type pNewTabWinData(createTableWindowData( _rComposedName, rWinName,rWinName ));

    // insert new window in window list
    VclPtr<OTableWindow> pNewTabWin = createWindow( pNewTabWinData );
    if ( pNewTabWin->Init() )
    {
        m_pView->getController().getTableWindowData().push_back( pNewTabWinData);
        // when we already have a table with this name insert the full qualified one instead
        if(m_aTableMap.find(rWinName) != m_aTableMap.end())
            m_aTableMap[_rComposedName] = pNewTabWin;
        else
            m_aTableMap[rWinName] = pNewTabWin;

        SetDefaultTabWinPosSize( pNewTabWin );
        pNewTabWin->Show();

        modified();
        if ( m_pAccessible )
            m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                    Any(),
                                                    makeAny(pNewTabWin->GetAccessible()));
    }
    else
    {
        pNewTabWin->clearListBox();
        pNewTabWin.disposeAndClear();
    }
}

void OJoinTableView::RemoveTabWin( OTableWindow* pTabWin )
{
    // first delete all connections of this window to others
    bool bRemove = true;
    TTableWindowData::value_type pData = pTabWin->GetData();
    sal_Int32 nCount = m_vTableConnection.size();
    auto aIter = m_vTableConnection.rbegin();
    while(aIter != m_vTableConnection.rend() && bRemove)
    {
        VclPtr<OTableConnection>& rTabConn = *aIter;
        if (
             (pData == rTabConn->GetData()->getReferencingTable()) ||
             (pData == rTabConn->GetData()->getReferencedTable())
           )
        {
            bRemove = RemoveConnection(rTabConn, true);
            aIter = m_vTableConnection.rbegin();
        }
        else
            ++aIter;
    }

    // then delete the window itself
    if ( bRemove )
    {
        if ( m_pAccessible )
            m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                    makeAny(pTabWin->GetAccessible()),Any()
                                                    );

        pTabWin->Hide();
        OJoinController& rController = m_pView->getController();
        TTableWindowData::iterator aFind = std::find(rController.getTableWindowData().begin(), rController.getTableWindowData().end(), pData);
        if(aFind != rController.getTableWindowData().end())
        {
            rController.getTableWindowData().erase(aFind);
            rController.setModified(true);
        }

        if ( !m_aTableMap.erase( pTabWin->GetWinName() ) )
            m_aTableMap.erase( pTabWin->GetComposedName() );

        if (pTabWin == m_pLastFocusTabWin)
            m_pLastFocusTabWin = nullptr;

        pTabWin->clearListBox();
        pTabWin->disposeOnce();
    }

    if ( static_cast<sal_Int32>(m_vTableConnection.size()) < (nCount-1) ) // if some connections could be removed
        modified();
}

namespace
{
    bool isScrollAllowed( OJoinTableView* _pView,long nDelta, bool bHoriz)
    {
        // adjust ScrollBar-Positions
        ScrollBar& rBar = bHoriz ? _pView->GetHScrollBar() : _pView->GetVScrollBar() ;

        long nOldThumbPos = rBar.GetThumbPos();
        long nNewThumbPos = nOldThumbPos + nDelta;
        if( nNewThumbPos < 0 )
            nNewThumbPos = 0;
        else if( nNewThumbPos > rBar.GetRangeMax() )
            nNewThumbPos = rBar.GetRangeMax();

        if ( bHoriz )
        {
            if( nNewThumbPos == _pView->GetScrollOffset().X() )
                return false;
        }
        else if ( nNewThumbPos == _pView->GetScrollOffset().Y() )
            return false;

        return true;
    }
    bool getMovementImpl(OJoinTableView* _pView,const Point& _rPoint,const Size& _rSize,long& _nScrollX,long& _nScrollY)
    {
        _nScrollY = _nScrollX = 0;
        // data about the tab win
        Point aUpperLeft = _rPoint;
        // normalize with respect to visibility
        aUpperLeft -= _pView->GetScrollOffset();
        Point aLowerRight(aUpperLeft.X() + _rSize.Width(), aUpperLeft.Y() + _rSize.Height());

        // data about ourself
        Size aSize = _pView->getRealOutputSize(); //GetOutputSizePixel();

        bool bVisible = true;
        bool bFitsHor = (aUpperLeft.X() >= 0) && (aLowerRight.X() <= aSize.Width());
        bool bFitsVert= (aUpperLeft.Y() >= 0) && (aLowerRight.Y() <= aSize.Height());
        if (!bFitsHor || !bFitsVert)
        {
            if (!bFitsHor)
            {
                // ensure the visibility of the right border
                if ( aLowerRight.X() > aSize.Width() )
                    _nScrollX = aLowerRight.X() - aSize.Width() + TABWIN_SPACING_X;

                // ensure the visibility of the left border (higher priority)
                if ( aUpperLeft.X() < 0 )
                    _nScrollX = aUpperLeft.X() - TABWIN_SPACING_X;
            }

            if (!bFitsVert)
            {
                // lower border
                if ( aLowerRight.Y() > aSize.Height() )
                    _nScrollY = aLowerRight.Y() - aSize.Height() + TABWIN_SPACING_Y;
                // upper border
                if ( aUpperLeft.Y() < 0 )
                    _nScrollY = aUpperLeft.Y() - TABWIN_SPACING_Y;
            }

            if ( _nScrollX ) // aSize.Width() > _rSize.Width() &&
                bVisible = isScrollAllowed(_pView,_nScrollX, true);

            if ( _nScrollY ) // aSize.Height() > _rSize.Height() &&
                bVisible = bVisible && isScrollAllowed(_pView,_nScrollY, false);

            if ( bVisible )
            {
                sal_Int32 nHRangeMax = _pView->GetHScrollBar().GetRangeMax();
                sal_Int32 nVRangeMax = _pView->GetVScrollBar().GetRangeMax();

                if ( aSize.Width() + _pView->GetHScrollBar().GetThumbPos() + _nScrollX > nHRangeMax )
                    bVisible = false;
                if ( bVisible && aSize.Height() + _pView->GetVScrollBar().GetThumbPos() + _nScrollY > nVRangeMax )
                    bVisible = false;
            }
        }

        return bVisible;
    }
} // end of ano namespace

bool OJoinTableView::isMovementAllowed(const Point& _rPoint,const Size& _rSize)
{
    long nX,nY;
    return getMovementImpl(this,_rPoint,_rSize,nX,nY);
}

void OJoinTableView::EnsureVisible(const OTableWindow* _pWin)
{
    // data about the tab win
    TTableWindowData::value_type pData = _pWin->GetData();
    EnsureVisible( pData->GetPosition() , pData->GetSize());
    Invalidate(InvalidateFlags::NoChildren);
}

void OJoinTableView::EnsureVisible(const Point& _rPoint,const Size& _rSize)
{
    long nScrollX,nScrollY;

    if ( getMovementImpl(this,_rPoint,_rSize,nScrollX,nScrollY) )
    {
        bool bVisible = true;
        if (nScrollX)
            bVisible = ScrollPane(nScrollX, true, true);

        if (nScrollY && bVisible)
            ScrollPane(nScrollY, false, true);
    }
}

void OJoinTableView::SetDefaultTabWinPosSize( OTableWindow* pTabWin )
{
    // determine position:
    // the window is divided into lines with height TABWIN_SPACING_Y+TABWIN_HEIGHT_STD.
    // Then for each line is checked, if there is space for another window.
    // If there is no space, the next line is checked.
    Size aOutSize = GetSizePixel();
    Point aNewPos( 0,0 );
    sal_uInt16 nRow = 0;
    bool bEnd = false;
    while( !bEnd )
    {
        // Set new position to start of line
        aNewPos.setX( TABWIN_SPACING_X );
        aNewPos.setY( (nRow+1) * TABWIN_SPACING_Y );

        // determine rectangle for the corresponding line
        tools::Rectangle aRowRect( Point(0,0), aOutSize );
        aRowRect.SetTop( nRow * ( TABWIN_SPACING_Y + TABWIN_HEIGHT_STD ) );
        aRowRect.SetBottom( (nRow+1) * ( TABWIN_SPACING_Y + TABWIN_HEIGHT_STD ) );

        // check occupied areas of this line
        for (auto const& elem : m_aTableMap)
        {
            OTableWindow* pOtherTabWin = elem.second;
            tools::Rectangle aOtherTabWinRect( pOtherTabWin->GetPosPixel(), pOtherTabWin->GetSizePixel() );

            if(
                ( (aOtherTabWinRect.Top()>aRowRect.Top()) && (aOtherTabWinRect.Top()<aRowRect.Bottom()) ) ||
                ( (aOtherTabWinRect.Bottom()>aRowRect.Top()) && (aOtherTabWinRect.Bottom()<aRowRect.Bottom()) )
              )
            {
                // TabWin is in the line
                if( aOtherTabWinRect.Right()>aNewPos.X() )
                    aNewPos.setX( aOtherTabWinRect.Right() + TABWIN_SPACING_X );
            }
        }

        // Is there space left in this line?
        if( (aNewPos.X()+TABWIN_WIDTH_STD)<aRowRect.Right() )
        {
            aNewPos.setY( aRowRect.Top() + TABWIN_SPACING_Y );
            bEnd = true;
        }
        else
        {
            if( (aRowRect.Bottom()+aRowRect.GetHeight()) > aOutSize.Height() )
            {
                // insert it in the first row
                sal_Int32 nCount = m_aTableMap.size() % (nRow+1);
                ++nCount;
                aNewPos.setY( nCount * TABWIN_SPACING_Y + (nCount-1)*CalcZoom(TABWIN_HEIGHT_STD) );
                bEnd = true;
            }
            else
                nRow++;

        }
    }

    // determine size
    Size aNewSize( CalcZoom(TABWIN_WIDTH_STD), CalcZoom(TABWIN_HEIGHT_STD) );

    // check if the new position in inside the scrollbars ranges
    Point aBottom(aNewPos);
    aBottom.AdjustX(aNewSize.Width() );
    aBottom.AdjustY(aNewSize.Height() );

    if(!GetHScrollBar().GetRange().IsInside(aBottom.X()))
        GetHScrollBar().SetRange( Range(0, aBottom.X()) );
    if(!GetVScrollBar().GetRange().IsInside(aBottom.Y()))
        GetVScrollBar().SetRange( Range(0, aBottom.Y()) );

    pTabWin->SetPosSizePixel( aNewPos, aNewSize );
}

void OJoinTableView::DataChanged(const DataChangedEvent& rDCEvt)
{
    if (rDCEvt.GetType() == DataChangedEventType::SETTINGS)
    {
        // consider the worst case: the colors changed, so adjust me
        InitColors();
        Invalidate(InvalidateFlags::NoChildren);
        // due to the Invalidate, the connections are redrawn, so that they are also pictured in the new colors
    }
}

void OJoinTableView::InitColors()
{
    // the colors for the illustration should be the system colors
    StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
    SetBackground(Wallpaper(aSystemStyle.GetDialogColor()));
}

void OJoinTableView::BeginChildMove( OTableWindow* pTabWin, const Point& rMousePos  )
{

    if (m_pView->getController().isReadOnly())
        return;

    m_pDragWin = pTabWin;
    SetPointer(PointerStyle::Move);
    Point aMousePos = ScreenToOutputPixel( rMousePos );
    m_aDragOffset = aMousePos - pTabWin->GetPosPixel();
    m_pDragWin->SetZOrder(nullptr, ZOrderFlags::First);
    StartTracking();
}

void OJoinTableView::NotifyTitleClicked( OTableWindow* pTabWin, const Point& rMousePos )
{
    DeselectConn(GetSelectedConn());
    BeginChildMove(pTabWin, rMousePos);
}

void OJoinTableView::BeginChildSizing( OTableWindow* pTabWin, PointerStyle nPointer )
{

    if (m_pView->getController().isReadOnly())
        return;

    SetPointer( nPointer );
    m_pSizingWin = pTabWin;
    StartTracking();
}

bool OJoinTableView::ScrollPane( long nDelta, bool bHoriz, bool bPaintScrollBars )
{
    bool bRet = true;

    // adjust ScrollBar-Positions
    if( bPaintScrollBars )
    {
        if( bHoriz )
        {
            long nOldThumbPos = GetHScrollBar().GetThumbPos();
            long nNewThumbPos = nOldThumbPos + nDelta;
            if( nNewThumbPos < 0 )
            {
                nNewThumbPos = 0;
                bRet = false;
            }
            if( nNewThumbPos > GetHScrollBar().GetRange().Max() )
            {
                nNewThumbPos = GetHScrollBar().GetRange().Max();
                bRet = false;
            }
            GetHScrollBar().SetThumbPos( nNewThumbPos );
            nDelta = GetHScrollBar().GetThumbPos() - nOldThumbPos;
        }
        else
        {
            long nOldThumbPos = GetVScrollBar().GetThumbPos();
            long nNewThumbPos = nOldThumbPos+nDelta;
            if( nNewThumbPos < 0 )
            {
                nNewThumbPos = 0;
                bRet = false;
            }
            if( nNewThumbPos > GetVScrollBar().GetRange().Max() )
            {
                nNewThumbPos = GetVScrollBar().GetRange().Max();
                bRet = false;
            }
            GetVScrollBar().SetThumbPos( nNewThumbPos );
            nDelta = GetVScrollBar().GetThumbPos() - nOldThumbPos;
        }
    }

    // If ScrollOffset hitting borders, no redrawing.
    if( (GetHScrollBar().GetThumbPos()==m_aScrollOffset.X()) &&
        (GetVScrollBar().GetThumbPos()==m_aScrollOffset.Y()) )
        return false;

    // set ScrollOffset anew
    if (bHoriz)
        m_aScrollOffset.setX( GetHScrollBar().GetThumbPos() );
    else
        m_aScrollOffset.setY( GetVScrollBar().GetThumbPos() );

    // move all windows
    OTableWindow* pTabWin;
    Point aPos;

    for (auto const& elem : m_aTableMap)
    {
        pTabWin = elem.second;
        aPos = pTabWin->GetPosPixel();

        if( bHoriz )
            aPos.AdjustX( -nDelta );
        else aPos.AdjustY( -nDelta );

        pTabWin->SetPosPixel( aPos );
    }

    Invalidate(); // InvalidateFlags::NoChildren

    return bRet;
}

void OJoinTableView::Tracking( const TrackingEvent& rTEvt )
{
    HideTracking();

    if (rTEvt.IsTrackingEnded())
    {
        if( m_pDragWin )
        {
            if (m_aDragScrollIdle.IsActive())
                m_aDragScrollIdle.Stop();

            // adjust position of child after moving
            // windows are not allowed to leave display range
            Point aDragWinPos = rTEvt.GetMouseEvent().GetPosPixel() - m_aDragOffset;
            Size aDragWinSize = m_pDragWin->GetSizePixel();
            if( aDragWinPos.X() < 0 )
                aDragWinPos.setX( 0 );
            if( aDragWinPos.Y() < 0 )
                aDragWinPos.setY( 0 );
            if( (aDragWinPos.X() + aDragWinSize.Width()) > m_aOutputSize.Width() )
                aDragWinPos.setX( m_aOutputSize.Width() - aDragWinSize.Width() - 1 );
            if( (aDragWinPos.Y() + aDragWinSize.Height()) > m_aOutputSize.Height() )
                aDragWinPos.setY( m_aOutputSize.Height() - aDragWinSize.Height() - 1 );
            if( aDragWinPos.X() < 0 )
                aDragWinPos.setX( 0 );
            if( aDragWinPos.Y() < 0 )
                aDragWinPos.setY( 0 );
            // TODO : don't position window anew, if it is leaving range, but just expand the range

            // position window
            EndTracking();
            m_pDragWin->SetZOrder(nullptr, ZOrderFlags::First);
            // check, if I really moved
            // (this prevents setting the modified-Flag, when there actually was no change0
            TTableWindowData::value_type pData = m_pDragWin->GetData();
            if ( ! (pData && pData->HasPosition() && (pData->GetPosition() == aDragWinPos)))
            {
                // old logic coordinates
                Point ptOldPos = m_pDragWin->GetPosPixel() + Point(GetHScrollBar().GetThumbPos(), GetVScrollBar().GetThumbPos());
                // new positioning
                m_pDragWin->SetPosPixel(aDragWinPos);
                TabWinMoved(m_pDragWin, ptOldPos);

                m_pDragWin->GrabFocus();
            }
            m_pDragWin = nullptr;
            SetPointer(PointerStyle::Arrow);
        }
        // else we handle the resizing
        else if( m_pSizingWin )
        {
            SetPointer( PointerStyle::Arrow );
            EndTracking();

            // old physical coordinates

            Size szOld = m_pSizingWin->GetSizePixel();
            Point ptOld = m_pSizingWin->GetPosPixel();
            Size aNewSize(CalcZoom(m_aSizingRect.GetSize().Width()),CalcZoom(m_aSizingRect.GetSize().Height()));
            m_pSizingWin->SetPosSizePixel( m_aSizingRect.TopLeft(), aNewSize );
            TabWinSized(m_pSizingWin, ptOld, szOld);

            m_pSizingWin->Invalidate( m_aSizingRect );
            m_pSizingWin = nullptr;
        }
    }
    else if (rTEvt.IsTrackingCanceled())
    {
        if (m_aDragScrollIdle.IsActive())
            m_aDragScrollIdle.Stop();
        EndTracking();
    }
    else
    {
        if( m_pDragWin )
        {
            m_ptPrevDraggingPos = rTEvt.GetMouseEvent().GetPosPixel();
            // scroll at window borders
            ScrollWhileDragging();
        }

        if( m_pSizingWin )
        {
            Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();
            m_aSizingRect = m_pSizingWin->getSizingRect(aMousePos,m_aOutputSize);
            Update();
            ShowTracking( m_aSizingRect, ShowTrackFlags::Small | ShowTrackFlags::TrackWindow );
        }
    }
}

void OJoinTableView::ConnDoubleClicked(VclPtr<OTableConnection>& /*rConnection*/)
{
}

void OJoinTableView::MouseButtonDown( const MouseEvent& rEvt )
{
    GrabFocus();
    Window::MouseButtonDown(rEvt);
}

void OJoinTableView::MouseButtonUp( const MouseEvent& rEvt )
{
    Window::MouseButtonUp(rEvt);
    // Has a connection been selected?
    if( !m_vTableConnection.empty() )
    {
        DeselectConn(GetSelectedConn());

        for (auto & elem : m_vTableConnection)
        {
            if( elem->CheckHit(rEvt.GetPosPixel()) )
            {
                SelectConn(elem);

                // Double-click
                if( rEvt.GetClicks() == 2 )
                    ConnDoubleClicked(elem);

                break;
            }
        }
    }
}

void OJoinTableView::KeyInput( const KeyEvent& rEvt )
{
    sal_uInt16 nCode = rEvt.GetKeyCode().GetCode();
    bool   bShift = rEvt.GetKeyCode().IsShift();
    bool   bCtrl = rEvt.GetKeyCode().IsMod1();

    if( !bCtrl && !bShift && (nCode==KEY_DELETE) )
    {
        if (GetSelectedConn())
            RemoveConnection(GetSelectedConn(), true);
    }
    else
        Window::KeyInput( rEvt );
}

void OJoinTableView::DeselectConn(OTableConnection* pConn)
{
    if (!pConn || !pConn->IsSelected())
        return;

    // deselect the corresponding entries in the ListBox of the table window
    OTableWindow* pWin = pConn->GetSourceWin();
    if (pWin && pWin->GetListBox())
        pWin->GetListBox()->SelectAll(false);

    pWin = pConn->GetDestWin();
    if (pWin && pWin->GetListBox())
        pWin->GetListBox()->SelectAll(false);

    pConn->Deselect();
    m_pSelectedConn = nullptr;
}

void OJoinTableView::SelectConn(OTableConnection* pConn)
{
    DeselectConn(GetSelectedConn());

    pConn->Select();
    m_pSelectedConn = pConn;
    GrabFocus(); // has to be called here because a table window may still be focused

    // select the concerned entries in the windows
    OTableWindow* pConnSource = pConn->GetSourceWin();
    OTableWindow* pConnDest = pConn->GetDestWin();
    if (pConnSource && pConnDest)
    {
        OTableWindowListBox* pSourceBox = pConnSource->GetListBox().get();
        OTableWindowListBox* pDestBox = pConnDest->GetListBox().get();
        if (pSourceBox && pDestBox)
        {
            pSourceBox->SelectAll(false);
            pDestBox->SelectAll(false);

            SvTreeListEntry* pFirstSourceVisible = pSourceBox->GetFirstEntryInView();
            SvTreeListEntry* pFirstDestVisible = pDestBox->GetFirstEntryInView();

            const std::vector<std::unique_ptr<OConnectionLine>>& rLines = pConn->GetConnLineList();
            auto aIter = rLines.rbegin();
            for(;aIter != rLines.rend();++aIter)
            {
                if ((*aIter)->IsValid())
                {
                    SvTreeListEntry* pSourceEntry = pSourceBox->GetEntryFromText((*aIter)->GetData()->GetSourceFieldName());
                    if (pSourceEntry)
                    {
                        pSourceBox->Select(pSourceEntry);
                        pSourceBox->MakeVisible(pSourceEntry);
                    }

                    SvTreeListEntry* pDestEntry = pDestBox->GetEntryFromText((*aIter)->GetData()->GetDestFieldName());
                    if (pDestEntry)
                    {
                        pDestBox->Select(pDestEntry);
                        pDestBox->MakeVisible(pDestEntry);
                    }

                }
            }

            if ((pFirstSourceVisible != pSourceBox->GetFirstEntryInView())
                || (pFirstDestVisible != pDestBox->GetFirstEntryInView()))
                // scrolling was done -> redraw
                Invalidate(InvalidateFlags::NoChildren);
        }
    }
}

void OJoinTableView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    DrawConnections(rRenderContext, rRect);
}

void OJoinTableView::InvalidateConnections()
{
    // draw Joins
    for (auto & conn : m_vTableConnection)
        conn->InvalidateConnection();
}

void OJoinTableView::DrawConnections(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    // draw Joins
    for(const auto& connection : m_vTableConnection)
        connection->Draw(rRenderContext, rRect);
    // finally redraw the selected one above all others
    if (GetSelectedConn())
        GetSelectedConn()->Draw(rRenderContext, rRect);
}

std::vector<VclPtr<OTableConnection> >::const_iterator OJoinTableView::getTableConnections(const OTableWindow* _pFromWin) const
{
    return std::find_if(  m_vTableConnection.begin(),
                            m_vTableConnection.end(),
                            [_pFromWin](OTableConnection * p) { return p->isTableConnection(_pFromWin); });
}

sal_Int32 OJoinTableView::getConnectionCount(const OTableWindow* _pFromWin) const
{
    return std::count_if( m_vTableConnection.begin(),
                            m_vTableConnection.end(),
                            [_pFromWin](OTableConnection * p) { return p->isTableConnection(_pFromWin); });
}

bool OJoinTableView::ExistsAConn(const OTableWindow* pFrom) const
{
    return getTableConnections(pFrom) != m_vTableConnection.end();
}

void OJoinTableView::ClearAll()
{
    SetUpdateMode(false);

    HideTabWins();

    // and the same with the Connections
    for (auto & elem : m_vTableConnection)
    {
        RemoveConnection(elem, true);
    }
    m_vTableConnection.clear();

    m_pLastFocusTabWin  = nullptr;
    m_pSelectedConn     = nullptr;

    // scroll to the upper left
    ScrollPane(-GetScrollOffset().X(), true, true);
    ScrollPane(-GetScrollOffset().Y(), false, true);
    Invalidate();
}

void OJoinTableView::ScrollWhileDragging()
{
    OSL_ENSURE(m_pDragWin != nullptr, "OJoinTableView::ScrollWhileDragging must not be called when a window is being dragged !");

    // kill the timer
    if (m_aDragScrollIdle.IsActive())
        m_aDragScrollIdle.Stop();

    Point aDragWinPos = m_ptPrevDraggingPos - m_aDragOffset;
    Size aDragWinSize = m_pDragWin->GetSizePixel();
    Point aLowerRight(aDragWinPos.X() + aDragWinSize.Width(), aDragWinPos.Y() + aDragWinSize.Height());

    if (aDragWinPos == m_pDragWin->GetPosPixel())
        return;

    // avoid illustration errors (when scrolling with active TrackingRect)
    HideTracking();

    bool bScrolling = false;
    bool bNeedScrollTimer = false;

    // scroll at window borders
    // TODO : only catch, if window would disappear completely (don't, if there is still a pixel visible)
    if( aDragWinPos.X() < 5 )
    {
        bScrolling = ScrollPane( -LINE_SIZE, true, true );
        if( !bScrolling && (aDragWinPos.X()<0) )
            aDragWinPos.setX( 0 );

        // do I need further (timer controlled) scrolling ?
        bNeedScrollTimer = bScrolling && (aDragWinPos.X() < 5);
    }

    if( aLowerRight.X() > m_aOutputSize.Width() - 5 )
    {
        bScrolling = ScrollPane( LINE_SIZE, true, true ) ;
        if( !bScrolling && ( aLowerRight.X() > m_aOutputSize.Width() ) )
            aDragWinPos.setX( m_aOutputSize.Width() - aDragWinSize.Width() );

        // do I need further (timer controlled) scrolling ?
        bNeedScrollTimer = bScrolling && (aLowerRight.X() > m_aOutputSize.Width() - 5);
    }

    if( aDragWinPos.Y() < 5 )
    {
        bScrolling = ScrollPane( -LINE_SIZE, false, true );
        if( !bScrolling && (aDragWinPos.Y()<0) )
            aDragWinPos.setY( 0 );

        bNeedScrollTimer = bScrolling && (aDragWinPos.Y() < 5);
    }

    if( aLowerRight.Y() > m_aOutputSize.Height() - 5 )
    {
        bScrolling = ScrollPane( LINE_SIZE, false, true );
        if( !bScrolling && ( (aDragWinPos.Y() + aDragWinSize.Height()) > m_aOutputSize.Height() ) )
            aDragWinPos.setY(  m_aOutputSize.Height() - aDragWinSize.Height() );

        bNeedScrollTimer = bScrolling && (aLowerRight.Y() > m_aOutputSize.Height() - 5);
    }

    // resetting timer, if still necessary
    if (bNeedScrollTimer)
    {
        m_aDragScrollIdle.SetPriority( TaskPriority::HIGH_IDLE );
        m_aDragScrollIdle.Start();
    }

    // redraw DraggingRect
    m_aDragRect = tools::Rectangle(m_ptPrevDraggingPos - m_aDragOffset, m_pDragWin->GetSizePixel());
    Update();
    ShowTracking( m_aDragRect, ShowTrackFlags::Small | ShowTrackFlags::TrackWindow );
}

IMPL_LINK_NOARG(OJoinTableView, OnDragScrollTimer, Timer *, void)
{
    ScrollWhileDragging();
}

void OJoinTableView::invalidateAndModify(std::unique_ptr<SfxUndoAction> _pAction)
{
    Invalidate(InvalidateFlags::NoChildren);
    m_pView->getController().addUndoActionAndInvalidate(std::move(_pAction));
}

void OJoinTableView::TabWinMoved(OTableWindow* ptWhich, const Point& ptOldPosition)
{
    Point ptThumbPos(GetHScrollBar().GetThumbPos(), GetVScrollBar().GetThumbPos());
    ptWhich->GetData()->SetPosition(ptWhich->GetPosPixel() + ptThumbPos);

    invalidateAndModify(std::make_unique<OJoinMoveTabWinUndoAct>(this, ptOldPosition, ptWhich));
}

void OJoinTableView::TabWinSized(OTableWindow* ptWhich, const Point& ptOldPosition, const Size& szOldSize)
{
    ptWhich->GetData()->SetSize(ptWhich->GetSizePixel());
    ptWhich->GetData()->SetPosition(ptWhich->GetPosPixel());

    invalidateAndModify(std::make_unique<OJoinSizeTabWinUndoAct>(this, ptOldPosition, szOldSize, ptWhich));
}

bool OJoinTableView::IsAddAllowed()
{

    // not, if Db readonly
    if (m_pView->getController().isReadOnly())
        return false;

    try
    {
        Reference< XConnection> xConnection = m_pView->getController().getConnection();
        if(!xConnection.is())
            return false;
        // not, if too many tables already
        Reference < XDatabaseMetaData > xMetaData( xConnection->getMetaData() );

        sal_Int32 nMax = xMetaData.is() ? xMetaData->getMaxTablesInSelect() : 0;
        if (nMax && nMax <= static_cast<sal_Int32>(m_aTableMap.size()))
            return false;
    }
    catch(SQLException&)
    {
        return false;
    }

    return true;
}

void OJoinTableView::executePopup(const Point& _aPos, VclPtr<OTableConnection>& rSelConnection)
{
    VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "dbaccess/ui/joinviewmenu.ui", "");
    VclPtr<PopupMenu> aContextMenu(aBuilder.get_menu("menu"));
    aContextMenu->Execute(this, _aPos);
    OString sIdent = aContextMenu->GetCurItemIdent();
    if (sIdent == "delete")
        RemoveConnection(rSelConnection, true);
    else if (sIdent == "edit")
        ConnDoubleClicked(rSelConnection); // is the same as double clicked
}

void OJoinTableView::Command(const CommandEvent& rEvt)
{

    bool bHandled = false;

    switch (rEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            if( m_vTableConnection.empty() )
                return;

            VclPtr<OTableConnection>& rSelConnection = GetSelectedConn();
            // when it wasn't a mouse event use the selected connection
            if (!rEvt.IsMouseEvent())
            {
                if (rSelConnection)
                {
                    const std::vector<std::unique_ptr<OConnectionLine>>& rLines = rSelConnection->GetConnLineList();
                    auto aIter = std::find_if(rLines.begin(), rLines.end(),std::mem_fn(&OConnectionLine::IsValid));
                    if( aIter != rLines.end() )
                        executePopup((*aIter)->getMidPoint(), rSelConnection);
                }
            }
            else
            {
                DeselectConn(rSelConnection);

                const Point& aMousePos = rEvt.GetMousePosPixel();
                for (auto & elem : m_vTableConnection)
                {
                    if( elem->CheckHit(aMousePos) )
                    {
                        SelectConn(elem);
                        if(!getDesignView()->getController().isReadOnly() && getDesignView()->getController().isConnected())
                            executePopup(rEvt.GetMousePosPixel(),elem);
                        break;
                    }
                }
            }
            bHandled = true;
        }
        break;
        default: break;
    }
    if (!bHandled)
        Window::Command(rEvt);
}

OTableConnection* OJoinTableView::GetTabConn(const OTableWindow* pLhs,const OTableWindow* pRhs,bool _bSupressCrossOrNaturalJoin) const
{
    OTableConnection* pConn = nullptr;
    OSL_ENSURE(pRhs || pLhs, "OJoinTableView::GetTabConn : invalid args !");
        // only one NULL-arg allowed

    if ((!pLhs || pLhs->ExistsAConn()) && (!pRhs || pRhs->ExistsAConn()))
    {
        for(VclPtr<OTableConnection> const & pData : m_vTableConnection)
        {
            if  (   (   (pData->GetSourceWin() == pLhs)
                    &&  (   (pData->GetDestWin() == pRhs)
                        ||  (nullptr == pRhs)
                        )
                    )
                ||  (   (pData->GetSourceWin() == pRhs)
                    &&  (   (pData->GetDestWin() == pLhs)
                        ||  (nullptr == pLhs)
                        )
                    )
                )
            {
                if ( _bSupressCrossOrNaturalJoin )
                {
                    if ( supressCrossNaturalJoin(pData->GetData()) )
                        continue;
                }
                pConn = pData;
                break;
            }
        }
    }
    return pConn;
}

bool OJoinTableView::PreNotify(NotifyEvent& rNEvt)
{
    bool bHandled = false;
    switch (rNEvt.GetType())
    {
        case MouseNotifyEvent::COMMAND:
        {
            const CommandEvent* pCommand = rNEvt.GetCommandEvent();
            if (pCommand->GetCommand() == CommandEventId::Wheel)
            {
                const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
                if (pData->GetMode() == CommandWheelMode::SCROLL)
                {
                    if (pData->GetDelta() > 0)
                        ScrollPane(-10 * pData->GetScrollLines(), pData->IsHorz(), true);
                    else
                        ScrollPane(10 * pData->GetScrollLines(), pData->IsHorz(), true);
                    bHandled = true;
                }
            }
        }
        break;
        case MouseNotifyEvent::KEYINPUT:
        {
            if (m_aTableMap.empty())
                // no tab wins -> no conns -> no traveling
                break;

            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            if (!pKeyEvent->GetKeyCode().IsMod1())
            {
                switch (pKeyEvent->GetKeyCode().GetCode())
                {
                    case KEY_TAB:
                    {
                        if (!HasChildPathFocus())
                            break;

                        bool bForward = !pKeyEvent->GetKeyCode().IsShift();
                        // is there an active tab win ?
                        OTableWindowMap::const_iterator aIter = std::find_if(m_aTableMap.begin(), m_aTableMap.end(),
                            [](const OTableWindowMap::value_type& rEntry) { return rEntry.second && rEntry.second->HasChildPathFocus(); });

                        OTableWindow* pNextWin = nullptr;
                        OTableConnection* pNextConn = nullptr;

                        if (aIter != m_aTableMap.end())
                        {   // there is a currently active tab win
                            // check if there is an "overflow" and we should select a conn instead of a win
                            if (!m_vTableConnection.empty())
                            {
                                if ((aIter->second == m_aTableMap.rbegin()->second) && bForward)
                                    // the last win is active and we're travelling forward -> select the first conn
                                    pNextConn = m_vTableConnection.begin()->get();
                                if ((aIter == m_aTableMap.begin()) && !bForward)
                                    // the first win is active and we're traveling backward -> select the last conn
                                    pNextConn = m_vTableConnection.rbegin()->get();
                            }

                            if (!pNextConn)
                            {
                                // no conn for any reason -> select the next or previous tab win
                                if(bForward)
                                {
                                    if ( aIter->second == m_aTableMap.rbegin()->second )
                                        pNextWin = m_aTableMap.begin()->second;
                                    else
                                    {
                                        ++aIter;
                                        pNextWin = aIter->second;
                                    }
                                }
                                else
                                {
                                    if (aIter == m_aTableMap.begin())
                                        pNextWin = m_aTableMap.rbegin()->second;
                                    else
                                    {
                                        --aIter;
                                        pNextWin = aIter->second;
                                    }
                                }
                            }
                        }
                        else
                        {   // no active tab win -> travel the connections
                            // find the currently selected conn within the conn list
                            sal_Int32 i(0);
                            for (auto const& elem : m_vTableConnection)
                            {
                                if ( elem.get() == GetSelectedConn() )
                                    break;
                                ++i;
                            }
                            if (i == sal_Int32(m_vTableConnection.size() - 1) && bForward)
                                // the last conn is active and we're travelling forward -> select the first win
                                pNextWin = m_aTableMap.begin()->second;
                            if ((i == 0) && !bForward && !m_aTableMap.empty())
                                // the first conn is active and we're travelling backward -> select the last win
                                pNextWin = m_aTableMap.rbegin()->second;

                            if (pNextWin)
                                DeselectConn(GetSelectedConn());
                            else
                                // no win for any reason -> select the next or previous conn
                                if (i < static_cast<sal_Int32>(m_vTableConnection.size()))
                                    // there is a currently active conn
                                    pNextConn = m_vTableConnection[(i + (bForward ? 1 : m_vTableConnection.size() - 1)) % m_vTableConnection.size()].get();
                                else
                                {   // no tab win selected, no conn selected
                                    if (!m_vTableConnection.empty())
                                        pNextConn = m_vTableConnection[bForward ? 0 : m_vTableConnection.size() - 1].get();
                                    else if (!m_aTableMap.empty())
                                    {
                                        if(bForward)
                                            pNextWin = m_aTableMap.begin()->second;
                                        else
                                            pNextWin = m_aTableMap.rbegin()->second;
                                    }
                                }
                        }

                        // now select the object
                        if (pNextWin)
                        {
                            if (pNextWin->GetListBox())
                                pNextWin->GetListBox()->GrabFocus();
                            else
                                pNextWin->GrabFocus();
                            EnsureVisible(pNextWin);
                        }
                        else if (pNextConn)
                        {
                            GrabFocus();
                                // necessary : a conn may be selected even if a tab win has the focus, in this case
                                // the next travel would select the same conn again if we would not reset the focus ...
                            SelectConn(pNextConn);
                        }
                    }
                    break;
                    case KEY_RETURN:
                    {
                        if (!pKeyEvent->GetKeyCode().IsShift() && GetSelectedConn() && HasFocus())
                            ConnDoubleClicked(GetSelectedConn());
                        break;
                    }
                }
            }
        }
        break;
        case MouseNotifyEvent::GETFOCUS:
        {
            if (m_aTableMap.empty())
                // no tab wins -> no conns -> no focus change
                break;
            vcl::Window* pSource = rNEvt.GetWindow();
            if (pSource)
            {
                vcl::Window* pSearchFor = nullptr;
                if (pSource->GetParent() == this)
                    // it may be one of the tab wins
                    pSearchFor = pSource;
                else if (pSource->GetParent() && (pSource->GetParent()->GetParent() == this))
                    // it may be one of th list boxes of one of the tab wins
                    pSearchFor = pSource->GetParent();

                if (pSearchFor)
                {
                    for (auto const& elem : m_aTableMap)
                    {
                        if (elem.second == pSearchFor)
                        {
                            m_pLastFocusTabWin = elem.second;
                            break;
                        }
                    }
                }
            }
        }
        break;
        default:
        break;
    }

    if (!bHandled)
        return Window::PreNotify(rNEvt);
    return true;
}

void OJoinTableView::GrabTabWinFocus()
{
    if (m_pLastFocusTabWin && m_pLastFocusTabWin->IsVisible())
    {
        if (m_pLastFocusTabWin->GetListBox())
            m_pLastFocusTabWin->GetListBox()->GrabFocus();
        else
            m_pLastFocusTabWin->GrabFocus();
    }
    else if (!m_aTableMap.empty() && m_aTableMap.begin()->second && m_aTableMap.begin()->second->IsVisible())
    {
        VclPtr<OTableWindow> pFirstWin = m_aTableMap.begin()->second;
        if (pFirstWin->GetListBox())
            pFirstWin->GetListBox()->GrabFocus();
        else
            pFirstWin->GrabFocus();
    }
}

void OJoinTableView::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    // FIXME RenderContext
    if ( nType == StateChangedType::Zoom )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        vcl::Font aFont = rStyleSettings.GetGroupFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont(*this, aFont);

        for (auto const& elem : m_aTableMap)
        {
            elem.second->SetZoom(GetZoom());
            Size aSize(CalcZoom(elem.second->GetSizePixel().Width()),CalcZoom(elem.second->GetSizePixel().Height()));
            elem.second->SetSizePixel(aSize);
        }
        Resize();
    }
}

void OJoinTableView::HideTabWins()
{
    SetUpdateMode(false);

    OTableWindowMap& rTabWins = GetTabWinMap();

    // working on a copy because the real list will be cleared in inner calls
    OTableWindowMap aCopy(rTabWins);
    for (auto const& elem : aCopy)
        RemoveTabWin(elem.second);

    m_pView->getController().setModified(true);

    SetUpdateMode(true);

}

sal_Int8 OJoinTableView::AcceptDrop( const AcceptDropEvent& /*_rEvt*/ )
{
    return DND_ACTION_NONE;
}

sal_Int8 OJoinTableView::ExecuteDrop( const ExecuteDropEvent& /*_rEvt*/ )
{
    return DND_ACTION_NONE;
}

void OJoinTableView::dragFinished( )
{
}

void OJoinTableView::clearLayoutInformation()
{
    m_pLastFocusTabWin  = nullptr;
    m_pSelectedConn     = nullptr;
    // delete lists
    for (auto & elem : m_aTableMap)
    {
        if ( elem.second )
            elem.second->clearListBox();
        elem.second.disposeAndClear();
    }

    m_aTableMap.clear();

    for (auto & elem : m_vTableConnection)
        elem.disposeAndClear();

    m_vTableConnection.clear();
}

void OJoinTableView::lookForUiActivities()
{
}

void OJoinTableView::LoseFocus()
{
    DeselectConn(GetSelectedConn());
    Window::LoseFocus();
}

void OJoinTableView::GetFocus()
{
    Window::GetFocus();
    if ( !m_aTableMap.empty() && !GetSelectedConn() )
        GrabTabWinFocus();
}

Reference< XAccessible > OJoinTableView::CreateAccessible()
{
    m_pAccessible = new OJoinDesignViewAccess(this);
    return m_pAccessible;
}

void OJoinTableView::modified()
{
    OJoinController& rController = m_pView->getController();
    rController.setModified( true );
    rController.InvalidateFeature(ID_BROWSER_ADDTABLE);
    rController.InvalidateFeature(SID_RELATION_ADD_RELATION);
}

void OJoinTableView::addConnection(OTableConnection* _pConnection,bool _bAddData)
{
    if ( _bAddData )
    {
#if OSL_DEBUG_LEVEL > 0
        TTableConnectionData& rTabConnDataList = m_pView->getController().getTableConnectionData();
        OSL_ENSURE( std::find(rTabConnDataList.begin(), rTabConnDataList.end(),_pConnection->GetData()) == rTabConnDataList.end(),"Data already in vector!");
#endif
        m_pView->getController().getTableConnectionData().push_back(_pConnection->GetData());
    }
    m_vTableConnection.emplace_back(_pConnection);
    _pConnection->RecalcLines();
    _pConnection->InvalidateConnection();

    modified();
    if ( m_pAccessible )
        m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                Any(),
                                                makeAny(_pConnection->GetAccessible()));
}

bool OJoinTableView::allowQueries() const
{
    return true;
}

void OJoinTableView::onNoColumns_throw()
{
    OSL_FAIL( "OTableWindow::onNoColumns_throw: cannot really handle this!" );
    throw SQLException();
}

bool OJoinTableView::supressCrossNaturalJoin(const TTableConnectionData::value_type& ) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
