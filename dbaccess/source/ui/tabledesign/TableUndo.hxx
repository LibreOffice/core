/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEUNDO_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEUNDO_HXX

#include "GeneralUndo.hxx"
#include <tools/multisel.hxx>

#include <vector>

#include <com/sun/star/uno/Any.h>
#include "TypeInfo.hxx"
#include <vcl/vclptr.hxx>

namespace dbaui
{
    class OTableRowView;
    class OTableRow;
    class OTableDesignUndoAct : public OCommentUndoAction
    {
    protected:
        VclPtr<OTableRowView> m_pTabDgnCtrl;

        virtual void    Undo() SAL_OVERRIDE;
        virtual void    Redo() SAL_OVERRIDE;
    public:
        TYPEINFO_OVERRIDE();
        OTableDesignUndoAct( OTableRowView* pOwner ,sal_uInt16 nCommentID);
        virtual ~OTableDesignUndoAct();
    };

    class OTableEditorCtrl;
    class OTableEditorUndoAct : public OTableDesignUndoAct
    {
    protected:
        VclPtr<OTableEditorCtrl> pTabEdCtrl;

    public:
        TYPEINFO_OVERRIDE();
        OTableEditorUndoAct( OTableEditorCtrl* pOwner,sal_uInt16 nCommentID );
        virtual ~OTableEditorUndoAct();
    };

    class OTableDesignCellUndoAct : public OTableDesignUndoAct
    {
    protected:
        sal_uInt16     m_nCol;
        long           m_nRow;
        css::uno::Any  m_sOldText;
        css::uno::Any  m_sNewText;

        virtual void    Undo() SAL_OVERRIDE;
        virtual void    Redo() SAL_OVERRIDE;
    public:
        TYPEINFO_OVERRIDE();
        OTableDesignCellUndoAct( OTableRowView* pOwner, long nRowID, sal_uInt16 nColumn );
        virtual ~OTableDesignCellUndoAct();
    };

    class OTableEditorTypeSelUndoAct : public OTableEditorUndoAct
    {
    protected:
        sal_uInt16          m_nCol;
        long            m_nRow;
        TOTypeInfoSP    m_pOldType;
        TOTypeInfoSP    m_pNewType;

        virtual void    Undo() SAL_OVERRIDE;
        virtual void    Redo() SAL_OVERRIDE;
    public:
        TYPEINFO_OVERRIDE();
        OTableEditorTypeSelUndoAct( OTableEditorCtrl* pOwner, long nRowID, sal_uInt16 nColumn, const TOTypeInfoSP& _pOldType );
        virtual ~OTableEditorTypeSelUndoAct();
    };

    class OTableEditorDelUndoAct : public OTableEditorUndoAct
    {
    protected:
        ::std::vector< std::shared_ptr<OTableRow> > m_aDeletedRows;

        virtual void    Undo() SAL_OVERRIDE;
        virtual void    Redo() SAL_OVERRIDE;
    public:
        TYPEINFO_OVERRIDE();
        explicit OTableEditorDelUndoAct( OTableEditorCtrl* pOwner );
        virtual ~OTableEditorDelUndoAct();
    };

    class OTableEditorInsUndoAct : public OTableEditorUndoAct
    {
    protected:
        ::std::vector< std::shared_ptr<OTableRow> > m_vInsertedRows;
        long                        m_nInsPos;

        virtual void    Undo() SAL_OVERRIDE;
        virtual void    Redo() SAL_OVERRIDE;
    public:
        TYPEINFO_OVERRIDE();
        OTableEditorInsUndoAct( OTableEditorCtrl* pOwner,
                                long nInsertPosition,
                                const ::std::vector<  std::shared_ptr<OTableRow> >& _vInsertedRows);
        virtual ~OTableEditorInsUndoAct();
    };

    class OTableEditorInsNewUndoAct : public OTableEditorUndoAct
    {
    protected:
        long m_nInsPos;
        long m_nInsRows;

        virtual void    Undo() SAL_OVERRIDE;
        virtual void    Redo() SAL_OVERRIDE;
    public:
        TYPEINFO_OVERRIDE();
        OTableEditorInsNewUndoAct( OTableEditorCtrl* pOwner, long nInsertPosition, long nInsertedRows );
        virtual ~OTableEditorInsNewUndoAct();
    };

    class OPrimKeyUndoAct : public OTableEditorUndoAct
    {
    protected:
        MultiSelection      m_aDelKeys,
                            m_aInsKeys;
        VclPtr<OTableEditorCtrl> m_pEditorCtrl;

        virtual void    Undo() SAL_OVERRIDE;
        virtual void    Redo() SAL_OVERRIDE;
    public:
        TYPEINFO_OVERRIDE();
        OPrimKeyUndoAct( OTableEditorCtrl* pOwner, const MultiSelection& aDeletedKeys, const MultiSelection& aInsertedKeys );
        virtual ~OPrimKeyUndoAct();
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
