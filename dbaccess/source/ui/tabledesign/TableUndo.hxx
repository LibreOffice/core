/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableUndo.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-04 08:52:07 $
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
#ifndef DBAUI_TABLEUNDO_HXX
#define DBAUI_TABLEUNDO_HXX

#ifndef DBAUI_GENERALUNDO_HXX
#include "GeneralUndo.hxx"
#endif
#ifndef _SV_MULTISEL_HXX
#include <tools/multisel.hxx>
#endif

#include <vector>

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
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
        TYPEINFO();
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
        TYPEINFO();
        OTableEditorUndoAct( OTableEditorCtrl* pOwner,USHORT nCommentID );
        virtual ~OTableEditorUndoAct();
    };


    //========================================================================
    class OTableDesignCellUndoAct : public OTableDesignUndoAct
    {
    protected:
        USHORT  m_nCol;
        long    m_nRow;
        ::com::sun::star::uno::Any  m_sOldText;
        ::com::sun::star::uno::Any  m_sNewText;

        virtual void    Undo();
        virtual void    Redo();
    public:
        TYPEINFO();
        OTableDesignCellUndoAct( OTableRowView* pOwner, long nRowID, USHORT nColumn );
        virtual ~OTableDesignCellUndoAct();
    };

    class OTypeInfo;
    //========================================================================
    class OTableEditorTypeSelUndoAct : public OTableEditorUndoAct
    {
    protected:
        USHORT          m_nCol;
        long            m_nRow;
        TOTypeInfoSP    m_pOldType;
        TOTypeInfoSP    m_pNewType;

        virtual void    Undo();
        virtual void    Redo();
    public:
        TYPEINFO();
        OTableEditorTypeSelUndoAct( OTableEditorCtrl* pOwner, long nRowID, USHORT nColumn, const TOTypeInfoSP& _pOldType );
        virtual ~OTableEditorTypeSelUndoAct();
    };

    //========================================================================
    class OTableEditorDelUndoAct : public OTableEditorUndoAct
    {
    protected:
        ::std::vector< ::boost::shared_ptr<OTableRow> > m_aDeletedRows;

        virtual void    Undo();
        virtual void    Redo();
    public:
        TYPEINFO();
        OTableEditorDelUndoAct( OTableEditorCtrl* pOwner );
        virtual ~OTableEditorDelUndoAct();
    };

    //========================================================================
    class OTableEditorInsUndoAct : public OTableEditorUndoAct
    {
    protected:
        ::std::vector< ::boost::shared_ptr<OTableRow> > m_vInsertedRows;
        long                        m_nInsPos;

        virtual void    Undo();
        virtual void    Redo();
    public:
        TYPEINFO();
        OTableEditorInsUndoAct( OTableEditorCtrl* pOwner,
                                long nInsertPosition,
                                const ::std::vector<  ::boost::shared_ptr<OTableRow> >& _vInsertedRows);
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
        TYPEINFO();
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
        TYPEINFO();
        OPrimKeyUndoAct( OTableEditorCtrl* pOwner, MultiSelection aDeletedKeys, MultiSelection aInsertedKeys );
        virtual ~OPrimKeyUndoAct();
    };
}
#endif // DBAUI_TABLEUNDO_HXX


