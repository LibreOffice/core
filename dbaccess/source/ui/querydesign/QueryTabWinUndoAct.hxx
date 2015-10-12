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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_QUERYTABWINUNDOACT_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_QUERYTABWINUNDOACT_HXX

#include "QueryDesignUndoAction.hxx"
#include <vector>
#include <algorithm>

namespace dbaui
{
    // OQueryTabWinUndoAct - undo base class for all which is concerned with insert/remove TabWins

    class OQueryTableWindow;
    class OTableConnection;
    class OQueryTableView;
    class OQueryTabWinUndoAct : public OQueryDesignUndoAction
    {
    protected:
        ::std::vector<VclPtr<OTableConnection> > m_vTableConnection;
        VclPtr<OQueryTableWindow>                m_pTabWin;
        bool                         m_bOwnerOfObjects;
        // am I the only owner of the managed objects? (changes with every redo or undo)

    public:
        OQueryTabWinUndoAct(OQueryTableView* pOwner, sal_uInt16 nCommentID);
        virtual ~OQueryTabWinUndoAct();

        void SetOwnership(bool bTakeIt) { m_bOwnerOfObjects = bTakeIt; }

        virtual void Undo() override = 0;
        virtual void Redo() override = 0;

        // access to the TabWin
        void SetTabWin(OQueryTableWindow* pTW) { m_pTabWin = pTW; }
        // now SetOwnership should be invoked

        ::std::vector<VclPtr<OTableConnection> >&       GetTabConnList() { return m_vTableConnection; }

        void InsertConnection( OTableConnection* pConnection ) { m_vTableConnection.push_back(pConnection); }
    };

}
#endif // INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_QUERYTABWINUNDOACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
