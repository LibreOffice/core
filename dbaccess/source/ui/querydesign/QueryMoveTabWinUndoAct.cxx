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


