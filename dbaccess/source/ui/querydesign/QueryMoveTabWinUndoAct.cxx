/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QueryMoveTabWinUndoAct.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:22:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef DBAUI_OQUERYMOVETABWINUNDOACT_HXX
#include "QueryMoveTabWinUndoAct.hxx"
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef DBAUI_QUERY_TABLEWINDOW_HXX
#include "QTableWindow.hxx"
#endif
using namespace dbaui;
//------------------------------------------------------------------------------
void OJoinMoveTabWinUndoAct::TogglePosition()
{
    Point ptFrameScrollPos(m_pOwner->GetHScrollBar()->GetThumbPos(), m_pOwner->GetVScrollBar()->GetThumbPos());
    Point ptNext = m_pTabWin->GetPosPixel() + ptFrameScrollPos;

    m_pTabWin->SetPosPixel(m_ptNextPosition - ptFrameScrollPos);
        // sieht so aus, als wenn das ptFrameScrollPos sinnlos ist, da ich es hier abziehe und auf das ptNext aufaddiere, wo
        // ich es das naechste Mal ja wieder abziehe ... Aber ptFrameScrollPos kann sich natuerlich beim naechsten Mal schon wieder
        // geaendert haben ...
    m_pOwner->EnsureVisible(m_pTabWin);

    m_ptNextPosition = ptNext;
}


