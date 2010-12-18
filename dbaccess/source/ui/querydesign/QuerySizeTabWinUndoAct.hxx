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
#ifndef DBAUI_QUERYSIZETABWINUNDOACT_HXX
#define DBAUI_QUERYSIZETABWINUNDOACT_HXX

#include "QueryDesignUndoAction.hxx"
#include "dbu_qry.hrc"

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
