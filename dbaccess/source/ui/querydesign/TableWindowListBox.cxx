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

#include "TableWindowListBox.hxx"
#include "TableWindow.hxx"
#include "QueryDesignView.hxx"
#include "QueryTableView.hxx"
#include "querycontroller.hxx"
#include "JoinExchange.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <vcl/svapp.hxx>

using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;

OJoinExchangeData::OJoinExchangeData(OTableWindowListBox* pBox)
    : pListBox(pBox)
    , pEntry(pBox->FirstSelected())
{ }

const sal_uLong SCROLLING_TIMESPAN = 500;
const long LISTBOX_SCROLLING_AREA = 6;
//==================================================================
// class OTableWindowListBox
//==================================================================
DBG_NAME(OTableWindowListBox)
//------------------------------------------------------------------------------
OTableWindowListBox::OTableWindowListBox( OTableWindow* pParent )
    :SvTreeListBox( pParent, WB_HASBUTTONS | WB_BORDER)
    ,m_aMousePos( Point(0,0) )
    ,m_pTabWin( pParent )
    ,m_nDropEvent(0)
    ,m_nUiEvent(0)
    ,m_bReallyScrolled( sal_False )
{
    DBG_CTOR(OTableWindowListBox,NULL);
    m_aScrollTimer.SetTimeout( SCROLLING_TIMESPAN );
    SetDoubleClickHdl( LINK(this, OTableWindowListBox, OnDoubleClick) );

    SetSelectionMode(SINGLE_SELECTION);

    SetHighlightRange( );
}

//------------------------------------------------------------------------------
void OTableWindowListBox::dragFinished( )
{
    // first show the error msg when existing
    m_pTabWin->getDesignView()->getController().showError(m_pTabWin->getDesignView()->getController().clearOccurredError());
    // second look for ui activities which should happen after d&d
    if (m_nUiEvent)
        Application::RemoveUserEvent(m_nUiEvent);
    m_nUiEvent = Application::PostUserEvent(LINK(this, OTableWindowListBox, LookForUiHdl));
}

//------------------------------------------------------------------------------
OTableWindowListBox::~OTableWindowListBox()
{
    DBG_DTOR(OTableWindowListBox,NULL);
    if (m_nDropEvent)
        Application::RemoveUserEvent(m_nDropEvent);
    if (m_nUiEvent)
        Application::RemoveUserEvent(m_nUiEvent);
    if( m_aScrollTimer.IsActive() )
        m_aScrollTimer.Stop();
    m_pTabWin = NULL;
}

//------------------------------------------------------------------------------
SvLBoxEntry* OTableWindowListBox::GetEntryFromText( const String& rEntryText )
{
    //////////////////////////////////////////////////////////////////////
    // Liste durchiterieren
    SvLBoxTreeList* pTreeList = GetModel();
    SvLBoxEntry* pEntry = pTreeList->First();
    OJoinDesignView* pView = m_pTabWin->getDesignView();
    OJoinController& rController = pView->getController();

    sal_Bool bCase = sal_False;
    try
    {
        Reference<XConnection> xConnection = rController.getConnection();
        if(xConnection.is())
        {
            Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
            if(xMeta.is())
                bCase = xMeta->supportsMixedCaseQuotedIdentifiers();
        }
        while( pEntry )
        {
            if((bCase ? rEntryText == GetEntryText(pEntry) : rEntryText.EqualsIgnoreCaseAscii(GetEntryText(pEntry))))
            {
                return pEntry;
            }
            pEntry = pTreeList->Next(pEntry);
        }
    }
    catch(SQLException&)
    {
    }

    return NULL;
}

//------------------------------------------------------------------------------
void OTableWindowListBox::NotifyScrolled()
{
    m_bReallyScrolled = sal_True;
}

//------------------------------------------------------------------------------
void OTableWindowListBox::NotifyEndScroll()
{
    if (m_bReallyScrolled)
        // die Verbindungen, die diese Tabelle eventuell hat, muessen neu gezeichnet werden
        m_pTabWin->getTableView()->Invalidate(INVALIDATE_NOCHILDREN);
        // ohne das INVALIDATE_NOCHILDREN wuerden auch alle Tabellen neu gezeichnet werden,
        // sprich : es flackert
    m_bReallyScrolled = sal_False;
}

//------------------------------------------------------------------------------
long OTableWindowListBox::PreNotify(NotifyEvent& rNEvt)
{
    sal_Bool bHandled = sal_False;
    switch (rNEvt.GetType())
    {
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            const KeyCode& rCode = pKeyEvent->GetKeyCode();

            if (rCode.GetCode() != KEY_RETURN)
            {
                if(m_pTabWin)
                {
                    bHandled = m_pTabWin->HandleKeyInput(*pKeyEvent);
                }
                break;
            }

            if (rCode.IsMod1() || rCode.IsMod2() || rCode.IsShift())
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
IMPL_LINK( OTableWindowListBox, ScrollUpHdl, SvTreeListBox*, /*pBox*/ )
{
    SvLBoxEntry* pEntry = GetEntry( m_aMousePos );
    if( !pEntry )
        return 0;

    if( pEntry != Last() )
    {
        ScrollOutputArea( -1 );
        pEntry = GetEntry( m_aMousePos );
        Select( pEntry, sal_True );
    }

    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK( OTableWindowListBox, ScrollDownHdl, SvTreeListBox*, /*pBox*/ )
{
    SvLBoxEntry* pEntry = GetEntry( m_aMousePos );
    if( !pEntry )
        return 0;

    if( pEntry != Last() )
    {
        ScrollOutputArea( 1 );
        pEntry = GetEntry( m_aMousePos );
        Select( pEntry, sal_True );
    }

    return 0;
}

//------------------------------------------------------------------------------
void OTableWindowListBox::StartDrag( sal_Int8 /*nAction*/, const Point& /*rPosPixel*/ )
{
    OJoinTableView* pCont = m_pTabWin->getTableView();
    if (!pCont->getDesignView()->getController().isReadOnly() && pCont->getDesignView()->getController().isConnected())
    {
        // asterix was not allowed to be copied to selection browsebox
        sal_Bool bFirstNotAllowed = FirstSelected() == First() && m_pTabWin->GetData()->IsShowAll();
        EndSelection();
        // create a description of the source
        OJoinExchangeData jxdSource(this);
        // put it into a exchange object
        OJoinExchObj* pJoin = new OJoinExchObj(jxdSource,bFirstNotAllowed);
        Reference< XTransferable > xEnsureDelete(pJoin);
        pJoin->StartDrag(this, DND_ACTION_LINK, this);
    }
}

//------------------------------------------------------------------------------
sal_Int8 OTableWindowListBox::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    sal_Int8 nDND_Action = DND_ACTION_NONE;
    // check the format
    if ( !OJoinExchObj::isFormatAvailable(GetDataFlavorExVector(),SOT_FORMATSTR_ID_SBA_TABID) // this means that the first entry is to be draged
        && OJoinExchObj::isFormatAvailable(GetDataFlavorExVector(),SOT_FORMATSTR_ID_SBA_JOIN) )
    {   // don't drop into the window if it's the drag source itself

        // remove the selection if the dragging operation is leaving the window
        if (_rEvt.mbLeaving)
            SelectAll(sal_False);
        else
        {
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
                SelectAll(sal_False);
            Select(pEntry, sal_True);

            // Auf den ersten Eintrag (*) kann nicht gedroppt werden
            if(!( m_pTabWin->GetData()->IsShowAll() && (pEntry==First()) ))
                nDND_Action = DND_ACTION_LINK;
        }
    }
    return nDND_Action;
}
// -----------------------------------------------------------------------------

//------------------------------------------------------------------------------
IMPL_LINK( OTableWindowListBox, LookForUiHdl, void *, /*EMPTY_ARG*/)
{
    m_nUiEvent = 0;
    m_pTabWin->getTableView()->lookForUiActivities();
    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK( OTableWindowListBox, DropHdl, void *, /*EMPTY_ARG*/)
{
    // create the connection
    m_nDropEvent = 0;
    OSL_ENSURE(m_pTabWin,"No TableWindow!");
    try
    {
        OJoinTableView* pCont = m_pTabWin->getTableView();
        OSL_ENSURE(pCont,"No QueryTableView!");
        pCont->AddConnection(m_aDropInfo.aSource, m_aDropInfo.aDest);
    }
    catch(const SQLException& e)
    {
        // remember the exception so that we can show them later when d&d is finished
        m_pTabWin->getDesignView()->getController().setErrorOccurred(::dbtools::SQLExceptionInfo(e));
    }
    return 0L;
}
//------------------------------------------------------------------------------
sal_Int8 OTableWindowListBox::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    TransferableDataHelper aDropped(_rEvt.maDropEvent.Transferable);
    if ( OJoinExchObj::isFormatAvailable(aDropped.GetDataFlavorExVector()))
    {   // don't drop into the window if it's the drag source itself
        m_aDropInfo.aSource = OJoinExchangeData(this);
        m_aDropInfo.aDest   = OJoinExchObj::GetSourceDescription(_rEvt.maDropEvent.Transferable);

        if (m_nDropEvent)
            Application::RemoveUserEvent(m_nDropEvent);
        m_nDropEvent = Application::PostUserEvent(LINK(this, OTableWindowListBox, DropHdl));

        return DND_ACTION_LINK;
    }
    return DND_ACTION_NONE;
}

//------------------------------------------------------------------------------
void OTableWindowListBox::LoseFocus()
{
    if(m_pTabWin)
        m_pTabWin->setActive(sal_False);
    SvTreeListBox::LoseFocus();
}

//------------------------------------------------------------------------------
void OTableWindowListBox::GetFocus()
{
    if(m_pTabWin)
        m_pTabWin->setActive();

    if (GetCurEntry() != NULL)
    {
        if ( GetSelectionCount() == 0 || GetCurEntry() != FirstSelected() )
        {
            if ( FirstSelected() )
                Select(FirstSelected(), sal_False);
            Select(GetCurEntry(), sal_True);
        }
        else
            ShowFocusRect(FirstSelected());
    }
    SvTreeListBox::GetFocus();
}

//------------------------------------------------------------------------------
IMPL_LINK( OTableWindowListBox, OnDoubleClick, SvTreeListBox *, /*pBox*/ )
{
    // meinem Elter Bescheid sagen
    Window* pParent = Window::GetParent();
    OSL_ENSURE(pParent != NULL, "OTableWindowListBox::OnDoubleClick : habe kein Parent !");

    static_cast<OTableWindow*>(pParent)->OnEntryDoubleClicked(GetHdlEntry());

    return 0;
}
// -----------------------------------------------------------------------------
void OTableWindowListBox::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            static_cast<OTableWindow*>(Window::GetParent())->Command(rEvt);
            break;
        }
        default:
            SvTreeListBox::Command(rEvt);
    }
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
