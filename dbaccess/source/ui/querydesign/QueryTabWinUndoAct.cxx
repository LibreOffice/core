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

#include "QueryTabWinUndoAct.hxx"
#include <osl/diagnose.h>
#include "QTableWindow.hxx"
#include "QTableWindowData.hxx"
#include "TableConnection.hxx"
#include "TableConnectionData.hxx"
#include "QueryDesignFieldUndoAct.hxx"
#include "QueryTableView.hxx"

using namespace dbaui;
DBG_NAME(OQueryDesignFieldUndoAct)
OQueryDesignFieldUndoAct::OQueryDesignFieldUndoAct(OSelectionBrowseBox* pSelBrwBox, sal_uInt16 nCommentID)
    : OCommentUndoAction(nCommentID)
    , pOwner(pSelBrwBox)
    , m_nColumnPostion(BROWSER_INVALIDID)
{
    DBG_CTOR(OQueryDesignFieldUndoAct,NULL);
}

OQueryDesignFieldUndoAct::~OQueryDesignFieldUndoAct()
{
    DBG_DTOR(OQueryDesignFieldUndoAct,NULL);
    pOwner = NULL;
}

DBG_NAME(OQueryTabWinUndoAct )
OQueryTabWinUndoAct::OQueryTabWinUndoAct(OQueryTableView* pOwner, sal_uInt16 nCommentID)
    :OQueryDesignUndoAction(pOwner, nCommentID)
    ,m_pTabWin(NULL)
{
    DBG_CTOR(OQueryTabWinUndoAct ,NULL);
}

OQueryTabWinUndoAct::~OQueryTabWinUndoAct()
{
    DBG_DTOR(OQueryTabWinUndoAct ,NULL);
    if (m_bOwnerOfObjects)
    {
        // I should take care to delete the window if I am the only owner
        OSL_ENSURE(m_pTabWin != NULL, "OQueryTabWinUndoAct::~OQueryTabWinUndoAct() : m_pTabWin sollte nicht NULL sein");
        OSL_ENSURE(!m_pTabWin->IsVisible(), "OQueryTabWinUndoAct::~OQueryTabWinUndoAct() : *m_pTabWin sollte nicht sichtbar sein");

        if ( m_pTabWin )
            m_pTabWin->clearListBox();
        delete m_pTabWin;

        // and of course the corresponding connections
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
