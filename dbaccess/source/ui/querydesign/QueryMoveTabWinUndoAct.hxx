/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QueryMoveTabWinUndoAct.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:24:57 $
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
#ifndef DBAUI_OQUERYMOVETABWINUNDOACT_HXX
#define DBAUI_OQUERYMOVETABWINUNDOACT_HXX

#ifndef DBAUI_QUERYDESIGNUNDOACTION_HXX
#include "QueryDesignUndoAction.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

namespace dbaui
{

    // ================================================================================================
    // OQueryMoveTabWinUndoAct - Undo-Klasse fuer Verschieben eines TabWins
    class OQueryTableWindow;
    class OTableWindow;
    class OJoinMoveTabWinUndoAct : public OQueryDesignUndoAction
    {
        Point           m_ptNextPosition;
        OTableWindow*   m_pTabWin;

    protected:
        void TogglePosition();

    public:
        OJoinMoveTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPosition, OTableWindow* pTabWin);

        virtual void    Undo() { TogglePosition(); }
        virtual void    Redo() { TogglePosition(); }
    };

    // ------------------------------------------------------------------------------------------------
    inline OJoinMoveTabWinUndoAct::OJoinMoveTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPosition, OTableWindow* pTabWin)
        :OQueryDesignUndoAction(pOwner, STR_QUERY_UNDO_MOVETABWIN)
        ,m_ptNextPosition(ptOriginalPosition)
        ,m_pTabWin(pTabWin)
    {
    }
}
#endif // DBAUI_OQUERYMOVETABWINUNDOACT_HXX


