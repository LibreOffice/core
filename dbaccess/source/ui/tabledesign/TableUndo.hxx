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
        OTableDesignUndoAct( OTableRowView* pOwner ,sal_uInt16 nCommentID);
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
        OTableEditorUndoAct( OTableEditorCtrl* pOwner,sal_uInt16 nCommentID );
        virtual ~OTableEditorUndoAct();
    };


    //========================================================================
    class OTableDesignCellUndoAct : public OTableDesignUndoAct
    {
    protected:
        sal_uInt16  m_nCol;
        long    m_nRow;
        ::com::sun::star::uno::Any  m_sOldText;
        ::com::sun::star::uno::Any  m_sNewText;

        virtual void    Undo();
        virtual void    Redo();
    public:
        TYPEINFO();
        OTableDesignCellUndoAct( OTableRowView* pOwner, long nRowID, sal_uInt16 nColumn );
        virtual ~OTableDesignCellUndoAct();
    };

    class OTypeInfo;
    //========================================================================
    class OTableEditorTypeSelUndoAct : public OTableEditorUndoAct
    {
    protected:
        sal_uInt16          m_nCol;
        long            m_nRow;
        TOTypeInfoSP    m_pOldType;
        TOTypeInfoSP    m_pNewType;

        virtual void    Undo();
        virtual void    Redo();
    public:
        TYPEINFO();
        OTableEditorTypeSelUndoAct( OTableEditorCtrl* pOwner, long nRowID, sal_uInt16 nColumn, const TOTypeInfoSP& _pOldType );
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
        sal_Bool                m_bActPrimKeySet;
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


