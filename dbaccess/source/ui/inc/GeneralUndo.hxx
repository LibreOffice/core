/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GeneralUndo.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 08:22:49 $
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
#ifndef DBAUI_GENERALUNDO_HXX
#define DBAUI_GENERALUNDO_HXX

#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif

namespace dbaui
{
    // ================================================================================================
    // SbaCommentUndoAction - Undo-Basisklasse fuer Aktionen, deren GetComment einen aus einer Sba-Ressource
    // geladenen String liefert

    class OCommentUndoAction : public SfxUndoAction
    {
        OModuleClient    m_aModuleClient;

    protected:
        String                  m_strComment; // undo, redo comment

    public:
        TYPEINFO();
        OCommentUndoAction(USHORT nCommentID) { m_strComment = String(ModuleRes(nCommentID)); }

        virtual UniString   GetComment() const { return m_strComment; }
    };
}
#endif // DBAUI_GENERALUNDO_HXX


