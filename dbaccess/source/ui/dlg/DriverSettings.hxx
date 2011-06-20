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
#ifndef DBAUI_DRIVERSETTINGS_HXX
#define DBAUI_DRIVERSETTINGS_HXX

#include "dsntypes.hxx"
#include <svl/poolitem.hxx>
#include <vector>

class SfxTabPage;
class Window;
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
        static void getSupportedIndirectSettings( const ::rtl::OUString& _sURLPrefix,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory,::std::vector< sal_Int32>& _out_rDetailsIds );

        /** Creates the detail page for Dbase
        */
        static  SfxTabPage* CreateDbase2( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for ado
        */
        static  SfxTabPage* CreateDbase( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for ado
        */
        static  SfxTabPage* CreateAdo( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for ODBC
        */
        static  SfxTabPage* CreateODBC( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for user
        */
        static  SfxTabPage* CreateUser( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for MySQLODBC
        */
        static  SfxTabPage* CreateMySQLODBC( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for MySQLJDBC
        */
        static  SfxTabPage* CreateMySQLJDBC( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for MySQLNATIVE
        */
        static  SfxTabPage* CreateMySQLNATIVE( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for Oracle JDBC
        */
        static SfxTabPage*  CreateOracleJDBC( Window* pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for Adabas
        */
        static  SfxTabPage* CreateAdabas( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for LDAP
        */
        static  SfxTabPage* CreateLDAP( Window* _pParent, const SfxItemSet& _rAttrSet );

        /// Creates the detail page for Text
        static  SfxTabPage* CreateText( Window* _pParent, const SfxItemSet& _rAttrSet );


        /// creates the GeneratedValues page
        static  SfxTabPage* CreateGeneratedValuesPage( Window* _pParent, const SfxItemSet& _rAttrSet );

        /// creates the "Special Settings" page of the "Advanced Settings" dialog
        static  SfxTabPage* CreateSpecialSettingsPage( Window* _pParent, const SfxItemSet& _rAttrSet );
    };
}

#endif // DBAUI_DRIVERSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
