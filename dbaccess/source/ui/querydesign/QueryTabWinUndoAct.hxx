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
#ifndef DBAUI_QUERYTABWINUNDOACT_HXX
#define DBAUI_QUERYTABWINUNDOACT_HXX

#include "QueryDesignUndoAction.hxx"
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif // INCLUDED_VECTOR

#include <algorithm>

namespace dbaui
{
    // ================================================================================================
    // OQueryTabWinUndoAct - Undo-Basisklasse fuer alles, was mit Einfuegen/Entfernen von TabWIns zu tun hat zu tun hat

    class OQueryTableWindow;
    class OTableConnection;
    class OQueryTableView;
    class OQueryTabWinUndoAct : public OQueryDesignUndoAction
    {
    protected:
        ::std::vector<OTableConnection*> m_vTableConnection;
        OQueryTableWindow*               m_pTabWin;
        sal_Bool                             m_bOwnerOfObjects;
            // bin ich alleiniger Eigentuemer der verwalteten Objekte ? (aendert sich mit jedem Redo oder Undo)

    public:
        OQueryTabWinUndoAct(OQueryTableView* pOwner, sal_uInt16 nCommentID);
        virtual ~OQueryTabWinUndoAct();

        void SetOwnership(sal_Bool bTakeIt) { m_bOwnerOfObjects = bTakeIt; }


        virtual void Undo() = 0;
        virtual void Redo() = 0;

        // Zugriff auf das TabWin
        void SetTabWin(OQueryTableWindow* pTW) { m_pTabWin = pTW; }
            // anschliessend sollte das SetOwnership aufgerufen werden

        // Zugriff auf die verwalteten Connections
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
