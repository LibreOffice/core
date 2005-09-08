/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QueryDesignUndoAction.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:24:12 $
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
#ifndef DBAUI_QUERYDESIGNUNDOACTION_HXX
#define DBAUI_QUERYDESIGNUNDOACTION_HXX

#ifndef DBAUI_GENERALUNDO_HXX
#include "GeneralUndo.hxx"
#endif

namespace dbaui
{
    // ================================================================================================
    // OQueryDesignUndoAction - Undo-Basisklasse fuer Aktionen im graphischen Abfrageentwurf (ohne Feldliste)

    class OJoinTableView;
    class OQueryDesignUndoAction : public OCommentUndoAction
    {
    protected:
        OJoinTableView* m_pOwner;       // in diesem Container spielt sich alles ab

    public:
        OQueryDesignUndoAction(OJoinTableView* pOwner, USHORT nCommentID) : OCommentUndoAction(nCommentID), m_pOwner(pOwner) { }
    };
}
#endif // DBAUI_QUERYDESIGNUNDOACTION_HXX

