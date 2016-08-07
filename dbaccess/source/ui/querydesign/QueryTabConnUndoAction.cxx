/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
#ifndef DBAUI_QUERYTABCONNUNDOACTION_HXX
#include "QueryTabConnUndoAction.hxx"
#endif
#ifndef DBAUI_QUERYTABLECONNECTION_HXX
#include "QTableConnection.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBAUI_QUERYADDTABCONNUNDOACTION_HXX
#include "QueryAddTabConnUndoAction.hxx"
#endif
#ifndef DBAUI_QUERYTABWINSHOWUNDOACT_HXX
#include "QueryTabWinShowUndoAct.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif

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
