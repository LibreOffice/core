/*************************************************************************
 *
 *  $RCSfile: QueryTabConnUndoAction.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-27 14:24:23 $
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
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif

using namespace dbaui;

DBG_NAME(OQueryTabConnUndoAction)
// ------------------------------------------------------------------------------------------------
OQueryTabConnUndoAction::~OQueryTabConnUndoAction()
{
    DBG_DTOR(OQueryTabConnUndoAction,NULL);
    if (m_bOwnerOfConn)
    {   // ich besitze die Connection -> loeschen
        delete m_pConnection->GetData();
        delete m_pConnection;
    }
}

// ------------------------------------------------------------------------------------------------
OQueryTabConnUndoAction::OQueryTabConnUndoAction(OQueryTableView* pOwner, USHORT nCommentID)
    :OQueryDesignUndoAction(pOwner, nCommentID)
    ,m_pConnection(NULL)
    ,m_bOwnerOfConn(FALSE)
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
    SetOwnership(TRUE);
}
// -----------------------------------------------------------------------------
void OQueryAddTabConnUndoAction::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->GetConnection(m_pConnection);
    SetOwnership(FALSE);
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
    SetOwnership(FALSE);
}
// -----------------------------------------------------------------------------
void OQueryDelTabConnUndoAction::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->DropConnection(m_pConnection);
    SetOwnership(TRUE);
}
// -----------------------------------------------------------------------------
OQueryTabWinShowUndoAct::OQueryTabWinShowUndoAct(OQueryTableView* pOwner)
 : OQueryTabWinUndoAct(pOwner, STR_QUERY_UNDO_TABWINSHOW)
{
}
// -----------------------------------------------------------------------------
void OQueryTabWinShowUndoAct::Undo()
{
    static_cast<OQueryTableView*>(m_pOwner)->HideTabWin(m_pTabWin, this);
    SetOwnership(TRUE);
}
// -----------------------------------------------------------------------------
void OQueryTabWinShowUndoAct::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->ShowTabWin(m_pTabWin, this,sal_True);
    SetOwnership(FALSE);
}
// -----------------------------------------------------------------------------
OQueryTabWinDelUndoAct::OQueryTabWinDelUndoAct(OQueryTableView* pOwner)
 : OQueryTabWinUndoAct(pOwner, STR_QUERY_UNDO_TABWINDELETE)
{
}
// -----------------------------------------------------------------------------
void OQueryTabWinDelUndoAct::Undo()
{
    static_cast<OQueryTableView*>(m_pOwner)->ShowTabWin( m_pTabWin, this,sal_True );
    SetOwnership(FALSE);
}
// -----------------------------------------------------------------------------
void OQueryTabWinDelUndoAct::Redo()
{
    static_cast<OQueryTableView*>(m_pOwner)->HideTabWin( m_pTabWin, this );
    SetOwnership(TRUE);
}
// -----------------------------------------------------------------------------
