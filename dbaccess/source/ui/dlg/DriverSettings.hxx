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
#pragma once

#include <sfx2/tabdlg.hxx>

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
        static std::unique_ptr<SfxTabPage> CreateDbase( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for ado
        */
        static std::unique_ptr<SfxTabPage> CreateAdo( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for ODBC
        */
        static std::unique_ptr<SfxTabPage> CreateODBC( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for user
        */
        static std::unique_ptr<SfxTabPage> CreateUser( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for MySQLODBC
        */
        static std::unique_ptr<SfxTabPage> CreateMySQLODBC( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for MySQLJDBC
        */
        static std::unique_ptr<SfxTabPage> CreateMySQLJDBC( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for MySQLNATIVE
        */
        static std::unique_ptr<SfxTabPage> CreateMySQLNATIVE( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for Oracle JDBC
        */
        static std::unique_ptr<SfxTabPage> CreateOracleJDBC( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /** Creates the detail page for LDAP
        */
        static std::unique_ptr<SfxTabPage> CreateLDAP( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /// Creates the detail page for Text
        static std::unique_ptr<SfxTabPage> CreateText( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /// creates the GeneratedValues page
        static std::unique_ptr<SfxTabPage> CreateGeneratedValuesPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );

        /// creates the "Special Settings" page of the "Advanced Settings" dialog
        static std::unique_ptr<SfxTabPage> CreateSpecialSettingsPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet );
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
