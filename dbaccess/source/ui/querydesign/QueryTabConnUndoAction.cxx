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

#include "QueryTabConnUndoAction.hxx"
#include "QTableConnection.hxx"
#include <tools/debug.hxx>
#include "QueryTableView.hxx"
#include "QueryAddTabConnUndoAction.hxx"
#include "QueryTabWinShowUndoAct.hxx"
#include "dbu_qry.hrc"

using namespace dbaui;

DBG_NAME(OQueryTabConnUndoAction)
OQueryTabConnUndoAction::~OQueryTabConnUndoAction()
{
    DBG_DTOR(OQueryTabConnUndoAction,NULL);
    if (m_bOwnerOfConn)
    {   // I have the connection -> delete
        m_pOwner->DeselectConn(m_pConnection);
        delete m_pConnection;
    }
}

OQueryTabConnUndoAction::OQueryTabConnUndoAction(OQueryTableView* pOwner, sal_uInt16 nCommentID)
    :OQueryDesignUndoAction(pOwner, nCommentID)
    ,m_pConnection(NULL)
    ,m_bOwnerOfConn(sal_False)
{
    DBG_CTOR(OQueryTabConnUndoAction,NULL);
}

OQueryAddTabConnUndoAction::OQueryAddTabConnUndoAction(OQueryTableView* pOwner)
    : OQueryTabConnUndoAction(pOwner, STR_QUERY_UNDO_INSERTCONNECTION)
{
}

void OQueryAddTabConnUndoAction::Undo()
{
    static_cast<OQueryTableView*>(m_pOwner)->DropConnection(m_pConnection);
    SetOwnership(sal_True);
}

void OQueryAddTabConnUndoAction::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->GetConnection(m_pConnection);
    SetOwnership(sal_False);
}

OQueryDelTabConnUndoAction::OQueryDelTabConnUndoAction(OQueryTableView* pOwner)
    : OQueryTabConnUndoAction(pOwner, STR_QUERY_UNDO_REMOVECONNECTION)
{
}

void OQueryDelTabConnUndoAction::Undo()
{
    static_cast<OQueryTableView*>(m_pOwner)->GetConnection(m_pConnection);
    SetOwnership(sal_False);
}

void OQueryDelTabConnUndoAction::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->DropConnection(m_pConnection);
    SetOwnership(sal_True);
}

OQueryTabWinShowUndoAct::OQueryTabWinShowUndoAct(OQueryTableView* pOwner)
 : OQueryTabWinUndoAct(pOwner, STR_QUERY_UNDO_TABWINSHOW)
{
}

OQueryTabWinShowUndoAct::~OQueryTabWinShowUndoAct()
{
}

void OQueryTabWinShowUndoAct::Undo()
{
    static_cast<OQueryTableView*>(m_pOwner)->HideTabWin(m_pTabWin, this);
    SetOwnership(sal_True);
}

void OQueryTabWinShowUndoAct::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->ShowTabWin(m_pTabWin, this,sal_True);
    SetOwnership(sal_False);
}

OQueryTabWinDelUndoAct::OQueryTabWinDelUndoAct(OQueryTableView* pOwner)
 : OQueryTabWinUndoAct(pOwner, STR_QUERY_UNDO_TABWINDELETE)
{
}

OQueryTabWinDelUndoAct::~OQueryTabWinDelUndoAct()
{
}

void OQueryTabWinDelUndoAct::Undo()
{
    static_cast<OQueryTableView*>(m_pOwner)->ShowTabWin( m_pTabWin, this,sal_True );
    SetOwnership(sal_False);
}

void OQueryTabWinDelUndoAct::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->HideTabWin( m_pTabWin, this );
    SetOwnership(sal_True);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
