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

#include <dsntypes.hxx>
#include <sfx2/tabdlg.hxx>
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

        /** Creates the detail page for ado
        */
        static  VclPtr<SfxTabPage> CreateDbase( TabPageParent _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for ado
        */
        static  VclPtr<SfxTabPage> CreateAdo( TabPageParent _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for ODBC
        */
        static  VclPtr<SfxTabPage> CreateODBC( TabPageParent _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for user
        */
        static  VclPtr<SfxTabPage> CreateUser( TabPageParent _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for MySQLODBC
        */
        static  VclPtr<SfxTabPage> CreateMySQLODBC( TabPageParent _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for MySQLJDBC
        */
        static  VclPtr<SfxTabPage> CreateMySQLJDBC( TabPageParent _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for MySQLNATIVE
        */
        static  VclPtr<SfxTabPage> CreateMySQLNATIVE( TabPageParent _pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for Oracle JDBC
        */
        static VclPtr<SfxTabPage>  CreateOracleJDBC( TabPageParent pParent, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for LDAP
        */
        static  VclPtr<SfxTabPage> CreateLDAP( TabPageParent _pParent, const SfxItemSet* _rAttrSet );

        /// Creates the detail page for Text
        static  VclPtr<SfxTabPage> CreateText( TabPageParent _pParent, const SfxItemSet* _rAttrSet );

        /// creates the GeneratedValues page
        static  VclPtr<SfxTabPage> CreateGeneratedValuesPage( TabPageParent _pParent, const SfxItemSet* _rAttrSet );

        /// creates the "Special Settings" page of the "Advanced Settings" dialog
        static  VclPtr<SfxTabPage> CreateSpecialSettingsPage( TabPageParent _pParent, const SfxItemSet* _rAttrSet );
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_DRIVERSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
