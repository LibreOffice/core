/*************************************************************************
 *
 *  $RCSfile: DriverSettings.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:41:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Ocke Janssen
 *
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

        /** Creates the detail page for Text
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateText( Window* _pParent, const SfxItemSet& _rAttrSet );


        /** Creates the GeneratedValues page
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateGeneratedValues( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates a common page with the attribute
              CBTP_USE_ENABLEOUTERJOIN
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateOJDsProperties( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates a common page with the attribute
              CBTP_USE_APPENDTABLEALIAS
            | CBTP_USE_BOOLEANCOMPARISON
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* Create1DsProperties( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates a common page with the attribute
              CBTP_USE_SQL92CHECK
            | CBTP_USE_APPENDTABLEALIAS
            | CBTP_USE_SUPPRESS_VERSION_COLUMN
            | CBTP_USE_BOOLEANCOMPARISON
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* Create2DsProperties( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates a common page with the attribute
              CBTP_USE_SQL92CHECK
            | CBTP_USE_APPENDTABLEALIAS
            | CBTP_USE_SUPPRESS_VERSION_COLUMN
            | CBTP_USE_ENABLEOUTERJOIN
            | CBTP_USE_BOOLEANCOMPARISON
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* Create3DsProperties( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates a common page with the attribute
              CBTP_USE_SQL92CHECK
            | CBTP_USE_APPENDTABLEALIAS
            | CBTP_USE_BOOLEANCOMPARISON
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateFileDsProperties( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates a common page with the attribute
              CBTP_USE_SQL92CHECK
            | CBTP_USE_APPENDTABLEALIAS
            | CBTP_USE_ENABLEOUTERJOIN
            | CBTP_USE_BOOLEANCOMPARISON
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* CreateAccessDsProperties( Window* _pParent, const SfxItemSet& _rAttrSet );

        /** Creates a common page with the attribute
              CBTP_USE_SQL92CHECK
            | CBTP_USE_APPENDTABLEALIAS
            | CBTP_USE_ENABLEOUTERJOIN
            | CBTP_USE_PARAMETERNAMESUBST
            | CBTP_USE_IGNOREDRIVER_PRIV
            | CBTP_USE_SUPPRESS_VERSION_COLUMN
            | CBTP_USE_BOOLEANCOMPARISON
            @param  _pParent
            @param  _rAttrSet

            @return SfxTabPage*
        */
        static  SfxTabPage* Create4DsProperties( Window* _pParent, const SfxItemSet& _rAttrSet );
    };
}

#endif // DBAUI_DRIVERSETTINGS_HXX

