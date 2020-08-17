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
#include <JoinController.hxx>
#include <JoinExchange.hxx>
#include <JoinTableView.hxx>
#include <JoinDesignView.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <vcl/svapp.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>

using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;

OJoinExchangeData::OJoinExchangeData(OTableWindowListBox* pBox)
    : pListBox(pBox)
    , nEntry(pBox->get_widget().get_selected_index())
{
}

OTableWindowListBox::OTableWindowListBox( OTableWindow* pParent )
    : InterimItemWindow(pParent, "dbaccess/ui/tablelistbox.ui", "TableListBox")
    , m_xTreeView(m_xBuilder->weld_tree_view("treeview"))
    , m_xDragDropTargetHelper(new TableWindowListBoxHelper(*this, m_xTreeView->get_drop_target()))
    , m_pTabWin( pParent )
    , m_nDropEvent(nullptr)
    , m_nUiEvent(nullptr)
{
    m_xTreeView->connect_row_activated(LINK(this, OTableWindowListBox, OnDoubleClick));
    m_xTreeView->connect_visible_range_changed(LINK(this, OTableWindowListBox, ScrollHdl));

    m_xHelper.set(new OJoinExchObj);
    rtl::Reference<TransferDataContainer> xHelper(m_xHelper.get());
    m_xTreeView->enable_drag_source(xHelper, DND_ACTION_LINK);
    m_xTreeView->connect_drag_begin(LINK(this, OTableWindowListBox, DragBeginHdl));
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
    m_pTabWin.clear();
    m_xDragDropTargetHelper.reset();
    m_xTreeView.reset();
    InterimItemWindow::dispose();
}

int OTableWindowListBox::GetEntryFromText( const OUString& rEntryText )
{
    // iterate through the list
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
        for (int nEntry = 0, nCount = m_xTreeView->n_children(); nEntry < nCount; ++nEntry)
        {
            if (bCase ? rEntryText == m_xTreeView->get_text(nEntry) : rEntryText.equalsIgnoreAsciiCase(m_xTreeView->get_text(nEntry)))
                return nEntry;
        }
    }
    catch(SQLException&)
    {
    }

    return -1;
}

IMPL_LINK_NOARG(OTableWindowListBox, ScrollHdl, weld::TreeView&, void)
{
    // connections of this table, if any,  should be redrawn
    m_pTabWin->getTableView()->Invalidate(InvalidateFlags::NoChildren);
}

IMPL_LINK(OTableWindowListBox, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = false;
    if (m_xTreeView->get_selected_index() == -1)
    {
        // no drag without a field
        return true;
    }

    OJoinTableView* pCont = m_pTabWin->getTableView();
    if (!pCont->getDesignView()->getController().isReadOnly() && pCont->getDesignView()->getController().isConnected())
    {
        // asterisk was not allowed to be copied to selection browsebox
        bool bFirstNotAllowed = m_xTreeView->is_selected(0) && m_pTabWin->GetData()->IsShowAll();
        // create a description of the source
        OJoinExchangeData jxdSource(this);
        // update the exchange object
        m_xHelper->setDescriptors(jxdSource, bFirstNotAllowed);

        return false;
    }

    return true;
}

sal_Int8 OTableWindowListBox::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    // to enable the autoscroll when we're close to the edges
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    bool bHasDestRow = m_xTreeView->get_dest_row_at_pos(_rEvt.maPosPixel, xEntry.get(), true);

    sal_Int8 nDND_Action = DND_ACTION_NONE;
    // check the format
    if ( !OJoinExchObj::isFormatAvailable(m_xDragDropTargetHelper->GetDataFlavorExVector(),SotClipboardFormatId::SBA_TABID) // this means that the first entry is to be dragged
        && OJoinExchObj::isFormatAvailable(m_xDragDropTargetHelper->GetDataFlavorExVector()) )
    {   // don't drop into the window if it's the drag source itself

        // remove the selection if the dragging operation is leaving the window
        if (_rEvt.mbLeaving)
            m_xTreeView->unselect_all();
        else
        {
            if (!bHasDestRow)
                return DND_ACTION_NONE;

            // automatically select right entry when dragging
            m_xTreeView->unselect_all();
            m_xTreeView->select(*xEntry);

            // one cannot drop on the first (*) entry
            if(!( m_pTabWin->GetData()->IsShowAll() && (m_xTreeView->get_iter_index_in_parent(*xEntry) == 0) ))
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
    if (m_pTabWin)
        m_pTabWin->setActive(false);
    InterimItemWindow::LoseFocus();
}

void OTableWindowListBox::GetFocus()
{
    if (m_pTabWin)
        m_pTabWin->setActive();

    std::unique_ptr<weld::TreeIter> xCurrent = m_xTreeView->make_iterator();
    if (m_xTreeView->get_cursor(xCurrent.get()))
    {
        m_xTreeView->unselect_all();
        m_xTreeView->select(*xCurrent);
    }

    InterimItemWindow::GetFocus();
}

IMPL_LINK_NOARG(OTableWindowListBox, OnDoubleClick, weld::TreeView&, bool)
{
    // tell my parent
    vcl::Window* pParent = Window::GetParent();
    OSL_ENSURE(pParent != nullptr, "OTableWindowListBox::OnDoubleClick : have no Parent !");

    std::unique_ptr<weld::TreeIter> xCurrent = m_xTreeView->make_iterator();
    if (!m_xTreeView->get_cursor(xCurrent.get()))
        return false;

    static_cast<OTableWindow*>(pParent)->OnEntryDoubleClicked(*xCurrent);

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
            InterimItemWindow::Command(rEvt);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
