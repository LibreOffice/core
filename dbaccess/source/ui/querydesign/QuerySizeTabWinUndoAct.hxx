/*************************************************************************
 *
 *  $RCSfile: QuerySizeTabWinUndoAct.hxx,v $
 *
 *  $Revision: 1.2 $
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
#ifndef DBAUI_QUERYSIZETABWINUNDOACT_HXX
#define DBAUI_QUERYSIZETABWINUNDOACT_HXX

#ifndef DBAUI_QUERYDESIGNUNDOACTION_HXX
#include "QueryDesignUndoAction.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
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


