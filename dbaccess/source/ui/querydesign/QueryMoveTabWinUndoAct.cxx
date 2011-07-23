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

#include "QueryMoveTabWinUndoAct.hxx"
#include "JoinTableView.hxx"
#include "QTableWindow.hxx"
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
