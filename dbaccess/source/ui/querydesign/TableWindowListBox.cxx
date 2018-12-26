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

#include <TableWindowListBox.hxx>
#include <TableWindow.hxx>
#include <QueryDesignView.hxx>
#include <QueryTableView.hxx>
#include <querycontroller.hxx>
#include <JoinExchange.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <vcl/svapp.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>

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
// class OTableWindowListBox
OTableWindowListBox::OTableWindowListBox( OTableWindow* pParent )
    :SvTreeListBox( pParent, WB_HASBUTTONS | WB_BORDER)
    ,m_aMousePos( Point(0,0) )
    ,m_pTabWin( pParent )
    ,m_nDropEvent(nullptr)
    ,m_nUiEvent(nullptr)
    ,m_bReallyScrolled( false )
{
    m_aScrollTimer.SetTimeout( SCROLLING_TIMESPAN );
    SetDoubleClickHdl( LINK(this, OTableWindowListBox, OnDoubleClick) );

    SetSelectionMode(SelectionMode::Single);

    SetHighlightRange( );
}

void OTableWindowListBox::dragFinished( )
{
    // first show the error msg when existing
    m_pTabWin->getDesignView()->getController().showError(m_pTabWin->getDesignView()->getController().clearOccurredError());
    // second look for ui activities which should happen after d&d
    if (m_nUiEvent)
        Application::RemoveUserEvent(m_nUiEvent);
    m_nUiEvent = Application::PostUserEvent(LINK(this, OTableWindowListBox, LookForUiHdl), nullptr, true);
}

OTableWindowListBox::~OTableWindowListBox()
{
    disposeOnce();
}

void OTableWindowListBox::dispose()
{
    if (m_nDropEvent)
        Application::RemoveUserEvent(m_nDropEvent);
    if (m_nUiEvent)
        Application::RemoveUserEvent(m_nUiEvent);
    if( m_aScrollTimer.IsActive() )
        m_aScrollTimer.Stop();
    m_pTabWin.clear();
    SvTreeListBox::dispose();
}

SvTreeListEntry* OTableWindowListBox::GetEntryFromText( const OUString& rEntryText )
{
    // iterate through the list
    SvTreeList* pTreeList = GetModel();
    SvTreeListEntry* pEntry = pTreeList->First();
    OJoinDesignView* pView = m_pTabWin->getDesignView();
    OJoinController& rController = pView->getController();

    try
    {
        bool bCase = false;
        const Reference<XConnection>& xConnection = rController.getConnection();
        if(xConnection.is())
        {
            Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
            if(xMeta.is())
                bCase = xMeta->supportsMixedCaseQuotedIdentifiers();
        }
        while( pEntry )
        {
            if(bCase ? rEntryText == GetEntryText(pEntry) : rEntryText.equalsIgnoreAsciiCase(GetEntryText(pEntry)))
            {
                return pEntry;
            }
            pEntry = pTreeList->Next(pEntry);
        }
    }
    catch(SQLException&)
    {
    }

    return nullptr;
}

void OTableWindowListBox::NotifyScrolled()
{
    m_bReallyScrolled = true;
}

void OTableWindowListBox::NotifyEndScroll()
{
    if (m_bReallyScrolled)
        // connections of this table, if any,  should be redrawn
        m_pTabWin->getTableView()->Invalidate(InvalidateFlags::NoChildren);

        // without InvalidateFlags::NoChildren all tables would be redrawn,
        // so: flickering
    m_bReallyScrolled = false;
}

bool OTableWindowListBox::PreNotify(NotifyEvent& rNEvt)
{
    bool bHandled = false;
    switch (rNEvt.GetType())
    {
        case MouseNotifyEvent::KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            const vcl::KeyCode& rCode = pKeyEvent->GetKeyCode();

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
            break;
        }
        default:
            break;
    }

    if (!bHandled)
        return SvTreeListBox::PreNotify(rNEvt);
    return true;
}

IMPL_LINK_NOARG( OTableWindowListBox, ScrollUpHdl, Timer*, void )
{
    SvTreeListEntry* pEntry = GetEntry( m_aMousePos );
    if( !pEntry )
        return;

    if( pEntry != Last() )
    {
        ScrollOutputArea( -1 );
        pEntry = GetEntry( m_aMousePos );
        Select( pEntry );
    }
}

IMPL_LINK_NOARG( OTableWindowListBox, ScrollDownHdl, Timer*, void )
{
    SvTreeListEntry* pEntry = GetEntry( m_aMousePos );
    if( !pEntry )
        return;

    if( pEntry != Last() )
    {
        ScrollOutputArea( 1 );
        pEntry = GetEntry( m_aMousePos );
        Select( pEntry );
    }
}

void OTableWindowListBox::StartDrag( sal_Int8 /*nAction*/, const Point& /*rPosPixel*/ )
{
    OJoinTableView* pCont = m_pTabWin->getTableView();
    if (!pCont->getDesignView()->getController().isReadOnly() && pCont->getDesignView()->getController().isConnected())
    {
        // asterisk was not allowed to be copied to selection browsebox
        bool bFirstNotAllowed = FirstSelected() == First() && m_pTabWin->GetData()->IsShowAll();
        EndSelection();
        // create a description of the source
        OJoinExchangeData jxdSource(this);
        // put it into a exchange object
        rtl::Reference<OJoinExchObj> pJoin = new OJoinExchObj(jxdSource,bFirstNotAllowed);
        pJoin->StartDrag(this, DND_ACTION_LINK, this);
    }
}

sal_Int8 OTableWindowListBox::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    sal_Int8 nDND_Action = DND_ACTION_NONE;
    // check the format
    if ( !OJoinExchObj::isFormatAvailable(GetDataFlavorExVector(),SotClipboardFormatId::SBA_TABID) // this means that the first entry is to be draged
        && OJoinExchObj::isFormatAvailable(GetDataFlavorExVector()) )
    {   // don't drop into the window if it's the drag source itself

        // remove the selection if the dragging operation is leaving the window
        if (_rEvt.mbLeaving)
            SelectAll(false);
        else
        {
            // hit test
            m_aMousePos = _rEvt.maPosPixel;
            Size aOutputSize = GetOutputSizePixel();
            SvTreeListEntry* pEntry = GetEntry( m_aMousePos );
            if( !pEntry )
                return DND_ACTION_NONE;

            // Scrolling Areas
            tools::Rectangle aBottomScrollArea( Point(0, aOutputSize.Height()-LISTBOX_SCROLLING_AREA),
                                         Size(aOutputSize.Width(), LISTBOX_SCROLLING_AREA) );
            tools::Rectangle aTopScrollArea( Point(0,0), Size(aOutputSize.Width(), LISTBOX_SCROLLING_AREA) );

             // scroll up if the pointer is on the upper scroll area
            if( aBottomScrollArea.IsInside(m_aMousePos) )
            {
                if( !m_aScrollTimer.IsActive() )
                {
                    m_aScrollTimer.SetInvokeHandler( LINK(this, OTableWindowListBox, ScrollUpHdl) );
                    ScrollUpHdl( nullptr );
                }
            }
            // scroll down if the pointer is on the lower scroll area
            else if( aTopScrollArea.IsInside(m_aMousePos) )
            {
                if( !m_aScrollTimer.IsActive() )
                {
                    m_aScrollTimer.SetInvokeHandler( LINK(this, OTableWindowListBox, ScrollDownHdl) );
                    ScrollDownHdl( nullptr );
                }
            }
            else
            {
                if( m_aScrollTimer.IsActive() )
                    m_aScrollTimer.Stop();
            }

            // automatically select right entry when dragging
            if ((FirstSelected() != pEntry) || NextSelected(FirstSelected()))
                SelectAll(false);
            Select(pEntry);

            // one cannot drop on the first (*) entry
            if(!( m_pTabWin->GetData()->IsShowAll() && (pEntry==First()) ))
                nDND_Action = DND_ACTION_LINK;
        }
    }
    return nDND_Action;
}

IMPL_LINK_NOARG( OTableWindowListBox, LookForUiHdl, void*, void )
{
    m_nUiEvent = nullptr;
    m_pTabWin->getTableView()->lookForUiActivities();
}

IMPL_LINK_NOARG( OTableWindowListBox, DropHdl, void*, void )
{
    // create the connection
    m_nDropEvent = nullptr;
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
}

sal_Int8 OTableWindowListBox::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    TransferableDataHelper aDropped(_rEvt.maDropEvent.Transferable);
    if ( OJoinExchObj::isFormatAvailable(aDropped.GetDataFlavorExVector()))
    {   // don't drop into the window if it's the drag source itself
        m_aDropInfo.aSource = OJoinExchangeData(this);
        m_aDropInfo.aDest   = OJoinExchObj::GetSourceDescription(_rEvt.maDropEvent.Transferable);

        if (m_nDropEvent)
            Application::RemoveUserEvent(m_nDropEvent);
        m_nDropEvent = Application::PostUserEvent(LINK(this, OTableWindowListBox, DropHdl), nullptr, true);

        return DND_ACTION_LINK;
    }
    return DND_ACTION_NONE;
}

void OTableWindowListBox::LoseFocus()
{
    if(m_pTabWin)
        m_pTabWin->setActive(false);
    SvTreeListBox::LoseFocus();
}

void OTableWindowListBox::GetFocus()
{
    if(m_pTabWin)
        m_pTabWin->setActive();

    if (GetCurEntry() != nullptr)
    {
        if ( GetSelectionCount() == 0 || GetCurEntry() != FirstSelected() )
        {
            if ( FirstSelected() )
                Select(FirstSelected(), false);
            Select(GetCurEntry());
        }
        else
            ShowFocusRect(FirstSelected());
    }
    SvTreeListBox::GetFocus();
}

IMPL_LINK_NOARG( OTableWindowListBox, OnDoubleClick, SvTreeListBox *, bool )
{
    // tell my parent
    vcl::Window* pParent = Window::GetParent();
    OSL_ENSURE(pParent != nullptr, "OTableWindowListBox::OnDoubleClick : have no Parent !");

    static_cast<OTableWindow*>(pParent)->OnEntryDoubleClicked(GetHdlEntry());

    return false;
}

void OTableWindowListBox::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            static_cast<OTableWindow*>(Window::GetParent())->Command(rEvt);
            break;
        }
        default:
            SvTreeListBox::Command(rEvt);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
