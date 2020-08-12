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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_ADTABDLG_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_ADTABDLG_HXX

#include <memory>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <vcl/weld.hxx>
#include "tabletree.hxx"

namespace dbaui
{
    /** unifies the access to a list of table/query objects
    */
    class TableObjectListFacade
    {
    public:
        virtual void    updateTableObjectList( bool _bAllowViews ) = 0;
        virtual OUString  getSelectedName( OUString& _out_rAliasName ) const = 0;
        virtual bool    isLeafSelected() const = 0;

        virtual ~TableObjectListFacade();
    };

    class IAddTableDialogContext
    {
    public:
        virtual css::uno::Reference< css::sdbc::XConnection >
                        getConnection() const = 0;
        virtual bool    allowViews() const = 0;
        virtual bool    allowQueries() const = 0;
        virtual bool    allowAddition() const = 0;
        virtual void    addTableWindow( const OUString& _rQualifiedTableName, const OUString& _rAliasName ) = 0;
        virtual void    onWindowClosing() = 0;

    protected:
        ~IAddTableDialogContext() {}
    };

    class OAddTableDlg : public weld::GenericDialogController
    {
        IAddTableDialogContext& m_rContext;
        std::unique_ptr< TableObjectListFacade > m_xCurrentList;

        std::unique_ptr<weld::RadioButton> m_xCaseTables;
        std::unique_ptr<weld::RadioButton> m_xCaseQueries;

        std::unique_ptr<OTableTreeListBox> m_xTableList;
        std::unique_ptr<weld::TreeView> m_xQueryList;

        std::unique_ptr<weld::Button> m_xAddButton;
        std::unique_ptr<weld::Button> m_xCloseButton;

        DECL_LINK( AddClickHdl, weld::Button&, void );
        DECL_LINK( CloseClickHdl, weld::Button&, void);
        DECL_LINK( TableListDoubleClickHdl, weld::TreeView&, bool );
        DECL_LINK( TableListSelectHdl, weld::TreeView&, void );
        DECL_LINK( OnTypeSelected, weld::Button&, void );

    public:
        OAddTableDlg(weld::Window* _pParent,
                     IAddTableDialogContext& _rContext);
        virtual ~OAddTableDlg() override;

        void Update();
        void OnClose();

        static  OUString  getDialogTitleForContext(
            IAddTableDialogContext const & _rContext );

    private:
        bool impl_isAddAllowed();

        enum ObjectList
        {
            Tables,
            Queries
        };
        void impl_switchTo( ObjectList _eList );
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_ADTABDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
