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
#include <JoinTableView.hxx>
#include <TableWindow.hxx>
#include <tools/gen.hxx>

namespace dbaui
{

    // OQueryMoveTabWinUndoAct - Undo class for moving a TabWin
    class OTableWindow;
    class OJoinMoveTabWinUndoAct final : public OQueryDesignUndoAction
    {
        Point           m_ptNextPosition;
        VclPtr<OTableWindow>   m_pTabWin;

        void TogglePosition();

    public:
        OJoinMoveTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPosition, OTableWindow* pTabWin);

        virtual void    Undo() override { TogglePosition(); }
        virtual void    Redo() override { TogglePosition(); }
    };

    inline OJoinMoveTabWinUndoAct::OJoinMoveTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPosition, OTableWindow* pTabWin)
        :OQueryDesignUndoAction(pOwner, STR_QUERY_UNDO_MOVETABWIN)
        ,m_ptNextPosition(ptOriginalPosition)
        ,m_pTabWin(pTabWin)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
