/*************************************************************************
 *
 *  $RCSfile: JoinTableView.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:12:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
//#ifndef DBAUI_QUERY_TABLEWINDOWDATA_HXX
//#include "QTableWindowData.hxx"
//#endif
#ifndef DBAUI_TABLEWINDOWLISTBOX_HXX
#include "TableWindowListBox.hxx"
#endif
#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINE_HXX
#include "ConnectionLine.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _URLBMK_HXX
#include <svtools/urlbmk.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef DBAUI_OQUERYMOVETABWINUNDOACT_HXX
#include "QueryMoveTabWinUndoAct.hxx"
#endif
#ifndef DBAUI_QUERYSIZETABWINUNDOACT_HXX
#include "QuerySizeTabWinUndoAct.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#include "TableWindowData.hxx"
#endif
#ifndef DBACCESS_JACCESS_HXX
#include "JAccess.hxx"
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#include <algorithm>
#include <functional>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::accessibility;

#define LINE_SIZE           50
////////////////////////////////////////////////////////////////
// Konstanten fuer das Fensterlayout
#define TABWIN_SPACING_X    17
#define TABWIN_SPACING_Y    17

#define TABWIN_WIDTH_STD    120
#define TABWIN_HEIGHT_STD   120

DBG_NAME(OScrollWindowHelper);
OScrollWindowHelper::OScrollWindowHelper( Window* pParent) : Window( pParent)
    ,m_aHScrollBar( this, WB_HSCROLL|WB_REPEAT|WB_DRAG )
    ,m_aVScrollBar( this, WB_VSCROLL|WB_REPEAT|WB_DRAG )
    ,m_pTableView(NULL)
    ,m_pCornerWindow(new ScrollBarBox(this, WB_3DLOOK))
{
    DBG_CTOR(OScrollWindowHelper,NULL);

    //////////////////////////////////////////////////////////////////////
    // ScrollBars

    GetHScrollBar()->SetRange( Range(0, 1000) );
    GetVScrollBar()->SetRange( Range(0, 1000) );

    GetHScrollBar()->SetLineSize( LINE_SIZE );
    GetVScrollBar()->SetLineSize( LINE_SIZE );

    GetHScrollBar()->Show();
    GetVScrollBar()->Show();
    m_pCornerWindow->Show();

    // normally we should be SCROLL_PANE
    SetAccessibleRole(AccessibleRole::SCROLL_PANE);
}

// -----------------------------------------------------------------------------
OScrollWindowHelper::~OScrollWindowHelper()
{
    DBG_DTOR(OScrollWindowHelper,NULL);
    ::std::auto_ptr<Window> aTemp(m_pCornerWindow);
    m_pCornerWindow = NULL;
    m_pTableView = NULL;
}

// -----------------------------------------------------------------------------
void OScrollWindowHelper::setTableView(OJoinTableView* _pTableView)
{
    m_pTableView = _pTableView;
    //////////////////////////////////////////////////////////////////////
    // ScrollBars
    GetHScrollBar()->SetScrollHdl( LINK(m_pTableView, OJoinTableView, ScrollHdl) );
    GetVScrollBar()->SetScrollHdl( LINK(m_pTableView, OJoinTableView, ScrollHdl) );
}
//------------------------------------------------------------------------------
void OScrollWindowHelper::Resize()
{
    Window::Resize();

    Size aTotalOutputSize = GetOutputSizePixel();
    long nHScrollHeight = GetHScrollBar()->GetSizePixel().Height();
    long nVScrollWidth = GetVScrollBar()->GetSizePixel().Width();

    GetHScrollBar()->SetPosSizePixel(
        Point( 0, aTotalOutputSize.Height()-nHScrollHeight ),
        Size( aTotalOutputSize.Width()-nVScrollWidth, nHScrollHeight )
        );

    GetVScrollBar()->SetPosSizePixel(
        Point( aTotalOutputSize.Width()-nVScrollWidth, 0 ),
        Size( nVScrollWidth, aTotalOutputSize.Height()-nHScrollHeight )
        );

    m_pCornerWindow->SetPosSizePixel(
        Point( aTotalOutputSize.Width() - nVScrollWidth, aTotalOutputSize.Height() - nHScrollHeight),
        Size( nVScrollWidth, nHScrollHeight )
        );

    GetHScrollBar()->SetPageSize( aTotalOutputSize.Width() );
    GetHScrollBar()->SetVisibleSize( aTotalOutputSize.Width() );

    GetVScrollBar()->SetPageSize( aTotalOutputSize.Height() );
    GetVScrollBar()->SetVisibleSize( aTotalOutputSize.Height() );

    // adjust the ranges of the scrollbars if neccessary
    long lRange = GetHScrollBar()->GetRange().Max() - GetHScrollBar()->GetRange().Min();
    if (m_pTableView->GetScrollOffset().X() + aTotalOutputSize.Width() > lRange)
        GetHScrollBar()->SetRangeMax(m_pTableView->GetScrollOffset().X() + aTotalOutputSize.Width() + GetHScrollBar()->GetRange().Min());

    lRange = GetVScrollBar()->GetRange().Max() - GetVScrollBar()->GetRange().Min();
    if (m_pTableView->GetScrollOffset().Y() + aTotalOutputSize.Height() > lRange)
        GetVScrollBar()->SetRangeMax(m_pTableView->GetScrollOffset().Y() + aTotalOutputSize.Height() + GetVScrollBar()->GetRange().Min());

    m_pTableView->SetPosSizePixel(Point( 0, 0 ),Size( aTotalOutputSize.Width()-nVScrollWidth, aTotalOutputSize.Height()-nHScrollHeight ));
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
TYPEINIT0(OJoinTableView);
//==================================================================
// class OJoinTableView
//==================================================================

//const long WINDOW_WIDTH = 1000;
//const long WINDOW_HEIGHT = 1000;
DBG_NAME(OJoinTableView);
//------------------------------------------------------------------------------
OJoinTableView::OJoinTableView( Window* pParent, OJoinDesignView* pView ) :
     Window( pParent,WB_BORDER )
    ,DropTargetHelper(this)
    ,m_pView( pView )
    ,m_pDragWin( NULL )
    ,m_pSizingWin( NULL )
    ,m_pSelectedConn( NULL )
    ,m_aDragOffset( Point(0,0) )
    ,m_aScrollOffset( Point(0,0) )
    ,m_bTrackingInitiallyMoved(FALSE)
    ,m_pLastFocusTabWin(NULL)
    ,m_pAccessible(NULL)
{
    DBG_CTOR(OJoinTableView,NULL);
    SetSizePixel( Size(1000, 1000) );

    InitColors();

    m_aDragScrollTimer.SetTimeoutHdl(LINK(this, OJoinTableView, OnDragScrollTimer));
}

//------------------------------------------------------------------------------
OJoinTableView::~OJoinTableView()
{
    DBG_DTOR(OJoinTableView,NULL);
    //////////////////////////////////////////////////////////////////////
    // Listen loeschen
    clearLayoutInformation();

    m_pLastFocusTabWin  = NULL;
    m_pSelectedConn     = NULL;
    m_pAccessible = NULL;
}
//------------------------------------------------------------------------------
IMPL_LINK( OJoinTableView, ScrollHdl, ScrollBar*, pScrollBar )
{
    //////////////////////////////////////////////////////////////////////
    // Alle Fenster verschieben
    Scroll( pScrollBar->GetDelta(), (pScrollBar == GetHScrollBar()), FALSE );

    return 0;
}
//------------------------------------------------------------------------------
void OJoinTableView::Resize()
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    Window::Resize();
    m_aOutputSize = GetSizePixel();

    // tab win positions may not be up-to-date
    if (m_aTableMap.empty())
        // no tab wins ...
        return;

    // we have at least one table so resize it
    m_aScrollOffset.X() = GetHScrollBar()->GetThumbPos();
    m_aScrollOffset.Y() = GetVScrollBar()->GetThumbPos();

    OTableWindow* pCheck = m_aTableMap.begin()->second;
    Point aRealPos = pCheck->GetPosPixel();
    Point aAssumedPos = pCheck->GetData()->GetPosition() - GetScrollOffset();

    if (aRealPos == aAssumedPos)
        // all ok
        return;

    OTableWindowMapIterator aIter = m_aTableMap.begin();
    for(;aIter != m_aTableMap.end();++aIter)
    {
        OTableWindow* pCurrent = aIter->second;
        Point aPos(pCurrent->GetData()->GetPosition() - GetScrollOffset());
        pCurrent->SetPosPixel(aPos);
    }
}
//------------------------------------------------------------------------------
ULONG OJoinTableView::GetTabWinCount()
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    return m_aTableMap.size();
}

//------------------------------------------------------------------------------
void OJoinTableView::AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest)
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    sal_Int32 nOldCount = m_vTableConnection.size();
}

//------------------------------------------------------------------------------
::std::vector<OTableConnection*>::const_iterator OJoinTableView::RemoveConnection( OTableConnection* _pConn,sal_Bool _bDelete )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    DeselectConn(_pConn);

    // to force a redraw
    _pConn->Invalidate();

    m_pView->getController()->removeConnectionData( _pConn->GetData() );

    ::std::vector<OTableConnection*>::const_iterator aNextPos = m_vTableConnection.erase(
                        ::std::find(m_vTableConnection.begin(),m_vTableConnection.end(),_pConn) );

    modified();
    if ( m_pAccessible )
        m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                makeAny(_pConn->GetAccessible()),
                                                Any());
    if ( _bDelete )
    {
        delete _pConn->GetData();
        delete _pConn;
    }

    return aNextPos;
}

//------------------------------------------------------------------------
OTableWindow* OJoinTableView::GetWindow( const String& rName )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    OTableWindowMapIterator aIter = m_aTableMap.find(rName);

    return aIter == m_aTableMap.end() ? NULL : aIter->second;
}
// -----------------------------------------------------------------------------
OTableWindowData* OJoinTableView::CreateImpl(const ::rtl::OUString& _rComposedName,
                                             const ::rtl::OUString& _rWinName)
{
    return new OTableWindowData( _rComposedName, _rWinName );
}
//------------------------------------------------------------------------------
void OJoinTableView::AddTabWin(const ::rtl::OUString& _rComposedName, const ::rtl::OUString& rWinName, BOOL bNewTable)
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    OSL_ENSURE(_rComposedName.getLength(),"There must be a table name supplied!");

    //////////////////////////////////////////////////////////////////
    // Neue Datenstruktur in DocShell eintragen
    OTableWindowData* pNewTabWinData = CreateImpl( _rComposedName, rWinName );


    //////////////////////////////////////////////////////////////////
    // Neues Fenster in Fensterliste eintragen
    OTableWindow* pNewTabWin = createWindow( pNewTabWinData );
    if ( pNewTabWin->Init() )
    {
        m_pView->getController()->getTableWindowData()->push_back( pNewTabWinData);
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
        delete pNewTabWinData;
        pNewTabWin->clearListBox();
        delete pNewTabWin;
    }
}

//------------------------------------------------------------------------------
void OJoinTableView::RemoveTabWin( OTableWindow* pTabWin )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    //////////////////////////////////////////////////////////////////////
    // first delete all connections of this window to others
    String aWinName = pTabWin->GetWinName();
    String sComposedName = pTabWin->GetComposedName();
    BOOL bRemove = TRUE;

    sal_Int32 nCount = m_vTableConnection.size();
    ::std::vector<OTableConnection*>::reverse_iterator aIter = m_vTableConnection.rbegin();
    for(;aIter != m_vTableConnection.rend();++aIter)
    {
        OTableConnection* pTabConn = (*aIter);
        if(
            ( aWinName == pTabConn->GetData()->GetSourceWinName())      ||
            ( aWinName == pTabConn->GetData()->GetDestWinName())        ||
            ( sComposedName == pTabConn->GetData()->GetSourceWinName()) ||
            ( sComposedName == pTabConn->GetData()->GetDestWinName())
          )
          bRemove = RemoveConnection( pTabConn ,sal_True) != m_vTableConnection.end();
    }

    //////////////////////////////////////////////////////////////////////
    // then delete the window itself
    if ( bRemove )
    {
        if ( m_pAccessible )
            m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                    makeAny(pTabWin->GetAccessible()),Any()
                                                    );

        pTabWin->Hide();
        ::std::vector< OTableWindowData*>::iterator aFind = ::std::find(m_pView->getController()->getTableWindowData()->begin(),m_pView->getController()->getTableWindowData()->end(),pTabWin->GetData());
        if(aFind != m_pView->getController()->getTableWindowData()->end())
        {
            delete *aFind;
            m_pView->getController()->getTableWindowData()->erase(aFind);
            m_pView->getController()->setModified(sal_True);
        }

        if(m_aTableMap.find(aWinName) != m_aTableMap.end())
            m_aTableMap.erase( aWinName );
        else
            m_aTableMap.erase( sComposedName );

        if (pTabWin == m_pLastFocusTabWin)
            m_pLastFocusTabWin = NULL;

        pTabWin->clearListBox();
        delete pTabWin;
    }

    if ( bRemove && (sal_Int32)m_aTableMap.size() < (nCount-1) ) // if some connections could be removed
        modified();
}
namespace
{
    // -----------------------------------------------------------------------------
    BOOL isScrollAllowed( OJoinTableView* _pView,long nDelta, BOOL bHoriz)
    {
        BOOL bRet = TRUE;
        //////////////////////////////////////////////////////////////////////
        // adjust ScrollBar-Positions
        ScrollBar* pBar = _pView->GetVScrollBar();
        if( bHoriz )
            pBar = _pView->GetHScrollBar();

        long nOldThumbPos = pBar->GetThumbPos();
        long nNewThumbPos = nOldThumbPos + nDelta;
        if( nNewThumbPos < 0 )
            nNewThumbPos = 0;// bRet = FALSE;
        else if( nNewThumbPos > pBar->GetRangeMax() )
            nNewThumbPos = pBar->GetRangeMax();// bRet = FALSE;

        if ( bHoriz )
        {
            if( nNewThumbPos == _pView->GetScrollOffset().X() )
                return FALSE;
        }
        else if ( nNewThumbPos == _pView->GetScrollOffset().Y() )
            return FALSE;

        return bRet;
    }
    // -----------------------------------------------------------------------------
    BOOL getMovementImpl(OJoinTableView* _pView,const Point& _rPoint,const Size& _rSize,long& _nScrollX,long& _nScrollY)
    {
        _nScrollY = _nScrollX = 0;
        // data about the tab win
        Point aUpperLeft = _rPoint;
        // normalize with respect to visibility
        aUpperLeft -= _pView->GetScrollOffset();
        //  aUpperLeft.Y() -= _pView->GetScrollOffset().Y();
        Point aLowerRight(aUpperLeft.X() + _rSize.Width(), aUpperLeft.Y() + _rSize.Height());

        // data about ourself
        Size aSize = _pView->getRealOutputSize(); //GetOutputSizePixel();

        BOOL bVisbile = TRUE;
        BOOL bFitsHor = (aUpperLeft.X() >= 0) && (aLowerRight.X() <= aSize.Width());
        BOOL bFitsVert= (aUpperLeft.Y() >= 0) && (aLowerRight.Y() <= aSize.Height());
        if (!bFitsHor || !bFitsVert)
        {
            // #100386# OJ
            if (!bFitsHor)
            {
                // ensure the visibility of the right border
                if ( aLowerRight.X() > aSize.Width() )
                    _nScrollX = aLowerRight.X() - aSize.Width() + TABWIN_SPACING_X;

                // ensure the visibility of the left border (higher priority)
                //  if ( (aUpperLeft.X() - _nScrollX) < 0 )
                if ( aUpperLeft.X() < 0 )
                    _nScrollX = aUpperLeft.X() - TABWIN_SPACING_X;
            }

            if (!bFitsVert)
            {
                // lower border
                if ( aLowerRight.Y() > aSize.Height() )
                    _nScrollY = aLowerRight.Y() - aSize.Height() + TABWIN_SPACING_Y;
                // upper border
                //  if ( (aUpperLeft.Y() - _nScrollY) < 0 )
                if ( aUpperLeft.Y() < 0 )
                    _nScrollY = aUpperLeft.Y() - TABWIN_SPACING_Y;
            }

            if ( _nScrollX ) // aSize.Width() > _rSize.Width() &&
                bVisbile = isScrollAllowed(_pView,_nScrollX, TRUE);

            if ( _nScrollY ) // aSize.Height() > _rSize.Height() &&
                bVisbile = bVisbile && isScrollAllowed(_pView,_nScrollY, FALSE);

            if ( bVisbile )
            {
                sal_Int32 nHRangeMax = _pView->GetHScrollBar()->GetRangeMax();
                sal_Int32 nVRangeMax = _pView->GetVScrollBar()->GetRangeMax();

                if ( aSize.Width() + _pView->GetHScrollBar()->GetThumbPos() + _nScrollX > nHRangeMax )
                    bVisbile = FALSE;
                if ( bVisbile && aSize.Height() + _pView->GetVScrollBar()->GetThumbPos() + _nScrollY > nVRangeMax )
                    bVisbile = FALSE;
            }
        }


        return bVisbile;
    }
} // end of ano namespace
// -----------------------------------------------------------------------------
BOOL OJoinTableView::isMovementAllowed(const Point& _rPoint,const Size& _rSize)
{
    long nX,nY;
    return getMovementImpl(this,_rPoint,_rSize,nX,nY);
}
//------------------------------------------------------------------------------
void OJoinTableView::EnsureVisible(const OTableWindow* _pWin)
{
    // data about the tab win
    OTableWindowData* pData = _pWin->GetData();
    //  Point aUpperLeft = pData->GetPosition();
    EnsureVisible( pData->GetPosition() , pData->GetSize());
    Invalidate(INVALIDATE_NOCHILDREN);
}
//------------------------------------------------------------------------------
void OJoinTableView::EnsureVisible(const Point& _rPoint,const Size& _rSize)
{
    long nScrollX,nScrollY;

    if ( getMovementImpl(this,_rPoint,_rSize,nScrollX,nScrollY) )
    {
        BOOL bVisbile = TRUE;
        if (nScrollX)
            bVisbile = Scroll(nScrollX, TRUE, TRUE);

        if (nScrollY)
            bVisbile = bVisbile && Scroll(nScrollY, FALSE, TRUE);
    }
}

//------------------------------------------------------------------------------
void OJoinTableView::SetDefaultTabWinPosSize( OTableWindow* pTabWin )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    //////////////////////////////////////////////////////////////////
    // Position bestimmen:
    // Das Fenster wird in Zeilen der Hoehe TABWIN_SPACING_Y+TABWIN_HEIGTH_STD aufgeteilt.
    // Dann wird fuer jede Zeile geprueft, ob noch Platz fuer ein weiteres Fenster ist.
    // Wenn kein Platz ist, wird die naechste Zeile ueberprueft.
    Size aOutSize = GetSizePixel();
    Point aNewPos( 0,0 );
    USHORT nRow = 0;
    BOOL bEnd = FALSE;
    while( !bEnd )
    {
        //////////////////////////////////////////////////////////////////
        // Neue Position auf Zeilenbeginn setzen
        aNewPos.X() = TABWIN_SPACING_X;
        aNewPos.Y() = (nRow+1) * TABWIN_SPACING_Y;

        //////////////////////////////////////////////////////////////////
        // Rectangle fuer die jeweilige Zeile bestimmen
        Rectangle aRowRect( Point(0,0), aOutSize );
        aRowRect.Top() = nRow * ( TABWIN_SPACING_Y + TABWIN_HEIGHT_STD );
        aRowRect.Bottom() = (nRow+1) * ( TABWIN_SPACING_Y + TABWIN_HEIGHT_STD );

        //////////////////////////////////////////////////////////////////
        // Belegte Bereiche dieser Zeile pruefen
        OTableWindow* pOtherTabWin;// = GetTabWinMap()->First();
        OTableWindowMapIterator aIter = m_aTableMap.begin();
        for(;aIter != m_aTableMap.end();++aIter)
        {
            pOtherTabWin = aIter->second;
            Rectangle aOtherTabWinRect( pOtherTabWin->GetPosPixel(), pOtherTabWin->GetSizePixel() );

            if(
                ( (aOtherTabWinRect.Top()>aRowRect.Top()) && (aOtherTabWinRect.Top()<aRowRect.Bottom()) ) ||
                ( (aOtherTabWinRect.Bottom()>aRowRect.Top()) && (aOtherTabWinRect.Bottom()<aRowRect.Bottom()) )
              )
            {
                //////////////////////////////////////////////////////////////////
                // TabWin liegt in der Zeile
                if( aOtherTabWinRect.Right()>aNewPos.X() )
                    aNewPos.X() = aOtherTabWinRect.Right() + TABWIN_SPACING_X;
            }
        }

        //////////////////////////////////////////////////////////////////
        // Ist in dieser Zeile noch Platz?
        if( (aNewPos.X()+TABWIN_WIDTH_STD)<aRowRect.Right() )
        {
            aNewPos.Y() = aRowRect.Top() + TABWIN_SPACING_Y;
            bEnd = TRUE;
        }
        else
        {
            if( (aRowRect.Bottom()+aRowRect.GetHeight()) > aOutSize.Height() )
            {
                // insert it in the first row
                sal_Int32 nCount = m_aTableMap.size() % (nRow+1);
                ++nCount;
                aNewPos.Y() = nCount * TABWIN_SPACING_Y + (nCount-1)*CalcZoom(TABWIN_HEIGHT_STD);
                bEnd = TRUE;
            }
            else
                nRow++;

        }
    }

    //////////////////////////////////////////////////////////////////
    // Groesse bestimmen
    Size aNewSize( CalcZoom(TABWIN_WIDTH_STD), CalcZoom(TABWIN_HEIGHT_STD) );

    // check if the new position in inside the scrollbars ranges
    Point aBottom(aNewPos);
    aBottom.X() += aNewSize.Width();
    aBottom.Y() += aNewSize.Height();

    if(!GetHScrollBar()->GetRange().IsInside(aBottom.X()))
        GetHScrollBar()->SetRange( Range(0, aBottom.X()) );
    if(!GetVScrollBar()->GetRange().IsInside(aBottom.Y()))
        GetVScrollBar()->SetRange( Range(0, aBottom.Y()) );

    pTabWin->SetPosSizePixel( aNewPos, aNewSize );
}

//------------------------------------------------------------------------------
void OJoinTableView::DataChanged(const DataChangedEvent& rDCEvt)
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS)
    {
        // nehmen wir den worst-case an : die Farben haben sich geaendert, also
        // mich anpassen
        InitColors();
        Invalidate(INVALIDATE_NOCHILDREN);
        // durch das Invalidate werden auch die Connections neu gezeichnet, so dass die auch
        // gleich in den neuen Farben dargestellt werden
    }
}

//------------------------------------------------------------------------------
void OJoinTableView::InitColors()
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    // die Farben fuer die Darstellung sollten die Systemfarben sein
    StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
    SetBackground(Wallpaper(Color(aSystemStyle.GetDialogColor())));
}

//------------------------------------------------------------------------------
void OJoinTableView::BeginChildMove( OTableWindow* pTabWin, const Point& rMousePos  )
{
    DBG_CHKTHIS(OJoinTableView,NULL);

    if (m_pView->getController()->isReadOnly())
        return;

    m_pDragWin = pTabWin;
    SetPointer(Pointer(POINTER_MOVE));
    Point aMousePos = ScreenToOutputPixel( rMousePos );
    m_aDragOffset = aMousePos - pTabWin->GetPosPixel();
    m_pDragWin->SetZOrder(NULL, WINDOW_ZORDER_FIRST);
    m_bTrackingInitiallyMoved = FALSE;
    StartTracking();
}

void OJoinTableView::NotifyTitleClicked( OTableWindow* pTabWin, const Point rMousePos )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    DeselectConn(GetSelectedConn());
    BeginChildMove(pTabWin, rMousePos);
}

//------------------------------------------------------------------------------
void OJoinTableView::BeginChildSizing( OTableWindow* pTabWin, const Pointer& rPointer )
{
    DBG_CHKTHIS(OJoinTableView,NULL);

    if (m_pView->getController()->isReadOnly())
        return;

    SetPointer( rPointer );
    m_pSizingWin = pTabWin;
    StartTracking();
}

//------------------------------------------------------------------------------
BOOL OJoinTableView::Scroll( long nDelta, BOOL bHoriz, BOOL bPaintScrollBars )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    BOOL bRet = TRUE;

    //////////////////////////////////////////////////////////////////////
    // ScrollBar-Positionen anpassen
    if( bPaintScrollBars )
    {
        if( bHoriz )
        {
            long nOldThumbPos = GetHScrollBar()->GetThumbPos();
            long nNewThumbPos = nOldThumbPos + nDelta;
            if( nNewThumbPos < 0 )
            {
                nNewThumbPos = 0;
                bRet = FALSE;
            }
            if( nNewThumbPos > GetHScrollBar()->GetRange().Max() )
            {
                nNewThumbPos = GetHScrollBar()->GetRange().Max();
                bRet = FALSE;
            }
            GetHScrollBar()->SetThumbPos( nNewThumbPos );
            nDelta = GetHScrollBar()->GetThumbPos() - nOldThumbPos;
        }
        else
        {
            long nOldThumbPos = GetVScrollBar()->GetThumbPos();
            long nNewThumbPos = nOldThumbPos+nDelta;
            if( nNewThumbPos < 0 )
            {
                nNewThumbPos = 0;
                bRet = FALSE;
            }
            if( nNewThumbPos > GetVScrollBar()->GetRange().Max() )
            {
                nNewThumbPos = GetVScrollBar()->GetRange().Max();
                bRet = FALSE;
            }
            GetVScrollBar()->SetThumbPos( nNewThumbPos );
            nDelta = GetVScrollBar()->GetThumbPos() - nOldThumbPos;
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Wenn ScrollOffset bereits an den Grenzen liegt, kein Neuzeichnen
    if( (GetHScrollBar()->GetThumbPos()==m_aScrollOffset.X()) &&
        (GetVScrollBar()->GetThumbPos()==m_aScrollOffset.Y()) )
        return FALSE;

    //////////////////////////////////////////////////////////////////////
    // ScrollOffset neu setzen
    if (bHoriz)
        m_aScrollOffset.X() = GetHScrollBar()->GetThumbPos();
    else
        m_aScrollOffset.Y() = GetVScrollBar()->GetThumbPos();

    //////////////////////////////////////////////////////////////////////
    // Alle Fenster verschieben
    OTableWindow* pTabWin;
    Point aPos;

    OTableWindowMapIterator aIter = m_aTableMap.begin();
    for(;aIter != m_aTableMap.end();++aIter)
    {
        pTabWin = aIter->second;
        aPos = pTabWin->GetPosPixel();

        if( bHoriz )
            aPos.X() -= nDelta;
        else aPos.Y() -= nDelta;

        pTabWin->SetPosPixel( aPos );
    }

    Invalidate(); // INVALIDATE_NOCHILDREN

    return bRet;
}

//------------------------------------------------------------------------------
void OJoinTableView::Tracking( const TrackingEvent& rTEvt )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    HideTracking();

    if (rTEvt.IsTrackingEnded())
    {
        if( m_pDragWin )
        {
            if (m_aDragScrollTimer.IsActive())
                m_aDragScrollTimer.Stop();

            //////////////////////////////////////////////////////////////////////
            // Position des Childs nach Verschieben anpassen
            //////////////////////////////////////////////////////////////////////
            // Fenster duerfen nicht aus Anzeigebereich herausbewegt werden
            Point aDragWinPos = rTEvt.GetMouseEvent().GetPosPixel() - m_aDragOffset;
            Size aDragWinSize = m_pDragWin->GetSizePixel();
            if( aDragWinPos.X() < 0 )
                aDragWinPos.X() = 0;
            if( aDragWinPos.Y() < 0 )
                aDragWinPos.Y() = 0;
            if( (aDragWinPos.X() + aDragWinSize.Width()) > m_aOutputSize.Width() )
                aDragWinPos.X() = m_aOutputSize.Width() - aDragWinSize.Width() - 1;
            if( (aDragWinPos.Y() + aDragWinSize.Height()) > m_aOutputSize.Height() )
                aDragWinPos.Y() = m_aOutputSize.Height() - aDragWinSize.Height() - 1;
            if( aDragWinPos.X() < 0 )
                aDragWinPos.X() = 0;
            if( aDragWinPos.Y() < 0 )
                aDragWinPos.Y() = 0;
            // TODO : nicht das Fenster neu positionieren, wenn es uebersteht, sondern einfach meinen Bereich erweitern


            //////////////////////////////////////////////////////////////////////
            // Fenster positionieren
            EndTracking();
            m_pDragWin->SetZOrder(NULL, WINDOW_ZORDER_FIRST);
            // erst mal testen, ob ich mich ueberhaupt bewegt habe
            // (das verhindert das Setzen des modified-Flags, wenn sich eigentlich gar nichts getan hat)
            OTableWindowData* pData = m_pDragWin->GetData();
            if ( ! (pData && pData->HasPosition() && (pData->GetPosition() == aDragWinPos)))
            {
                // die alten logischen Koordinaten
                Point ptOldPos = m_pDragWin->GetPosPixel() + Point(GetHScrollBar()->GetThumbPos(), GetVScrollBar()->GetThumbPos());
                // neu positionieren
                m_pDragWin->SetPosPixel(aDragWinPos);
                TabWinMoved(m_pDragWin, ptOldPos);

                m_pDragWin->GrabFocus();
            }
            m_pDragWin = NULL;
            SetPointer(Pointer(POINTER_ARROW));
        }
        // else we handle the resizing
        else if( m_pSizingWin )
        {
            SetPointer( Pointer() );
            EndTracking();

            // die alten physikalischen Koordinaten

            Size szOld = m_pSizingWin->GetSizePixel();
            Point ptOld = m_pSizingWin->GetPosPixel();
            Size aNewSize(CalcZoom(m_aSizingRect.GetSize().Width()),CalcZoom(m_aSizingRect.GetSize().Height()));
            m_pSizingWin->SetPosSizePixel( m_aSizingRect.TopLeft(), aNewSize );
            TabWinSized(m_pSizingWin, ptOld, szOld);

            m_pSizingWin->Invalidate( m_aSizingRect );
            m_pSizingWin = NULL;
        }
    }
    else if (rTEvt.IsTrackingCanceled())
    {
        if (m_aDragScrollTimer.IsActive())
            m_aDragScrollTimer.Stop();
        EndTracking();
    }
    else
    {
        if( m_pDragWin )
        {
            m_ptPrevDraggingPos = rTEvt.GetMouseEvent().GetPosPixel();
            // an Fenstergrenzen scrollen
            ScrollWhileDragging();
        }

        if( m_pSizingWin )
        {
            Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();
            m_aSizingRect = m_pSizingWin->getSizingRect(aMousePos,m_aOutputSize);
            Update();
            ShowTracking( m_aSizingRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
        }
    }
}

//------------------------------------------------------------------------------
void OJoinTableView::ConnDoubleClicked( OTableConnection* pConnection )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
}

//------------------------------------------------------------------------------
void OJoinTableView::MouseButtonDown( const MouseEvent& rEvt )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    GrabFocus();
    Window::MouseButtonDown(rEvt);
}

//------------------------------------------------------------------------------
void OJoinTableView::MouseButtonUp( const MouseEvent& rEvt )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    Window::MouseButtonUp(rEvt);
    //////////////////////////////////////////////////////////////////////
    // Wurde eine Connection ausgewaehlt?
    if( !m_vTableConnection.empty() )
    {
        DeselectConn(GetSelectedConn());

        ::std::vector<OTableConnection*>::iterator aIter = m_vTableConnection.begin();
        for(;aIter != m_vTableConnection.end();++aIter)
        {
            if( (*aIter)->CheckHit(rEvt.GetPosPixel()) )
            {
                SelectConn((*aIter));

                // Doppelclick
                if( rEvt.GetClicks() == 2 )
                    ConnDoubleClicked( (*aIter) );

                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
void OJoinTableView::KeyInput( const KeyEvent& rEvt )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    USHORT nCode = rEvt.GetKeyCode().GetCode();
    BOOL   bShift = rEvt.GetKeyCode().IsShift();
    BOOL   bCtrl = rEvt.GetKeyCode().IsMod1();

    if( !bCtrl && !bShift && (nCode==KEY_DELETE) )
    {
        if (GetSelectedConn())
            RemoveConnection( GetSelectedConn() ,sal_True);
    }
    else
        Window::KeyInput( rEvt );
}

//------------------------------------------------------------------------------
void OJoinTableView::DeselectConn(OTableConnection* pConn)
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    if (!pConn || !pConn->IsSelected())
        return;

    // die zugehoerigen Eitnraege in der ListBox des Tabellenfenster deselektieren
    OTableWindow* pWin = pConn->GetSourceWin();
    if (pWin && pWin->GetListBox())
        pWin->GetListBox()->SelectAll(FALSE);

    pWin = pConn->GetDestWin();
    if (pWin && pWin->GetListBox())
        pWin->GetListBox()->SelectAll(FALSE);

    pConn->Deselect();
    m_pSelectedConn = NULL;
}

//------------------------------------------------------------------------------
void OJoinTableView::SelectConn(OTableConnection* pConn)
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    DeselectConn(GetSelectedConn());

    pConn->Select();
    m_pSelectedConn = pConn;
    GrabFocus(); // has to be called here because a table window may still be focused

    // die betroffenene Eintraege in den Windows selektieren
    OTableWindow* pConnSource = pConn->GetSourceWin();
    OTableWindow* pConnDest = pConn->GetDestWin();
    if (pConnSource && pConnDest)
    {
        OTableWindowListBox* pSourceBox = pConnSource->GetListBox();
        OTableWindowListBox* pDestBox = pConnDest->GetListBox();
        if (pSourceBox && pDestBox)
        {
            pSourceBox->SelectAll(FALSE);
            pDestBox->SelectAll(FALSE);

            SvLBoxEntry* pFirstSourceVisible = pSourceBox->GetFirstEntryInView();
            SvLBoxEntry* pFirstDestVisible = pDestBox->GetFirstEntryInView();

            const ::std::vector<OConnectionLine*>* pLines = pConn->GetConnLineList();
            ::std::vector<OConnectionLine*>::const_reverse_iterator aIter = pLines->rbegin();
            for(;aIter != pLines->rend();++aIter)
            {
                if ((*aIter)->IsValid())
                {
                    SvLBoxEntry* pSourceEntry = pSourceBox->GetEntryFromText((*aIter)->GetData()->GetSourceFieldName());
                    OSL_ENSURE(pSourceEntry,"Could not find column in Source ListBox!");
                    if (pSourceEntry)
                    {
                        pSourceBox->Select(pSourceEntry, TRUE);
                        pSourceBox->MakeVisible(pSourceEntry);
                    }

                    SvLBoxEntry* pDestEntry = pDestBox->GetEntryFromText((*aIter)->GetData()->GetDestFieldName());
                    OSL_ENSURE(pDestEntry,"Could not find column in Dest ListBox!");
                    if (pDestEntry)
                    {
                        pDestBox->Select(pDestEntry, TRUE);
                        pDestBox->MakeVisible(pDestEntry);
                    }

                }
            }

            if ((pFirstSourceVisible != pSourceBox->GetFirstEntryInView())
                || (pFirstDestVisible != pDestBox->GetFirstEntryInView()))
                // es wurde gescrollt -> neu zeichnen
                Invalidate(INVALIDATE_NOCHILDREN);
        }
    }
}
//------------------------------------------------------------------------------
void OJoinTableView::Paint( const Rectangle& rRect )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    DrawConnections( rRect );
}

//------------------------------------------------------------------------------
void OJoinTableView::InvalidateConnections()
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    //////////////////////////////////////////////////////////////////////
    // Die Joins zeichnen
    ::std::for_each(m_vTableConnection.begin(),m_vTableConnection.end(),
        ::std::mem_fun(& OTableConnection::Invalidate));
}

//------------------------------------------------------------------------------
void OJoinTableView::DrawConnections( const Rectangle& rRect )
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    //////////////////////////////////////////////////////////////////////
    // Die Joins zeichnen
    ::std::vector<OTableConnection*>::iterator aIter = m_vTableConnection.begin();
    for(;aIter != m_vTableConnection.end();++aIter)
        (*aIter)->Draw( rRect );
    // zum Schluss noch mal die selektierte ueber alle anderen drueber
    if (GetSelectedConn())
        GetSelectedConn()->Draw( rRect );
}


//------------------------------------------------------------------------------
::std::vector<OTableConnection*>::const_iterator OJoinTableView::getTableConnections(const OTableWindow* _pFromWin) const
{
    return ::std::find_if(  m_vTableConnection.begin(),
                            m_vTableConnection.end(),
                            ::std::bind2nd(::std::mem_fun(&OTableConnection::isTableConnection),_pFromWin));
}
// -----------------------------------------------------------------------------
sal_Int32 OJoinTableView::getConnectionCount(const OTableWindow* _pFromWin) const
{
    return ::std::count_if( m_vTableConnection.begin(),
                            m_vTableConnection.end(),
                            ::std::bind2nd(::std::mem_fun(&OTableConnection::isTableConnection),_pFromWin));
}
//------------------------------------------------------------------------------
BOOL OJoinTableView::ExistsAConn(const OTableWindow* pFrom) const
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    return getTableConnections(pFrom) != m_vTableConnection.end();
}
//------------------------------------------------------------------------
void OJoinTableView::ClearAll()
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    SetUpdateMode(FALSE);

    HideTabWins();

    // und das selbe mit den Connections
    ::std::vector<OTableConnection*>::iterator aIter = m_vTableConnection.begin();
    for(;aIter != m_vTableConnection.end();++aIter)
        RemoveConnection( *aIter ,sal_True);
    m_vTableConnection.clear();

    m_pLastFocusTabWin  = NULL;
    m_pSelectedConn     = NULL;

    // scroll to the upper left
    Scroll(-GetScrollOffset().X(), TRUE, TRUE);
    Scroll(-GetScrollOffset().Y(), FALSE, TRUE);
    Invalidate();
}

//------------------------------------------------------------------------
BOOL OJoinTableView::ScrollWhileDragging()
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    DBG_ASSERT(m_pDragWin != NULL, "OJoinTableView::ScrollWhileDragging darf nur waehrend Dragging eines Fensters aufgerufen werden !");

    // den Timer schon mal killen
    if (m_aDragScrollTimer.IsActive())
        m_aDragScrollTimer.Stop();

    Point aDragWinPos = m_ptPrevDraggingPos - m_aDragOffset;
    Size aDragWinSize = m_pDragWin->GetSizePixel();
    Point aLowerRight(aDragWinPos.X() + aDragWinSize.Width(), aDragWinPos.Y() + aDragWinSize.Height());

    if (!m_bTrackingInitiallyMoved && (aDragWinPos == m_pDragWin->GetPosPixel()))
        return TRUE;

    // Darstellungsfehler vermeiden (wenn bei aktivem TrackingRect gescrollt wird)
    HideTracking();

    BOOL bScrolling = FALSE;
    BOOL bNeedScrollTimer = FALSE;

    // An Fenstergrenzen scrollen
    // TODO : nur dann abfangen, wenn das Fenster komplett verschwinden wuerde (nicht, solange noch ein Pixel sichtbar ist)
    if( aDragWinPos.X() < 5 )
    {
        bScrolling = Scroll( -LINE_SIZE, TRUE, TRUE );
        if( !bScrolling && (aDragWinPos.X()<0) )
            aDragWinPos.X() = 0;

        // brauche ich weiteres (timergesteuertes) Scrolling ?
        bNeedScrollTimer = bScrolling && (aDragWinPos.X() < 5);
    }

    if( aLowerRight.X() > m_aOutputSize.Width() - 5 )
    {
        bScrolling = Scroll( LINE_SIZE, TRUE, TRUE ) ;
        if( !bScrolling && ( aLowerRight.X() > m_aOutputSize.Width() ) )
            aDragWinPos.X() = m_aOutputSize.Width() - aDragWinSize.Width();

        // brauche ich weiteres (timergesteuertes) Scrolling ?
        bNeedScrollTimer = bScrolling && (aLowerRight.X() > m_aOutputSize.Width() - 5);
    }

    if( aDragWinPos.Y() < 5 )
    {
        bScrolling = Scroll( -LINE_SIZE, FALSE, TRUE );
        if( !bScrolling && (aDragWinPos.Y()<0) )
            aDragWinPos.Y() = 0;

        bNeedScrollTimer = bScrolling && (aDragWinPos.Y() < 5);
    }

    if( aLowerRight.Y() > m_aOutputSize.Height() - 5 )
    {
        bScrolling = Scroll( LINE_SIZE, FALSE, TRUE );
        if( !bScrolling && ( (aDragWinPos.Y() + aDragWinSize.Height()) > m_aOutputSize.Height() ) )
            aDragWinPos.Y() =  m_aOutputSize.Height() - aDragWinSize.Height();

        bNeedScrollTimer = bScrolling && (aLowerRight.Y() > m_aOutputSize.Height() - 5);
    }

    // Timer neu setzen, wenn noch notwendig
    if (bNeedScrollTimer)
    {
        m_aDragScrollTimer.SetTimeout(100);
        m_aDragScrollTimer.Start();
    }

    // das DraggingRect neu zeichnen
    m_aDragRect = Rectangle(m_ptPrevDraggingPos - m_aDragOffset, m_pDragWin->GetSizePixel());
    Update();
    ShowTracking( m_aDragRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );

    return bScrolling;
}

//------------------------------------------------------------------------
IMPL_LINK(OJoinTableView, OnDragScrollTimer, void*, EMPTYARG)
{
    ScrollWhileDragging();
    return 0L;
}
// -----------------------------------------------------------------------------
void OJoinTableView::invalidateAndModify(SfxUndoAction *_pAction)
{
    Invalidate(INVALIDATE_NOCHILDREN);
    m_pView->getController()->addUndoActionAndInvalidate(_pAction);
}
//------------------------------------------------------------------------
void OJoinTableView::TabWinMoved(OTableWindow* ptWhich, const Point& ptOldPosition)
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    Point ptThumbPos(GetHScrollBar()->GetThumbPos(), GetVScrollBar()->GetThumbPos());
    ptWhich->GetData()->SetPosition(ptWhich->GetPosPixel() + ptThumbPos);

    invalidateAndModify(new OJoinMoveTabWinUndoAct(this, ptOldPosition, ptWhich));
}

//------------------------------------------------------------------------
void OJoinTableView::TabWinSized(OTableWindow* ptWhich, const Point& ptOldPosition, const Size& szOldSize)
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    ptWhich->GetData()->SetSize(ptWhich->GetSizePixel());
    ptWhich->GetData()->SetPosition(ptWhich->GetPosPixel());

    invalidateAndModify(new OJoinSizeTabWinUndoAct(this, ptOldPosition, szOldSize, ptWhich));
}

//------------------------------------------------------------------------------
BOOL OJoinTableView::IsAddAllowed()
{
    DBG_CHKTHIS(OJoinTableView,NULL);

    // nicht wenn Db readonly
    if (m_pView->getController()->isReadOnly())
        return FALSE;

    try
    {
        Reference< XConnection> xConnection = m_pView->getController()->getConnection();
        if(!xConnection.is())
            return FALSE;
        // nicht wenn schon zuviele Tabellen
        Reference < XDatabaseMetaData > xMetaData( xConnection->getMetaData() );

        sal_Int32 nMax = xMetaData.is() ? xMetaData->getMaxTablesInSelect() : 0;
        if (nMax && nMax <= (sal_Int32)m_aTableMap.size())
            return FALSE;
    }
    catch(SQLException&)
    {
        return FALSE;
    }

    // nicht wenn keine Joins moeglich
//  if (!GetDatabase()->IsCapable(SDB_CAP_JOIN) && nMax <= GetTabWinCount())
//      return FALSE;

    return TRUE;
}
// -----------------------------------------------------------------------------
void OJoinTableView::executePopup(const Point& _aPos,OTableConnection* _pSelConnection)
{
    PopupMenu aContextMenu( ModuleRes( RID_MENU_JOINVIEW_CONNECTION ) );
    switch (aContextMenu.Execute(this, _aPos))
    {
        case SID_DELETE:
            RemoveConnection( _pSelConnection ,sal_True);
            break;
        case ID_QUERY_EDIT_JOINCONNECTION:
            ConnDoubleClicked( _pSelConnection ); // is the same as double clicked
            break;
    }
}
//------------------------------------------------------------------------------
void OJoinTableView::Command(const CommandEvent& rEvt)
{
    DBG_CHKTHIS(OJoinTableView,NULL);

    BOOL bHandled = FALSE;

    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            if( m_vTableConnection.empty() )
                return;

            OTableConnection* pSelConnection = GetSelectedConn();
            // when it wasn't a mouse event use the selected connection
            if (!rEvt.IsMouseEvent())
            {
                if( pSelConnection )
                {
                    const ::std::vector<OConnectionLine*>* pLines = pSelConnection->GetConnLineList();
                    ::std::vector<OConnectionLine*>::const_iterator aIter = ::std::find_if(pLines->begin(),pLines->end(),::std::mem_fun(&OConnectionLine::IsValid));
                    if( aIter != pLines->end() )
                        executePopup((*aIter)->getMidPoint(),pSelConnection);
                }
            }
            else
            {
                DeselectConn(pSelConnection);

                const Point& aMousePos = rEvt.GetMousePosPixel();
                ::std::vector<OTableConnection*>::iterator aIter = m_vTableConnection.begin();
                for(;aIter != m_vTableConnection.end();++aIter)
                {
                    if( (*aIter)->CheckHit(aMousePos) )
                    {
                        SelectConn(*aIter);
                        if(!getDesignView()->getController()->isReadOnly() && getDesignView()->getController()->isConnected())
                            executePopup(rEvt.GetMousePosPixel(),*aIter);
                        break;
                    }
                }
            }
            bHandled = TRUE;
        }
    }
    if (!bHandled)
        Window::Command(rEvt);
}

//------------------------------------------------------------------------------
OTableConnection* OJoinTableView::GetTabConn(const OTableWindow* pLhs,const OTableWindow* pRhs,const OTableConnection* _rpFirstAfter) const
{
    OTableConnection* pConn = NULL;
    DBG_ASSERT(pRhs || pLhs, "OJoinTableView::GetTabConn : invalid args !");
        // only one NULL-arg allowed

    if ((!pLhs || pLhs->ExistsAConn()) && (!pRhs || pRhs->ExistsAConn()))
    {
        BOOL bFoundStart = _rpFirstAfter ? FALSE : TRUE;

        ::std::vector<OTableConnection*>::const_iterator aIter = m_vTableConnection.begin();
        for(;aIter != m_vTableConnection.end();++aIter)
        {
            OTableConnection* pData = *aIter;

            if  (   (   (pData->GetSourceWin() == pLhs)
                    &&  (   (pData->GetDestWin() == pRhs)
                        ||  (NULL == pRhs)
                        )
                    )
                ||  (   (pData->GetSourceWin() == pRhs)
                    &&  (   (pData->GetDestWin() == pLhs)
                        ||  (NULL == pLhs)
                        )
                    )
                )
            {
                if (bFoundStart)
                {
                    pConn = pData;
                    break;
                }

                if (!pConn)
                    // used as fallback : if there is no conn after _rpFirstAfter the first conn between the two tables
                    // will be used
                    pConn = pData;

                if (pData == _rpFirstAfter)
                    bFoundStart = TRUE;
            }
        }
    }
    return pConn;
}

//------------------------------------------------------------------------------
long OJoinTableView::PreNotify(NotifyEvent& rNEvt)
{
    BOOL bHandled = FALSE;
    switch (rNEvt.GetType())
    {
        case EVENT_COMMAND:
        {
            const CommandEvent* pCommand = rNEvt.GetCommandEvent();
            if (pCommand->GetCommand() == COMMAND_WHEEL)
            {
                const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
                if (pData->GetMode() == COMMAND_WHEEL_SCROLL)
                {
                    if (pData->GetDelta() > 0)
                        Scroll(-10 * pData->GetScrollLines(), pData->IsHorz(), TRUE);
                    else
                        Scroll(10 * pData->GetScrollLines(), pData->IsHorz(), TRUE);
                    bHandled = TRUE;
                }
            }
        }
        break;
        case EVENT_KEYINPUT:
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

                        BOOL bForward = !pKeyEvent->GetKeyCode().IsShift();
                        // is there an active tab win ?
                        OTableWindowMapIterator aIter = m_aTableMap.begin();
                        for(;aIter != m_aTableMap.end();++aIter)
                            if (aIter->second && aIter->second->HasChildPathFocus())
                                break;

                        OTableWindow* pNextWin = NULL;
                        OTableConnection* pNextConn = NULL;

                        if (aIter != m_aTableMap.end())
                        {   // there is a currently active tab win
                            // check if there is an "overflow" and we should select a conn instead of a win
                            if (!m_vTableConnection.empty())
                            {
                                if ((aIter->second == m_aTableMap.rbegin()->second) && bForward)
                                    // the last win is active and we're travelling forward -> select the first conn
                                    pNextConn = *m_vTableConnection.begin();
                                if ((aIter == m_aTableMap.begin()) && !bForward)
                                    // the first win is active an we're traveling backward -> select the last conn
                                    pNextConn = *m_vTableConnection.rbegin();
                            }

                            if (!pNextConn)
                            {
                                // no conn for any reason -> select the next or previous tab win
                                if(bForward)
                                {
                                    if ((aIter->second == m_aTableMap.rbegin()->second))
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
                            ::std::vector<OTableConnection*>::iterator aIter = m_vTableConnection.begin();
                            for(;aIter != m_vTableConnection.end();++aIter,++i)
                            {
                                if ((*aIter) == GetSelectedConn())
                                    break;
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
                                if (i < (sal_Int32)m_vTableConnection.size())
                                    // there is a currently active conn
                                    pNextConn = m_vTableConnection[(i + (bForward ? 1 : m_vTableConnection.size() - 1)) % m_vTableConnection.size()];
                                else
                                {   // no tab win selected, no conn selected
                                    if (!m_vTableConnection.empty())
                                        pNextConn = m_vTableConnection[bForward ? 0 : m_vTableConnection.size() - 1];
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
                                // neccessary : a conn may be selected even if a tab win has the focus, in this case
                                // the next travel would select the same conn again if we would not reset te focus ...
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
        case EVENT_GETFOCUS:
        {
            if (m_aTableMap.empty())
                // no tab wins -> no conns -> no focus change
                break;
            Window* pSource = rNEvt.GetWindow();
            if (pSource)
            {
                Window* pSearchFor = NULL;
                if (pSource->GetParent() == this)
                    // it may be one of the tab wins
                    pSearchFor = pSource;
                else if (pSource->GetParent() && (pSource->GetParent()->GetParent() == this))
                    // it may be one of th list boxes of one of the tab wins
                    pSearchFor = pSource->GetParent();

                if (pSearchFor)
                {
                    OTableWindowMapIterator aIter = m_aTableMap.begin();
                    for(;aIter != m_aTableMap.end();++aIter)
                    {
                        if (aIter->second == pSearchFor)
                        {
                            m_pLastFocusTabWin = aIter->second;
                            break;
                        }
                    }
                }
            }
        }
        break;
    }

    if (!bHandled)
        return Window::PreNotify(rNEvt);
    return 1L;
}

//------------------------------------------------------------------------------
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
        OTableWindow* pFirstWin = m_aTableMap.begin()->second;
        if (pFirstWin->GetListBox())
            pFirstWin->GetListBox()->GrabFocus();
        else
            pFirstWin->GrabFocus();
    }
}
// -----------------------------------------------------------------------------
void OJoinTableView::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_ZOOM )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        Font aFont = rStyleSettings.GetGroupFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );

        OTableWindowMapIterator aIter = m_aTableMap.begin();
        for(;aIter != m_aTableMap.end();++aIter)
        {
            aIter->second->SetZoom(GetZoom());
            Size aSize(CalcZoom(aIter->second->GetSizePixel().Width()),CalcZoom(aIter->second->GetSizePixel().Height()));
            aIter->second->SetSizePixel(aSize);
        }
        Resize();
    }
}
//------------------------------------------------------------------------------
void OJoinTableView::HideTabWins()
{
    DBG_CHKTHIS(OJoinTableView,NULL);
    SetUpdateMode(sal_False);

    OTableWindowMap* pTabWins = GetTabWinMap();
    if ( pTabWins )
    {
        OTableWindowMap::iterator aIter = pTabWins->begin();
        while(aIter != pTabWins->end())
        {
            OTableWindow* pTabWin = aIter->second;
            ++aIter;
            RemoveTabWin(pTabWin);
        }
    }

    m_pView->getController()->setModified(sal_True);

    SetUpdateMode(sal_True);

}
// -----------------------------------------------------------------------------
sal_Int8 OJoinTableView::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    return DND_ACTION_NONE;
}
// -----------------------------------------------------------------------------
sal_Int8 OJoinTableView::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    return DND_ACTION_NONE;
}
// -----------------------------------------------------------------------------
void OJoinTableView::dragFinished( )
{
}
//------------------------------------------------------------------------------
void OJoinTableView::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
}
// -----------------------------------------------------------------------------
void OJoinTableView::clearLayoutInformation()
{
    sal_Int32 nOldCount = m_aTableMap.size() + m_vTableConnection.size();
    //////////////////////////////////////////////////////////////////////
    // Listen loeschen
    OTableWindowMapIterator aIter = m_aTableMap.begin();
    OTableWindowMapIterator aEnd  = m_aTableMap.end();
    for(;aIter != aEnd;++aIter)
    {
        if ( aIter->second )
            aIter->second->clearListBox();
        ::std::auto_ptr<Window> aTemp(aIter->second);
        aIter->second = NULL;
    }

    m_aTableMap.clear();

    ::std::vector<OTableConnection*>::const_iterator aIter2 = m_vTableConnection.begin();
    for(;aIter2 != m_vTableConnection.end();++aIter2)
        delete *aIter2;

    m_vTableConnection.clear();
}
// -----------------------------------------------------------------------------
void OJoinTableView::lookForUiActivities()
{
}
// -----------------------------------------------------------------------------
void OJoinTableView::LoseFocus()
{
    DeselectConn(GetSelectedConn());
    Window::LoseFocus();
}
// -----------------------------------------------------------------------------
void OJoinTableView::GetFocus()
{
    Window::GetFocus();
    if ( !m_aTableMap.empty() && !GetSelectedConn() )
        GrabTabWinFocus();
}
// -----------------------------------------------------------------------------
Reference< XAccessible > OJoinTableView::CreateAccessible()
{
    Reference< XAccessible > aRet;
    if (getDesignView()->getController())
    {
        // create our VIEWPORT
        OJoinDesignViewAccess* pAccessible = new OJoinDesignViewAccess(this);
        m_pAccessible = pAccessible;
        aRet = pAccessible;
    }
    else
        aRet = Window::CreateAccessible();
    return aRet;
}
// -----------------------------------------------------------------------------
void OJoinTableView::modified()
{
    OJoinController* pController = m_pView->getController();
    pController->setModified( sal_True );
    pController->InvalidateFeature(ID_BROWSER_ADDTABLE);
    pController->InvalidateFeature(ID_RELATION_ADD_RELATION);
}
// -----------------------------------------------------------------------------
void OJoinTableView::addConnection(OTableConnection* _pConnection,sal_Bool _bAddData)
{
    if ( _bAddData )
    {
#if OSL_DEBUG_LEVEL > 0
        ::std::vector< OTableConnectionData*>* pTabConnDataList = m_pView->getController()->getTableConnectionData();
        OSL_ENSURE( ::std::find(pTabConnDataList->begin(),pTabConnDataList->end(),_pConnection->GetData()) == pTabConnDataList->end(),"Data already in vector!");
#endif
        m_pView->getController()->getTableConnectionData()->push_back(_pConnection->GetData());
    }
    m_vTableConnection.push_back(_pConnection);
    _pConnection->RecalcLines();
    _pConnection->Invalidate();

    modified();
    if ( m_pAccessible )
        m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                Any(),
                                                makeAny(_pConnection->GetAccessible()));
}
// -----------------------------------------------------------------------------



