/*************************************************************************
 *
 *  $RCSfile: QueryTabWinShowUndoAct.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-05 09:25:19 $
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

#ifndef DBAUI_QUERYTABWINSHOWUNDOACT_HXX
#define DBAUI_QUERYTABWINSHOWUNDOACT_HXX

#ifndef DBAUI_QUERYTABWINUNDOACT_HXX
#include "QueryTabWinUndoAct.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif

namespace dbaui
{
    // ================================================================================================
    // OQueryTabWinShowUndoAct - Undo-Klasse fuer Anzeigen eines TabWins

    class OQueryTabWinShowUndoAct : public OQueryTabWinUndoAct
    {
    public:
        OQueryTabWinShowUndoAct(OQueryTableView* pOwner) : OQueryTabWinUndoAct(pOwner, STR_QUERY_UNDO_TABWINSHOW) { }

        virtual void    Undo() { m_pOwner->HideTabWin(m_pTabWin, this); SetOwnership(TRUE); }
        virtual void    Redo() { m_pOwner->ShowTabWin(m_pTabWin, this); SetOwnership(FALSE); }
    };

    // ================================================================================================
    // OQueryTabWinDelUndoAct - Undo-Klasse fuer Loeschen eines TabWins

    class OQueryTabWinDelUndoAct : public OQueryTabWinUndoAct
    {
    public:
        OQueryTabWinDelUndoAct(OQueryTableView* pOwner) : OQueryTabWinUndoAct(pOwner, STR_QUERY_UNDO_TABWINDELETE) { }

        virtual void    Undo() { m_pOwner->ShowTabWin( m_pTabWin, this ); SetOwnership(FALSE); }
        virtual void    Redo() { m_pOwner->HideTabWin( m_pTabWin, this ); SetOwnership(TRUE); }
    };
}
#endif // DBAUI_QUERYTABWINSHOWUNDOACT_HXX



