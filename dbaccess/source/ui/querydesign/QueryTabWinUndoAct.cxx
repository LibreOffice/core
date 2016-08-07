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





