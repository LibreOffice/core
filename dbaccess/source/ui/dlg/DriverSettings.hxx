/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DriverSettings.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:49:04 $
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

        /** fills the DetailIds for the given type
            @param  _eType
                The Type of the data source.
            @param  _rDetailsIds
                Will be filled.
        */
        static void fillDetailIds(DATASOURCE_TYPE _eType,::std::vector< sal_Int32>& _rDetailsIds);

        /** Creates the detail page for Dbase
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateDbase2( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for ado
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */


        static  SfxTabPage* CreateDbase( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for ado
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateAdo( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for ODBC
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateODBC( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for user
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateUser( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for MySQLODBC
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateMySQLODBC( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for MySQLJDBC
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateMySQLJDBC( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for Oracle JDBC
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static SfxTabPage*  CreateOracleJDBC( Window* pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for Adabas
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateAdabas( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates the detail page for LDAP
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateLDAP( Window* _pParent, const SfxItemSet& _rAttrSet );


        /// Creates the detail page for Text
        static  SfxTabPage* CreateText( Window* _pParent, const SfxItemSet& _rAttrSet );


        /// creates the GeneratedValues page
        static  SfxTabPage* CreateGeneratedValuesPage( Window* _pParent, const SfxItemSet& _rAttrSet );

        /// creates a common page with the attributes needed for MySQL advanced settings.
        static  SfxTabPage* CreateMySQLSettingsPage( Window* _pParent, const SfxItemSet& _rAttrSet );

        /// creates a common page with the attributes needed for Adabas advanced settings.
        static  SfxTabPage* CreateAdabasSettingsPage( Window* _pParent, const SfxItemSet& _rAttrSet );

        /// creates a common page with the attributes needed for ADO advanced settings.
        static  SfxTabPage* CreateADOSettingsPage( Window* _pParent, const SfxItemSet& _rAttrSet );

        /// creates a common page with the attributes needed for file-based (dBase, CSV) advanced settings.
        static  SfxTabPage* CreateFileSettingsPage( Window* _pParent, const SfxItemSet& _rAttrSet );

        /// creates a common page with the attributes needed for MSA advanced settings.
        static  SfxTabPage* CreateAccessSettingsPage( Window* _pParent, const SfxItemSet& _rAttrSet );

        /// creates a common page with all supported advanced attributes
        static  SfxTabPage* CreateFullSettingsPage( Window* _pParent, const SfxItemSet& _rAttrSet );
    };
}

#endif // DBAUI_DRIVERSETTINGS_HXX

