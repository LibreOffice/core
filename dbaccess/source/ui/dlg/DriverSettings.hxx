/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DriverSettings.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:55:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef DBAUI_DRIVERSETTINGS_HXX
#define DBAUI_DRIVERSETTINGS_HXX

#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
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
        static void getSupportedIndirectSettings( DATASOURCE_TYPE _eType,::std::vector< sal_Int32>& _out_rDetailsIds );

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

