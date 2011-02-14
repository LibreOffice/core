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
#ifndef DBAUI_OQueryTabWinUndoAct_HXX
#include "QueryTabWinUndoAct.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef DBAUI_QUERY_TABLEWINDOW_HXX
#include "QTableWindow.hxx"
#endif
#ifndef DBAUI_QUERY_TABLEWINDOWDATA_HXX
#include "QTableWindowData.hxx"
#endif
#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef DBAUI_QUERYDESIGNFIELDUNDOACT_HXX
#include "QueryDesignFieldUndoAct.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif


using namespace dbaui;
DBG_NAME(OQueryDesignFieldUndoAct)
OQueryDesignFieldUndoAct::OQueryDesignFieldUndoAct(OSelectionBrowseBox* pSelBrwBox, sal_uInt16 nCommentID)
    : OCommentUndoAction(nCommentID)
    , pOwner(pSelBrwBox)
    , m_nColumnPostion(BROWSER_INVALIDID)
{
    DBG_CTOR(OQueryDesignFieldUndoAct,NULL);
}
// -----------------------------------------------------------------------------
OQueryDesignFieldUndoAct::~OQueryDesignFieldUndoAct()
{
    DBG_DTOR(OQueryDesignFieldUndoAct,NULL);
    pOwner = NULL;
}
// -----------------------------------------------------------------------------

DBG_NAME(OQueryTabWinUndoAct )
// ------------------------------------------------------------------------------------------------
OQueryTabWinUndoAct::OQueryTabWinUndoAct(OQueryTableView* pOwner, sal_uInt16 nCommentID)
    :OQueryDesignUndoAction(pOwner, nCommentID)
    ,m_pTabWin(NULL)
{
    DBG_CTOR(OQueryTabWinUndoAct ,NULL);
}
//==============================================================================
OQueryTabWinUndoAct::~OQueryTabWinUndoAct()
{
    DBG_DTOR(OQueryTabWinUndoAct ,NULL);
    if (m_bOwnerOfObjects)
    {   // wenn ich der alleinige Owner des Fenster bin, muss ich dafuer sorgen, dass es geloescht wird
        OSL_ENSURE(m_pTabWin != NULL, "OQueryTabWinUndoAct::~OQueryTabWinUndoAct() : m_pTabWin sollte nicht NULL sein");
        OSL_ENSURE(!m_pTabWin->IsVisible(), "OQueryTabWinUndoAct::~OQueryTabWinUndoAct() : *m_pTabWin sollte nicht sichtbar sein");

        if ( m_pTabWin )
            m_pTabWin->clearListBox();
        delete m_pTabWin;

        // und natuerlich auch die entsprechenden Connections
        ::std::vector<OTableConnection*>::iterator aIter = m_vTableConnection.begin();
        ::std::vector<OTableConnection*>::iterator aEnd = m_vTableConnection.end();
        for(;aIter != aEnd;++aIter)
        {
            m_pOwner->DeselectConn(*aIter);
            delete (*aIter);
        }
        m_vTableConnection.clear();
    }
}
//------------------------------------------------------------------------------
void OTabFieldCellModifiedUndoAct::Undo()
{
    pOwner->EnterUndoMode();
    OSL_ENSURE(m_nColumnPostion != BROWSER_INVALIDID,"Column position was not set add the undo action!");
    OSL_ENSURE(m_nColumnPostion < pOwner->GetColumnCount(),"Position outside the column count!");
    if ( m_nColumnPostion != BROWSER_INVALIDID )
    {
        sal_uInt16 nColumnId = pOwner->GetColumnId(m_nColumnPostion);
        String strNext = pOwner->GetCellContents(m_nCellIndex, nColumnId);
        pOwner->SetCellContents(m_nCellIndex, nColumnId, m_strNextCellContents);
        m_strNextCellContents = strNext;
    }
    pOwner->LeaveUndoMode();
}

//------------------------------------------------------------------------------
void OTabFieldSizedUndoAct::Undo()
{
    pOwner->EnterUndoMode();
    OSL_ENSURE(m_nColumnPostion != BROWSER_INVALIDID,"Column position was not set add the undo action!");
    if ( m_nColumnPostion != BROWSER_INVALIDID )
    {
        sal_uInt16 nColumnId = pOwner->GetColumnId(m_nColumnPostion);
        long nNextWidth = pOwner->GetColumnWidth(nColumnId);
        pOwner->SetColWidth(nColumnId, m_nNextWidth);
        m_nNextWidth = nNextWidth;
    }
    pOwner->LeaveUndoMode();
}
// -----------------------------------------------------------------------------
void OTabFieldMovedUndoAct::Undo()
{
    pOwner->EnterUndoMode();
    OSL_ENSURE(m_nColumnPostion != BROWSER_INVALIDID,"Column position was not set add the undo action!");
    if ( m_nColumnPostion != BROWSER_INVALIDID )
    {
        sal_uInt16 nId = pDescr->GetColumnId();
        sal_uInt16 nOldPos = pOwner->GetColumnPos(nId);
        pOwner->SetColumnPos(nId,m_nColumnPostion);
        pOwner->ColumnMoved(nId,sal_False);
        m_nColumnPostion = nOldPos;
    }
    pOwner->LeaveUndoMode();
}
// -----------------------------------------------------------------------------





