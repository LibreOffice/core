/*************************************************************************
 *
 *  $RCSfile: TableWindowListBox.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-30 13:06:46 $
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
#ifndef DBAUI_TABLEWINDOWLISTBOX_HXX
#include "TableWindowListBox.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef DBAUI_JOINEXCHANGE_HXX
#include "JoinExchange.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _SVX_DBEXCH_HRC
#include <svx/dbexch.hrc>
#endif
#ifndef _DTRANS_HXX
#include <so3/dtrans.hxx>
#endif
#ifndef _SV_EXCHANGE_HXX
#include <vcl/exchange.hxx>
#endif


using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;

const ULONG SCROLLING_TIMESPAN = 500;
const long LISTBOX_SCROLLING_AREA = 6;
//==================================================================
// class OTableWindowListBox
//==================================================================
DBG_NAME(OTableWindowListBox);
//------------------------------------------------------------------------------
OTableWindowListBox::OTableWindowListBox( OTableWindow* pParent, const String& rDatabaseName, const String& rTableName ) :
     SvTreeListBox( pParent, WB_HASBUTTONS | WB_BORDER)
#if SUPD<627
    ,DragSourceHelper(this)
    ,DropTargetHelper(this)
#endif
    ,m_pTabWin( pParent )
    ,m_aMousePos( Point(0,0) )
    ,m_bReallyScrolled( sal_False )
    ,m_bDragSource( sal_False )
{
    DBG_CTOR(OTableWindowListBox,NULL);
    EnableDrop();
    m_aScrollTimer.SetTimeout( SCROLLING_TIMESPAN );
    SetDoubleClickHdl( LINK(this, OTableWindowListBox, DoubleClickHdl) );

    SetHighlightRange( );
}

//------------------------------------------------------------------------------
void OTableWindowListBox::dragFinished( )
{
    m_bDragSource = sal_False;
}

//------------------------------------------------------------------------------
OTableWindowListBox::~OTableWindowListBox()
{
    if( m_aScrollTimer.IsActive() )
        m_aScrollTimer.Stop();
    DBG_DTOR(OTableWindowListBox,NULL);
}

//------------------------------------------------------------------------------
SvLBoxEntry* OTableWindowListBox::GetEntryFromText( const String& rEntryText )
{
    //////////////////////////////////////////////////////////////////////
    // Liste durchiterieren
    SvTreeList* pTreeList = GetModel();
    SvLBoxEntry* pEntry = (SvLBoxEntry*)pTreeList->First();
    OJoinDesignView* pView = m_pTabWin->getDesignView();
    OJoinController* pController = pView->getController();
    Reference<XDatabaseMetaData> xMeta = pController->getConnection()->getMetaData();

    BOOL bCase = xMeta->storesMixedCaseQuotedIdentifiers();

    while( pEntry )
    {
        if((bCase ? rEntryText == GetEntryText(pEntry) : rEntryText.EqualsIgnoreCaseAscii(GetEntryText(pEntry))))
            return pEntry;
        pEntry = (SvLBoxEntry*)pTreeList->Next( pEntry );
    }

    return NULL;
}

//------------------------------------------------------------------------------
void OTableWindowListBox::NotifyScrolled()
{
    m_bReallyScrolled = TRUE;
}

//------------------------------------------------------------------------------
void OTableWindowListBox::NotifyEndScroll()
{
    if (m_bReallyScrolled)
        // die Verbindungen, die diese Tabelle eventuell hat, muessen neu gezeichnet werden
        static_cast<OQueryTableView*>(m_pTabWin->getTableView())->Invalidate(INVALIDATE_NOCHILDREN);
        // ohne das INVALIDATE_NOCHILDREN wuerden auch alle Tabellen neu gezeichnet werden,
        // sprich : es flackert
    m_bReallyScrolled = FALSE;
}

//------------------------------------------------------------------------------
long OTableWindowListBox::PreNotify(NotifyEvent& rNEvt)
{
    BOOL bHandled = FALSE;
    switch (rNEvt.GetType())
    {
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            const KeyCode& rCode = pKeyEvent->GetKeyCode();
            if (rCode.IsMod1() || rCode.IsMod2() || rCode.IsShift())
                break;
            if (rCode.GetCode() != KEY_RETURN)
                break;

            if (FirstSelected())
                static_cast<OTableWindow*>(Window::GetParent())->OnEntryDoubleClicked(FirstSelected());
        }
        break;
    }

    if (!bHandled)
        return SvTreeListBox::PreNotify(rNEvt);
    return 1L;
}

//------------------------------------------------------------------------------
IMPL_LINK( OTableWindowListBox, ScrollUpHdl, SvTreeListBox*, pBox )
{
    SvLBoxEntry* pEntry = GetEntry( m_aMousePos );
    if( !pEntry )
        return 0;

    if( pEntry != Last() )
    {
        ScrollOutputArea( -1 );
        pEntry = GetEntry( m_aMousePos );
        Select( pEntry, TRUE );
//      m_aScrollTimer.Start();
    }

    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK( OTableWindowListBox, ScrollDownHdl, SvTreeListBox*, pBox )
{
    SvLBoxEntry* pEntry = GetEntry( m_aMousePos );
    if( !pEntry )
        return 0;

    if( pEntry != Last() )
    {
        ScrollOutputArea( 1 );
        pEntry = GetEntry( m_aMousePos );
        Select( pEntry, TRUE );
//      m_aScrollTimer.Start();
    }

    return 0;
}

//------------------------------------------------------------------------------
void OTableWindowListBox::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    OQueryTableView* pCont = static_cast<OQueryTableView*>(m_pTabWin->getTableView());
    if (!pCont->getDesignView()->getController()->isReadOnly())
    {
        EndSelection();

        // eine Beschreibung der Source
        OJoinExchangeData jxdSource(this);

        // in ein Exchange-Objekt packen
        OJoinExchObj* pJoin = new OJoinExchObj(jxdSource);
        Reference< XTransferable > xEnsureDelete(pJoin);

        m_bDragSource = sal_True;
        pJoin->StartDrag(this, DND_ACTION_LINK, this);
    }
}

//------------------------------------------------------------------------------
sal_Int8 OTableWindowListBox::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    // check the format
    if (!OJoinExchObj::isFormatAvailable(GetDataFlavorExVector()))
        return DND_ACTION_NONE;

    // don't drop into the window if it's the drag source itself
    if (m_bDragSource)
        return DND_ACTION_NONE;

    // remove the selection if the dragging operation is leaving the window
    if (_rEvt.mbLeaving)
    {
        SelectAll(FALSE);
        return DND_ACTION_NONE;
    }

/*
    // Wenn der erste Eintrag der Quelle (*) gedraggt wird, lehne ich grundsaetzlich ab
    // TODO there isn't a exchange object yet

    OJoinExchangeData jxdSource = ((OJoinExchObj*)&xDataObj)->GetSourceDescription();
    if (jxdSource.pListBox->GetTabWin()->GetData()->IsShowAll() && (jxdSource.pListBox->First() == jxdSource.pEntry))
        return FALSE;
*/

    // hit test
    m_aMousePos = _rEvt.maPosPixel;
    Size aOutputSize = GetOutputSizePixel();
    SvLBoxEntry* pEntry = GetEntry( m_aMousePos );
    if( !pEntry )
        return DND_ACTION_NONE;

    // Scrolling Areas
    Rectangle aBottomScrollArea( Point(0, aOutputSize.Height()-LISTBOX_SCROLLING_AREA),
                                 Size(aOutputSize.Width(), LISTBOX_SCROLLING_AREA) );
    Rectangle aTopScrollArea( Point(0,0), Size(aOutputSize.Width(), LISTBOX_SCROLLING_AREA) );

    // Wenn Zeiger auf der oberen ScrollingArea steht, nach oben scrollen
    if( aBottomScrollArea.IsInside(m_aMousePos) )
    {
        if( !m_aScrollTimer.IsActive() )
        {
            m_aScrollTimer.SetTimeoutHdl( LINK(this, OTableWindowListBox, ScrollUpHdl) );
            ScrollUpHdl( this );
        }
    }

    // Wenn Zeiger auf der oberen ScrollingArea steht, nach unten scrollen
    else if( aTopScrollArea.IsInside(m_aMousePos) )
    {
        if( !m_aScrollTimer.IsActive() )
        {
            m_aScrollTimer.SetTimeoutHdl( LINK(this, OTableWindowListBox, ScrollDownHdl) );
            ScrollDownHdl( this );
        }
    }
    else
    {
        if( m_aScrollTimer.IsActive() )
            m_aScrollTimer.Stop();
    }

    // Beim Drag automatisch den richtigen Eintrag selektieren
    if ((FirstSelected() != pEntry) || (FirstSelected() && NextSelected(FirstSelected())))
        SelectAll(FALSE);
    Select(pEntry, TRUE);

    // Auf den ersten Eintrag (*) kann nicht gedroppt werden
    if( m_pTabWin->GetData()->IsShowAll() && (pEntry==First()) )
        return DND_ACTION_NONE;

    return DND_ACTION_LINK;
}

//------------------------------------------------------------------------------
sal_Int8 OTableWindowListBox::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    TransferableDataHelper aDropped(_rEvt.maDropEvent.Transferable);
    if (!OJoinExchObj::isFormatAvailable(aDropped.GetDataFlavorExVector()))
    {
        DBG_ERROR("OTableWindowListBox::ExecuteDrop: this should never have passed AcceptDrop!");
        return DND_ACTION_NONE;
    }

    // target window description
    OJoinExchangeData jxdDest(this);

    // source window description
    OJoinExchangeData jxdSource = OJoinExchObj::GetSourceDescription(_rEvt.maDropEvent.Transferable);

    // create the connection
    OQueryTableView* pCont = static_cast<OQueryTableView*>(m_pTabWin->getTableView());
    pCont->AddConnection(jxdSource, jxdDest);

    return DND_ACTION_LINK;
}

//------------------------------------------------------------------------------
void OTableWindowListBox::LoseFocus()
{
    m_pTabWin->LoseFocus();
    SvTreeListBox::LoseFocus();
}

//------------------------------------------------------------------------------
void OTableWindowListBox::GetFocus()
{
    m_pTabWin->GetFocus();
    SvTreeListBox::GetFocus();
    if (GetCurEntry() != NULL)
        if (GetSelectionCount() == 0)
            Select(GetCurEntry(), TRUE);
        else
            ShowFocusRect(FirstSelected());
}

//------------------------------------------------------------------------------
IMPL_LINK( OTableWindowListBox, DoubleClickHdl, SvTreeListBox *, pBox )
{
    // meinem Elter Bescheid sagen
    Window* pParent = Window::GetParent();
    DBG_ASSERT(pParent != NULL, "OTableWindowListBox::DoubleClickHdl : habe kein Parent !");

    static_cast<OTableWindow*>(pParent)->OnEntryDoubleClicked(GetHdlEntry());

    return 0;
}