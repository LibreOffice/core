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

#include <svtools/svtdllapi.h>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/scrbar.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/util/AliasProgrammaticPair.hpp>
#include <com/sun/star/sdb/XDatabaseContext.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <unotools/configitem.hxx>


namespace svt
{



    // = AddressBookSourceDialog

    struct AddressBookSourceDialogData;
    class SVT_DLLPUBLIC AddressBookSourceDialog : public ModalDialog
    {
    private:
        // Controls
        VclPtr<ComboBox>       m_pDatasource;
        VclPtr<PushButton>     m_pAdministrateDatasources;
        VclPtr<ComboBox>       m_pTable;
        VclPtr<ScrollBar>      m_pFieldScroller;

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

        AddressBookSourceDialogData*
                               m_pImpl;

    public:
        AddressBookSourceDialog( vcl::Window* _pParent,
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
        AddressBookSourceDialog( vcl::Window* _pParent,
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            const css::uno::Reference< css::sdbc::XDataSource >& _rxTransientDS,
            const OUString& _rDataSourceName,
            const OUString& _rTable,
            const css::uno::Sequence< css::util::AliasProgrammaticPair >& _rMapping
        );

        virtual ~AddressBookSourceDialog();
        virtual void dispose() override;

        // to be used if the object was constructed for editing a field mapping only
        void        getFieldMapping(
            css::uno::Sequence< css::util::AliasProgrammaticPair >& _rMapping) const;

    protected:
        void    implConstruct();

        // Window overridables
        virtual bool        PreNotify( NotifyEvent& _rNEvt ) override;

        // implementations
        void    implScrollFields(sal_Int32 _nPos, bool _bAdjustFocus, bool _bAdjustScrollbar);
        static void implSelectField(ListBox* _pBox, const OUString& _rText);

        void    resetTables();
        void    resetFields();

        // fill in the data sources listbox
        void    initializeDatasources();

        // initialize the dialog from the configuration data
        void    loadConfiguration();

        DECL_LINK_TYPED(OnFieldScroll, ScrollBar*, void);
        DECL_LINK_TYPED(OnFieldSelect, ListBox&, void);
        DECL_LINK_TYPED(OnAdministrateDatasources, Button*, void);
        DECL_STATIC_LINK_TYPED(AddressBookSourceDialog, OnComboGetFocus, Control&, void);
        DECL_LINK_TYPED(OnComboLoseFocus, Control&, void);
        DECL_LINK_TYPED(OnComboSelect, ComboBox&, void);
        DECL_LINK_TYPED(OnOkClicked, Button*, void);
        DECL_LINK_TYPED(OnDelayedInitialize, void*, void);
    };



}   // namespace svt


#endif // INCLUDED_SVTOOLS_ADDRESSTEMPLATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
