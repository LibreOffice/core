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
#ifndef DBAUI_QYDLGTAB_HXX
#define DBAUI_QYDLGTAB_HXX

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

#include <com/sun/star/sdbc/XConnection.hpp>

#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>
#include <tools/string.hxx>

#include "tabletree.hxx"

#include <memory>

namespace dbaui
{
    //========================================================================
    /** unifies the access to a list of table/query objects
    */
    class TableObjectListFacade
    {
    public:
        virtual void    updateTableObjectList( bool _bAllowViews ) = 0;
        virtual String  getSelectedName( String& _out_rAliasName ) const = 0;
        virtual bool    isLeafSelected() const = 0;

        virtual ~TableObjectListFacade();
    };

    //========================================================================
    class IAddTableDialogContext
    {
    public:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                        getConnection() const = 0;
        virtual bool    allowViews() const = 0;
        virtual bool    allowQueries() const = 0;
        virtual bool    allowAddition() const = 0;
        virtual void    addTableWindow( const String& _rQualifiedTableName, const String& _rAliasName ) = 0;
        virtual void    onWindowClosing( const Window* _pWindow ) = 0;
    };

    //========================================================================
    class OAddTableDlg : public ModelessDialog
    {
        RadioButton         m_aCaseTables;
        RadioButton         m_aCaseQueries;

        OTableTreeListBox   m_aTableList;
        SvTreeListBox       m_aQueryList;
        ::std::auto_ptr< TableObjectListFacade >
                            m_pCurrentList;

        PushButton          aAddButton;
        CancelButton        aCloseButton;
        HelpButton          aHelpButton;

        IAddTableDialogContext&
                            m_rContext;

        DECL_LINK( AddClickHdl, Button* );
        DECL_LINK( CloseClickHdl, Button* );
        DECL_LINK( TableListDoubleClickHdl, void* );
        DECL_LINK( TableListSelectHdl, void* );
        DECL_LINK( OnTypeSelected, void* );

    public:
        OAddTableDlg(
            Window* _pParent,
            IAddTableDialogContext& _rContext );
        virtual ~OAddTableDlg();

        void DetermineAddTable() { aAddButton.Enable( impl_isAddAllowed() ); }
        void Update();

        static  String  getDialogTitleForContext(
            IAddTableDialogContext& _rContext );

    private:
        virtual sal_Bool Close();

        bool impl_isAddAllowed();
        void impl_addTable();

        enum ObjectList
        {
            Tables,
            Queries
        };
        void impl_switchTo( ObjectList _eList );
    };
}
#endif // DBAUI_QYDLGTAB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
