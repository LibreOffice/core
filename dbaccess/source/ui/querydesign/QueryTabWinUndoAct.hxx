/*************************************************************************
 *
 *  $RCSfile: QueryTabWinUndoAct.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-27 14:24:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_QUERYTABWINUNDOACT_HXX
#define DBAUI_QUERYTABWINUNDOACT_HXX

#ifndef DBAUI_QUERYDESIGNUNDOACTION_HXX
#include "QueryDesignUndoAction.hxx"
#endif
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
        BOOL                             m_bOwnerOfObjects;
            // bin ich alleiniger Eigentuemer der verwalteten Objekte ? (aendert sich mit jedem Redo oder Undo)

    public:
        OQueryTabWinUndoAct(OQueryTableView* pOwner, USHORT nCommentID);
        virtual ~OQueryTabWinUndoAct();

        void SetOwnership(BOOL bTakeIt) { m_bOwnerOfObjects = bTakeIt; }


        virtual void Undo() = 0;
        virtual void Redo() = 0;

        // Zugriff auf das TabWin
        void SetTabWin(OQueryTableWindow* pTW) { m_pTabWin = pTW; }
            // anschliessend sollte das SetOwnership aufgerufen werden

        // Zugriff auf die verwalteten Connections
        USHORT  ConnCount() { return m_vTableConnection.size(); }

        ::std::vector<OTableConnection*>*       GetTabConnList() { return &m_vTableConnection; }

        void InsertConnection( OTableConnection* pConnection ) { m_vTableConnection.push_back(pConnection); }
        void RemoveConnection( OTableConnection* pConnection )
        {
            m_vTableConnection.erase(::std::remove(m_vTableConnection.begin(),m_vTableConnection.end(),pConnection),m_vTableConnection.end());
        }
    };


}
#endif // DBAUI_QUERYTABWINUNDOACT_HXX



