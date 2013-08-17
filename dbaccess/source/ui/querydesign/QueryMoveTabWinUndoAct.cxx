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

#include "QueryMoveTabWinUndoAct.hxx"
#include "JoinTableView.hxx"
#include "QTableWindow.hxx"
using namespace dbaui;
void OJoinMoveTabWinUndoAct::TogglePosition()
{
    Point ptFrameScrollPos(m_pOwner->GetHScrollBar()->GetThumbPos(), m_pOwner->GetVScrollBar()->GetThumbPos());
    Point ptNext = m_pTabWin->GetPosPixel() + ptFrameScrollPos;

    m_pTabWin->SetPosPixel(m_ptNextPosition - ptFrameScrollPos);

    // it looks like ptFrameScrollPos is meaningless, then I it substract here and add it to ptNext, and
    // next time I substract again ...
    // AbetptFrameScrollPos could have changed next time ...
    m_pOwner->EnsureVisible(m_pTabWin);

    m_ptNextPosition = ptNext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
