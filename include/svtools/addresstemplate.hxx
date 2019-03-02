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

#ifndef INCLUDED_SVTOOLS_ADDRESSTEMPLATE_HXX
#define INCLUDED_SVTOOLS_ADDRESSTEMPLATE_HXX

#include <memory>
#include <svtools/svtdllapi.h>
#include <vcl/weld.hxx>

namespace com :: sun :: star :: container { class XNameAccess; }
namespace com :: sun :: star :: sdbc { class XDataSource; }
namespace com :: sun :: star :: sdb { class XDatabaseContext; }
namespace com :: sun :: star :: uno { class XComponentContext; }
namespace com :: sun :: star :: util { struct AliasProgrammaticPair; }

#define FIELD_PAIRS_VISIBLE         5
#define FIELD_CONTROLS_VISIBLE      2 * FIELD_PAIRS_VISIBLE

namespace svt
{
    // = AddressBookSourceDialog
    struct AddressBookSourceDialogData;
    class SVT_DLLPUBLIC AddressBookSourceDialog final : public weld::GenericDialogController
    {
    public:
        AddressBookSourceDialog(weld::Window* _pParent,
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB );

        /** if you use this ctor, the dialog
            <ul><li>will not store it's data in the configuration (nor initially retrieve it from there)</li>
                <li>will not allow to change the data source name</li>
                <li>will not allow to change the table name</li>
                <li>will not allow to call the data source administration dialog</li>
            </ul>

            @param _rxORB
                a service factory to use for various UNO related needs
            @param _rxTransientDS
                the data source to obtain connections from
            @param _rDataSourceName
                the to-be name of _rxTransientDS. This is only for displaying this
                name to the user, since the dialog completely works on _rxTransientDS,
                and doesn't allow to change this.
            @param _rTable
                the table name to display. It must refer to a valid table, relative to a connection
                obtained from <arg>_rxTransientDS</arg>
        */
        AddressBookSourceDialog(weld::Window* _pParent,
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            const css::uno::Reference< css::sdbc::XDataSource >& _rxTransientDS,
            const OUString& _rDataSourceName,
            const OUString& _rTable,
            const css::uno::Sequence< css::util::AliasProgrammaticPair >& _rMapping
        );

        virtual ~AddressBookSourceDialog() override;

        // to be used if the object was constructed for editing a field mapping only
        void        getFieldMapping(
            css::uno::Sequence< css::util::AliasProgrammaticPair >& _rMapping) const;

    private:
        void    implConstruct();

        // implementations
        void    implScrollFields(sal_Int32 nPos, bool bAdjustFocus, bool bAdjustScrollbar);
        static void implSelectField(weld::ComboBox* pBox, const OUString& rText);

        void    resetTables();
        void    resetFields();

        // fill in the data sources listbox
        void    initializeDatasources();

        // initialize the dialog from the configuration data
        void    loadConfiguration();

        DECL_LINK(OnFieldScroll, weld::ScrolledWindow&, void);
        DECL_LINK(OnFieldSelect, weld::ComboBox&, void);
        DECL_LINK(OnAdministrateDatasources, weld::Button&, void);
        DECL_STATIC_LINK(AddressBookSourceDialog, OnComboGetFocus, weld::Widget&, void);
        DECL_LINK(OnComboLoseFocus, weld::Widget&, void);
        DECL_LINK(OnComboSelect, weld::ComboBox&, void);
        DECL_LINK(OnOkClicked, weld::Button&, void);
        DECL_LINK(OnDelayedInitialize, void*, void);

        // string to display for "no selection"
        const OUString         m_sNoFieldSelection;

        /// the DatabaseContext for selecting data sources
        css::uno::Reference< css::sdb::XDatabaseContext >
                               m_xDatabaseContext;
        // the ORB for creating objects
        css::uno::Reference< css::uno::XComponentContext >
                               m_xORB;
        css::uno::Reference< css::container::XNameAccess >
                               m_xCurrentDatasourceTables;

        // Controls
        std::unique_ptr<weld::ComboBox> m_xDatasource;
        std::unique_ptr<weld::Button> m_xAdministrateDatasources;
        std::unique_ptr<weld::ComboBox> m_xTable;
        std::unique_ptr<weld::ScrolledWindow> m_xFieldScroller;
        std::unique_ptr<weld::Button> m_xOKButton;
        std::unique_ptr<weld::Widget> m_xGrid;

        std::unique_ptr<AddressBookSourceDialogData> m_pImpl;
    };
}   // namespace svt

#endif // INCLUDED_SVTOOLS_ADDRESSTEMPLATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
