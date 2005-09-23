/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QueryDesignFieldUndoAct.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:42:49 $
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
#ifndef DBAUI_QUERYDESIGNFIELDUNDOACT_HXX
#define DBAUI_QUERYDESIGNFIELDUNDOACT_HXX

#ifndef DBAUI_GENERALUNDO_HXX
#include "GeneralUndo.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef DBAUI_QUERYDESIGN_OSELECTIONBROWSEBOX_HXX
#include "SelectionBrowseBox.hxx"
#endif


namespace dbaui
{
    // ================================================================================================
    // OQueryDesignFieldUndoAct - Basisklasse fuer Undos in der Feldauflistung im Abfrageentwurf


    class OQueryDesignFieldUndoAct : public OCommentUndoAction
    {
    protected:
        OSelectionBrowseBox*    pOwner;
        USHORT                  m_nColumnPostion;

        virtual void    Undo() = 0;
        virtual void    Redo() = 0;

    public:
        OQueryDesignFieldUndoAct(OSelectionBrowseBox* pSelBrwBox, USHORT nCommentID);
        virtual ~OQueryDesignFieldUndoAct();

        inline void SetColumnPosition(USHORT _nColumnPostion)
        {
            m_nColumnPostion = _nColumnPostion;
            OSL_ENSURE(m_nColumnPostion != BROWSER_INVALIDID,"Column position was not set add the undo action!");
            OSL_ENSURE(m_nColumnPostion < pOwner->GetColumnCount(),"Position outside the column count!");
        }
    };

    // ================================================================================================
    // OTabFieldCellModifiedUndoAct - Undo-Klasse fuer Aendern einer Zelle einer Spaltenbeschreibung

    class OTabFieldCellModifiedUndoAct : public OQueryDesignFieldUndoAct
    {
    protected:
        String      m_strNextCellContents;
        sal_Int32   m_nCellIndex;

    public:
        OTabFieldCellModifiedUndoAct(OSelectionBrowseBox* pSelBrwBox)
            : OQueryDesignFieldUndoAct(pSelBrwBox, STR_QUERY_UNDO_MODIFY_CELL)
            ,m_nCellIndex(BROWSER_INVALIDID){ }

        inline void SetCellContents(const String& str)  { m_strNextCellContents = str; }
        inline void SetCellIndex(sal_Int32 nIndex)      { m_nCellIndex = nIndex; }

        virtual void Undo();
        virtual void Redo() { Undo(); }
    };

    // ================================================================================================
    // OTabFieldSizedUndoAct - Undo-Klasse fuer Aendern einer Spaltenbreite

    class OTabFieldSizedUndoAct : public OQueryDesignFieldUndoAct
    {
    protected:
        long        m_nNextWidth;

    public:
        OTabFieldSizedUndoAct(OSelectionBrowseBox* pSelBrwBox) : OQueryDesignFieldUndoAct(pSelBrwBox, STR_QUERY_UNDO_SIZE_COLUMN), m_nNextWidth(0) { }

        inline void SetOriginalWidth(long nWidth) { OSL_ENSURE(m_nNextWidth =! 0,"Width is 0!");m_nNextWidth = nWidth; }

        virtual void Undo();
        virtual void Redo() { Undo(); }
    };

    // ================================================================================================
    // OTabFieldUndoAct - Basisklasse fuer Undos in der Feldauflistung im Abfrageentwurf, die mit Veraendern einer kompletten Feldbeschreibung zu tun haben

    class OTabFieldUndoAct : public OQueryDesignFieldUndoAct
    {
    protected:
        OTableFieldDescRef      pDescr;     // geloeschte Spaltenbeschreibung

    public:
        OTabFieldUndoAct(OSelectionBrowseBox* pSelBrwBox, USHORT nCommentID) : OQueryDesignFieldUndoAct(pSelBrwBox, nCommentID) { }

        void SetTabFieldDescr(OTableFieldDescRef pDescription) { pDescr = pDescription; }
    };

    // ================================================================================================
    // OTabFieldDelUndoAct - Undo-Klasse fuer Loeschen eines Feldes

    class OTabFieldDelUndoAct : public OTabFieldUndoAct
    {
    protected:
        virtual void Undo() { pOwner->EnterUndoMode();pOwner->InsertColumn(pDescr, m_nColumnPostion);pOwner->LeaveUndoMode(); }
        virtual void Redo() { pOwner->EnterUndoMode();pOwner->RemoveColumn(pDescr->GetColumnId());pOwner->LeaveUndoMode(); }

    public:
        OTabFieldDelUndoAct(OSelectionBrowseBox* pSelBrwBox) : OTabFieldUndoAct(pSelBrwBox, STR_QUERY_UNDO_TABFIELDDELETE) { }
    };

    // ================================================================================================
    // OTabFieldDelUndoAct - Undo-Klasse fuer Anlegen eines Feldes

    class OTabFieldCreateUndoAct : public OTabFieldUndoAct
    {
    protected:
        virtual void Undo() { pOwner->EnterUndoMode();pOwner->RemoveColumn(pDescr->GetColumnId());pOwner->LeaveUndoMode();}
        virtual void Redo() { pOwner->EnterUndoMode();pOwner->InsertColumn(pDescr, m_nColumnPostion);pOwner->LeaveUndoMode();}

    public:
        OTabFieldCreateUndoAct(OSelectionBrowseBox* pSelBrwBox) : OTabFieldUndoAct(pSelBrwBox, STR_QUERY_UNDO_TABFIELDCREATE) { }
    };

    // ================================================================================================
    // OTabFieldMovedUndoAct - Undo-class when a field was moved inside the selection

    class OTabFieldMovedUndoAct : public OTabFieldUndoAct
    {
    protected:
        virtual void Undo();
        virtual void Redo()
        {
            Undo();
        }

    public:
        OTabFieldMovedUndoAct(OSelectionBrowseBox* pSelBrwBox) : OTabFieldUndoAct(pSelBrwBox, STR_QUERY_UNDO_TABFIELDMOVED) { }
    };
}
#endif // DBAUI_QUERYDESIGNFIELDUNDOACT_HXX



