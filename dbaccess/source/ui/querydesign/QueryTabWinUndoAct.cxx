/*************************************************************************
 *
 *  $RCSfile: QueryTabWinUndoAct.cxx,v $
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
DBG_NAME(OQueryTabWinUndoAct )
// ------------------------------------------------------------------------------------------------
OQueryTabWinUndoAct::OQueryTabWinUndoAct(OQueryTableView* pOwner, USHORT nCommentID)
    :OQueryDesignUndoAction(pOwner, nCommentID)
    ,m_pTabWin(NULL)
    ,m_bOwnerOfObjects(FALSE)
{
    DBG_CTOR(OQueryTabWinUndoAct ,NULL);
}
//==============================================================================
OQueryTabWinUndoAct::~OQueryTabWinUndoAct()
{
    if (m_bOwnerOfObjects)
    {   // wenn ich der alleinige Owner des Fenster bin, muss ich dafuer sorgen, dass es geloescht wird
        OSL_ENSURE(m_pTabWin != NULL, "OQueryTabWinUndoAct::~OQueryTabWinUndoAct() : m_pTabWin sollte nicht NULL sein");
        OSL_ENSURE(!m_pTabWin->IsVisible(), "OQueryTabWinUndoAct::~OQueryTabWinUndoAct() : *m_pTabWin sollte nicht sichtbar sein");

        delete m_pTabWin->GetData();
        delete m_pTabWin;

        // und natuerlich auch die entsprechenden Connections
        ::std::vector<OTableConnection*>::iterator aIter = m_vTableConnection.begin();
        for(;aIter != m_vTableConnection.end();++aIter)
        {
            delete (*aIter)->GetData();
            delete (*aIter);
        }
        m_vTableConnection.clear();
    }
    DBG_DTOR(OQueryTabWinUndoAct ,NULL);
}
//------------------------------------------------------------------------------
void OTabFieldCellModifiedUndoAct::Undo()
{
    String strNext = pOwner->GetCellContents(m_nCellIndex, m_nColId);
    pOwner->SetCellContents(m_nCellIndex, m_nColId, m_strNextCellContents);
    m_strNextCellContents = strNext;
}

//------------------------------------------------------------------------------
void OTabFieldSizedUndoAct::Undo()
{
    long nNextWidth = pOwner->GetColumnWidth(m_nColId);
    pOwner->SetColWidth(m_nColId, m_nNextWidth);
    m_nNextWidth = nNextWidth;
}



