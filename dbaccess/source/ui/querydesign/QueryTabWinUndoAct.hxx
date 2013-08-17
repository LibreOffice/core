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
#ifndef DBAUI_QUERYTABWINUNDOACT_HXX
#define DBAUI_QUERYTABWINUNDOACT_HXX

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
        ::std::vector<OTableConnection*> m_vTableConnection;
        OQueryTableWindow*               m_pTabWin;
        sal_Bool                             m_bOwnerOfObjects;
        // am I the only owner of the managed objects? (changes with every redo or undo)

    public:
        OQueryTabWinUndoAct(OQueryTableView* pOwner, sal_uInt16 nCommentID);
        virtual ~OQueryTabWinUndoAct();

        void SetOwnership(sal_Bool bTakeIt) { m_bOwnerOfObjects = bTakeIt; }

        virtual void Undo() = 0;
        virtual void Redo() = 0;

        // access to the TabWin
        void SetTabWin(OQueryTableWindow* pTW) { m_pTabWin = pTW; }
        // now SetOwnership should be invoked

        // access to the managed connections
        sal_uInt16  ConnCount() { return (sal_uInt16)m_vTableConnection.size(); }

        ::std::vector<OTableConnection*>*       GetTabConnList() { return &m_vTableConnection; }

        void InsertConnection( OTableConnection* pConnection ) { m_vTableConnection.push_back(pConnection); }
        void RemoveConnection( OTableConnection* pConnection )
        {
            m_vTableConnection.erase(::std::remove(m_vTableConnection.begin(),m_vTableConnection.end(),pConnection),m_vTableConnection.end());
        }
    };

}
#endif // DBAUI_QUERYTABWINUNDOACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
