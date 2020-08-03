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
#pragma once

#include "QueryDesignUndoAction.hxx"
#include <strings.hrc>
#include <TableWindow.hxx>

namespace dbaui
{

    // OQuerySizeTabWinUndoAct - undo class to change size of TabWins
    class OTableWindow;
    class OJoinSizeTabWinUndoAct final : public OQueryDesignUndoAction
    {
        Point           m_ptNextPosition;
        Size            m_szNextSize;
        VclPtr<OTableWindow>   m_pTabWin;

        inline void ToggleSizePosition();

    public:
        OJoinSizeTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPos, const Size& szOriginalSize, OTableWindow* pTabWin);
        // Boundary condition: while retrieving size/position scrolling is not allowed, meaning the position
        // here returns physical and not logical coordinates
        // (in contrary to QueryMoveTabWinUndoAct)

        virtual void    Undo() override { ToggleSizePosition(); }
        virtual void    Redo() override { ToggleSizePosition(); }
    };

    inline OJoinSizeTabWinUndoAct::OJoinSizeTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPos, const Size& szOriginalSize, OTableWindow* pTabWin)
        :OQueryDesignUndoAction(pOwner, STR_QUERY_UNDO_SIZETABWIN)
        ,m_ptNextPosition(ptOriginalPos)
        ,m_szNextSize(szOriginalSize)
        ,m_pTabWin(pTabWin)
    {
    }

    inline void OJoinSizeTabWinUndoAct::ToggleSizePosition()
    {
        Point ptNext = m_pTabWin->GetPosPixel();
        Size szNext = m_pTabWin->GetSizePixel();

        m_pOwner->Invalidate(InvalidateFlags::NoChildren);
        m_pTabWin->SetPosSizePixel(m_ptNextPosition, m_szNextSize);
        m_pOwner->Invalidate(InvalidateFlags::NoChildren);

        m_ptNextPosition = ptNext;
        m_szNextSize = szNext;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
