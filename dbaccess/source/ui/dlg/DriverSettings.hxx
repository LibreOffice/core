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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_DRIVERSETTINGS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_DRIVERSETTINGS_HXX

#include "dsntypes.hxx"
#include <svl/poolitem.hxx>
#include <vcl/vclptr.hxx>
#include <vector>

class SfxTabPage;
namespace vcl { class Window; }
namespace dbaui
{
    /// a collection class for all details a driver needs
    class ODriversSettings
    {
    public:

        /** filles the IDs of the settings which are reflected in indirect data source properties
            (aka properties in the css.sdb.DataSource.Info sequence)

            @param  _eType
                The Type of the data source.
            @param  _out_rDetailsIds
                Will be filled.
        */
        static void getSupportedIndirectSettings( const OUString& _sURLPrefix,const css::uno::Reference< css::uno::XComponentContext >& _xContext,::std::vector< sal_Int32>& _out_rDetailsIds );

        /** Creates the detail page for ado
        */
        static  VclPtr<SfxTabPage> CreateDbase( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for ado
        */
        static  VclPtr<SfxTabPage> CreateAdo( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for ODBC
        */
        static  VclPtr<SfxTabPage> CreateODBC( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for user
        */
        static  VclPtr<SfxTabPage> CreateUser( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for MySQLODBC
        */
        static  VclPtr<SfxTabPage> CreateMySQLODBC( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for MySQLJDBC
        */
        static  VclPtr<SfxTabPage> CreateMySQLJDBC( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for MySQLNATIVE
        */
        static  VclPtr<SfxTabPage> CreateMySQLNATIVE( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for Oracle JDBC
        */
        static VclPtr<SfxTabPage>  CreateOracleJDBC( vcl::Window* pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for LDAP
        */
        static  VclPtr<SfxTabPage> CreateLDAP( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );

        /// Creates the detail page for Text
        static  VclPtr<SfxTabPage> CreateText( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );


        /// creates the GeneratedValues page
        static  VclPtr<SfxTabPage> CreateGeneratedValuesPage( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );

        /// creates the "Special Settings" page of the "Advanced Settings" dialog
        static  VclPtr<SfxTabPage> CreateSpecialSettingsPage( vcl::Window* _pParent, const SfxItemSet* _rAttrSet );
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_DRIVERSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
