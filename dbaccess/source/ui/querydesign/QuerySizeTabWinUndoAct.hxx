/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QuerySizeTabWinUndoAct.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:25:17 $
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
#ifndef DBAUI_QUERYSIZETABWINUNDOACT_HXX
#define DBAUI_QUERYSIZETABWINUNDOACT_HXX

#ifndef DBAUI_QUERYDESIGNUNDOACTION_HXX
#include "QueryDesignUndoAction.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif

namespace dbaui
{

    // ================================================================================================
    // OQuerySizeTabWinUndoAct - Undo-Klasse fuer Groessenveraenderung eines TabWins
    class OTableWindow;
    class OJoinSizeTabWinUndoAct : public OQueryDesignUndoAction
    {
        Point           m_ptNextPosition;
        Size            m_szNextSize;
        OTableWindow*   m_pTabWin;

    protected:
        inline void ToggleSizePosition();

    public:
        OJoinSizeTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPos, const Size& szOriginalSize, OTableWindow* pTabWin);
        // Nebenbedingung : es darf nicht gescrollt worden sein, waehrend die neue Groesse/Position ermittelt wurde, das heisst, die Position
        // hier sind physische, nicht logische Koordinaten
        // (im Gegensatz zur QueryMoveTabWinUndoAct)

        virtual void    Undo() { ToggleSizePosition(); }
        virtual void    Redo() { ToggleSizePosition(); }
    };

    //------------------------------------------------------------------------------
    inline OJoinSizeTabWinUndoAct::OJoinSizeTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPos, const Size& szOriginalSize, OTableWindow* pTabWin)
        :OQueryDesignUndoAction(pOwner, STR_QUERY_UNDO_SIZETABWIN)
        ,m_ptNextPosition(ptOriginalPos)
        ,m_szNextSize(szOriginalSize)
        ,m_pTabWin(pTabWin)
    {
    }

    //------------------------------------------------------------------------------
    inline void OJoinSizeTabWinUndoAct::ToggleSizePosition()
    {
        Point ptNext = m_pTabWin->GetPosPixel();
        Size szNext = m_pTabWin->GetSizePixel();

        m_pOwner->Invalidate(INVALIDATE_NOCHILDREN);
        m_pTabWin->SetPosSizePixel(m_ptNextPosition, m_szNextSize);
        m_pOwner->Invalidate(INVALIDATE_NOCHILDREN);

        m_ptNextPosition = ptNext;
        m_szNextSize = szNext;
    }
}

#endif //DBAUI_QUERYSIZETABWINUNDOACT_HXX


