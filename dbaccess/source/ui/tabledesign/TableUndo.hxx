/*************************************************************************
 *
 *  $RCSfile: TableUndo.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:24:50 $
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
#ifndef DBAUI_TABLEUNDO_HXX
#define DBAUI_TABLEUNDO_HXX

#ifndef DBAUI_GENERALUNDO_HXX
#include "GeneralUndo.hxx"
#endif
#ifndef _SV_MULTISEL_HXX
#include <tools/multisel.hxx>
#endif
#ifndef _VECTOR_
#include <vector>
#endif

namespace dbaui
{
    //========================================================================
    class OTableRowView;
    class OTableRow;
    class OTableDesignUndoAct : public OCommentUndoAction
    {
    protected:
        OTableRowView* m_pTabDgnCtrl;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableDesignUndoAct( OTableRowView* pOwner ,USHORT nCommentID);
        virtual ~OTableDesignUndoAct();
    };

    //========================================================================
    class OTableEditorCtrl;
    class OTableEditorUndoAct : public OTableDesignUndoAct
    {
    protected:
        OTableEditorCtrl* pTabEdCtrl;

    public:
        OTableEditorUndoAct( OTableEditorCtrl* pOwner,USHORT nCommentID );
        virtual ~OTableEditorUndoAct();
    };


    //========================================================================
    class OTableDesignCellUndoAct : public OTableDesignUndoAct
    {
    protected:
        USHORT  m_nCol;
        long    m_nRow;
        ::rtl::OUString m_sOldText;
        ::rtl::OUString m_sNewText;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableDesignCellUndoAct( OTableRowView* pOwner, long nRowID, USHORT nColumn );
        virtual ~OTableDesignCellUndoAct();
    };

    class OTypeInfo;
    //========================================================================
    class OTableEditorTypeSelUndoAct : public OTableEditorUndoAct
    {
    protected:
        USHORT              m_nCol;
        long                m_nRow;
        const OTypeInfo*    m_pOldType;
        const OTypeInfo*    m_pNewType;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableEditorTypeSelUndoAct( OTableEditorCtrl* pOwner, long nRowID, USHORT nColumn, const OTypeInfo* _pOldType );
        virtual ~OTableEditorTypeSelUndoAct();
    };

    //========================================================================
    class OTableEditorDelUndoAct : public OTableEditorUndoAct
    {
    protected:
        ::std::vector<OTableRow*>   m_aDeletedRows;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableEditorDelUndoAct( OTableEditorCtrl* pOwner );
        virtual ~OTableEditorDelUndoAct();
    };

    //========================================================================
    class OTableEditorInsUndoAct : public OTableEditorUndoAct
    {
    protected:
        ::std::vector<OTableRow*>   m_aInsertedRows;
        long                        m_nInsPos;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableEditorInsUndoAct( OTableEditorCtrl* pOwner, long nInsertPosition );
        virtual ~OTableEditorInsUndoAct();
    };

    //========================================================================
    class OTableEditorInsNewUndoAct : public OTableEditorUndoAct
    {
    protected:
        long m_nInsPos;
        long m_nInsRows;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableEditorInsNewUndoAct( OTableEditorCtrl* pOwner, long nInsertPosition, long nInsertedRows );
        virtual ~OTableEditorInsNewUndoAct();
    };

    //========================================================================
    class OPrimKeyUndoAct : public OTableEditorUndoAct
    {
    protected:
        MultiSelection      m_aDelKeys,
                            m_aInsKeys;
        BOOL                m_bActPrimKeySet;
        OTableEditorCtrl* m_pEditorCtrl;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OPrimKeyUndoAct( OTableEditorCtrl* pOwner, MultiSelection aDeletedKeys, MultiSelection aInsertedKeys );
        virtual ~OPrimKeyUndoAct();
    };
}
#endif // DBAUI_TABLEUNDO_HXX


