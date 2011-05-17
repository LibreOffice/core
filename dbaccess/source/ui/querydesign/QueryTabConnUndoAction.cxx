/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "QueryTabConnUndoAction.hxx"
#include "QTableConnection.hxx"
#include <tools/debug.hxx>
#include "QueryTableView.hxx"
#include "QueryAddTabConnUndoAction.hxx"
#include "QueryTabWinShowUndoAct.hxx"
#include "dbu_qry.hrc"

using namespace dbaui;

DBG_NAME(OQueryTabConnUndoAction)
// ------------------------------------------------------------------------------------------------
OQueryTabConnUndoAction::~OQueryTabConnUndoAction()
{
    DBG_DTOR(OQueryTabConnUndoAction,NULL);
    if (m_bOwnerOfConn)
    {   // ich besitze die Connection -> loeschen
        m_pOwner->DeselectConn(m_pConnection);
        delete m_pConnection;
    }
}

// ------------------------------------------------------------------------------------------------
OQueryTabConnUndoAction::OQueryTabConnUndoAction(OQueryTableView* pOwner, sal_uInt16 nCommentID)
    :OQueryDesignUndoAction(pOwner, nCommentID)
    ,m_pConnection(NULL)
    ,m_bOwnerOfConn(sal_False)
{
    DBG_CTOR(OQueryTabConnUndoAction,NULL);
}
// -----------------------------------------------------------------------------
OQueryAddTabConnUndoAction::OQueryAddTabConnUndoAction(OQueryTableView* pOwner)
    : OQueryTabConnUndoAction(pOwner, STR_QUERY_UNDO_INSERTCONNECTION)
{
}
// -----------------------------------------------------------------------------
void OQueryAddTabConnUndoAction::Undo()
{
    static_cast<OQueryTableView*>(m_pOwner)->DropConnection(m_pConnection);
    SetOwnership(sal_True);
}
// -----------------------------------------------------------------------------
void OQueryAddTabConnUndoAction::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->GetConnection(m_pConnection);
    SetOwnership(sal_False);
}
// -----------------------------------------------------------------------------
OQueryDelTabConnUndoAction::OQueryDelTabConnUndoAction(OQueryTableView* pOwner)
    : OQueryTabConnUndoAction(pOwner, STR_QUERY_UNDO_REMOVECONNECTION)
{
}
// -----------------------------------------------------------------------------
void OQueryDelTabConnUndoAction::Undo()
{
    static_cast<OQueryTableView*>(m_pOwner)->GetConnection(m_pConnection);
    SetOwnership(sal_False);
}
// -----------------------------------------------------------------------------
void OQueryDelTabConnUndoAction::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->DropConnection(m_pConnection);
    SetOwnership(sal_True);
}
// -----------------------------------------------------------------------------
OQueryTabWinShowUndoAct::OQueryTabWinShowUndoAct(OQueryTableView* pOwner)
 : OQueryTabWinUndoAct(pOwner, STR_QUERY_UNDO_TABWINSHOW)
{
}
// -----------------------------------------------------------------------------
OQueryTabWinShowUndoAct::~OQueryTabWinShowUndoAct()
{
}
// -----------------------------------------------------------------------------
void OQueryTabWinShowUndoAct::Undo()
{
    static_cast<OQueryTableView*>(m_pOwner)->HideTabWin(m_pTabWin, this);
    SetOwnership(sal_True);
}
// -----------------------------------------------------------------------------
void OQueryTabWinShowUndoAct::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->ShowTabWin(m_pTabWin, this,sal_True);
    SetOwnership(sal_False);
}
// -----------------------------------------------------------------------------
OQueryTabWinDelUndoAct::OQueryTabWinDelUndoAct(OQueryTableView* pOwner)
 : OQueryTabWinUndoAct(pOwner, STR_QUERY_UNDO_TABWINDELETE)
{
}
// -----------------------------------------------------------------------------
OQueryTabWinDelUndoAct::~OQueryTabWinDelUndoAct()
{
}
// -----------------------------------------------------------------------------
void OQueryTabWinDelUndoAct::Undo()
{
    static_cast<OQueryTableView*>(m_pOwner)->ShowTabWin( m_pTabWin, this,sal_True );
    SetOwnership(sal_False);
}
// -----------------------------------------------------------------------------
void OQueryTabWinDelUndoAct::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->HideTabWin( m_pTabWin, this );
    SetOwnership(sal_True);
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
