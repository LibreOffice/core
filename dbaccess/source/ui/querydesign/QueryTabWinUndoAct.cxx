/*************************************************************************
 *
 *  $RCSfile: QueryTabWinUndoAct.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:13:02 $
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
OQueryDesignFieldUndoAct::OQueryDesignFieldUndoAct(OSelectionBrowseBox* pSelBrwBox, USHORT nCommentID)
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
OQueryTabWinUndoAct::OQueryTabWinUndoAct(OQueryTableView* pOwner, USHORT nCommentID)
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

        delete m_pTabWin->GetData();
        if ( m_pTabWin )
            m_pTabWin->clearListBox();
        delete m_pTabWin;

        // und natuerlich auch die entsprechenden Connections
        ::std::vector<OTableConnection*>::iterator aIter = m_vTableConnection.begin();
        for(;aIter != m_vTableConnection.end();++aIter)
        {
            m_pOwner->DeselectConn(*aIter);
            delete (*aIter)->GetData();
            delete (*aIter);
        }
        m_vTableConnection.clear();
    }
}
//------------------------------------------------------------------------------
void OTabFieldCellModifiedUndoAct::Undo()
{
    OSL_ENSURE(m_nColumnPostion != BROWSER_INVALIDID,"Column position was not set add the undo action!");
    OSL_ENSURE(m_nColumnPostion < pOwner->GetColumnCount(),"Position outside the column count!");
    if ( m_nColumnPostion != BROWSER_INVALIDID )
    {
        USHORT nColumnId = pOwner->GetColumnId(m_nColumnPostion);
        String strNext = pOwner->GetCellContents(m_nCellIndex, nColumnId);
        pOwner->SetCellContents(m_nCellIndex, nColumnId, m_strNextCellContents);
        m_strNextCellContents = strNext;
    }
}

//------------------------------------------------------------------------------
void OTabFieldSizedUndoAct::Undo()
{
    OSL_ENSURE(m_nColumnPostion != BROWSER_INVALIDID,"Column position was not set add the undo action!");
    if ( m_nColumnPostion != BROWSER_INVALIDID )
    {
        USHORT nColumnId = pOwner->GetColumnId(m_nColumnPostion);
        long nNextWidth = pOwner->GetColumnWidth(nColumnId);
        pOwner->SetColWidth(nColumnId, m_nNextWidth);
        m_nNextWidth = nNextWidth;
    }
}
// -----------------------------------------------------------------------------
void OTabFieldMovedUndoAct::Undo()
{
    OSL_ENSURE(m_nColumnPostion != BROWSER_INVALIDID,"Column position was not set add the undo action!");
    if ( m_nColumnPostion != BROWSER_INVALIDID )
    {
        sal_uInt16 nId = pDescr->GetColumnId();
        USHORT nOldPos = pOwner->GetColumnPos(nId);
        pOwner->SetColumnPos(nId,m_nColumnPostion);
        pOwner->ColumnMoved(nId,FALSE);
        m_nColumnPostion = nOldPos;
    }
}
// -----------------------------------------------------------------------------





