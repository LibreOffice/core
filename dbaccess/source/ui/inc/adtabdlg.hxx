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
#ifndef DBAUI_QYDLGTAB_HXX
#define DBAUI_QYDLGTAB_HXX

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

#include <com/sun/star/sdbc/XConnection.hpp>

#include <vcl/lstbox.hxx>
#include <tools/string.hxx>

#include "tabletree.hxx"

#include <memory>

namespace dbaui
{
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

    protected:
        ~IAddTableDialogContext() {}
    };

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
