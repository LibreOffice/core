/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DriverSettings.cxx,v $
 * $Revision: 1.17.18.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "DriverSettings.hxx"
#include "dsitems.hxx"
#include "datasourceui.hxx"


using namespace dbaui;
void ODriversSettings::getSupportedIndirectSettings( ::dbaccess::DATASOURCE_TYPE _eType, ::std::vector< sal_Int32>& _out_rDetailsIds )
{
    // for a number of settings, we do not need to use hard-coded here, but can ask a
    // central DataSourceUI instance.
    DataSourceUI aDSUI( _eType );
    const USHORT nGenericKnownSettings[] =
    {
         DSID_SQL92CHECK,
         DSID_APPEND_TABLE_ALIAS,
         DSID_AS_BEFORE_CORRNAME,
         DSID_ENABLEOUTERJOIN,
         DSID_IGNOREDRIVER_PRIV,
         DSID_PARAMETERNAMESUBST,
         DSID_SUPPRESSVERSIONCL,
         DSID_CATALOG,
         DSID_SCHEMA,
         DSID_INDEXAPPENDIX,
         DSID_CHECK_REQUIRED_FIELDS,
         DSID_AUTORETRIEVEENABLED,
         DSID_AUTOINCREMENTVALUE,
         DSID_AUTORETRIEVEVALUE,
         DSID_BOOLEANCOMPARISON,
         DSID_ESCAPE_DATETIME,
         0
    };
    for ( const USHORT* pGenericKnowSetting = nGenericKnownSettings; *pGenericKnowSetting; ++pGenericKnowSetting )
        if ( aDSUI.hasSetting( *pGenericKnowSetting ) )
            _out_rDetailsIds.push_back( *pGenericKnowSetting );

    // the rest is hard-coded. On the long run, all of this should be done via DataSourceUI::hasSetting
    switch ( _eType )
    {
        case  ::dbaccess::DST_DBASE:
            _out_rDetailsIds.push_back(DSID_SHOWDELETEDROWS);
            _out_rDetailsIds.push_back(DSID_CHARSET);
            break;

        case  ::dbaccess::DST_FLAT:
            _out_rDetailsIds.push_back(DSID_FIELDDELIMITER);
            _out_rDetailsIds.push_back(DSID_TEXTDELIMITER);
            _out_rDetailsIds.push_back(DSID_DECIMALDELIMITER);
            _out_rDetailsIds.push_back(DSID_THOUSANDSDELIMITER);
            _out_rDetailsIds.push_back(DSID_TEXTFILEEXTENSION);
            _out_rDetailsIds.push_back(DSID_TEXTFILEHEADER);
            _out_rDetailsIds.push_back(DSID_CHARSET);
            break;

        case  ::dbaccess::DST_ADABAS:
            _out_rDetailsIds.push_back(DSID_CHARSET);
            _out_rDetailsIds.push_back(DSID_CONN_SHUTSERVICE);
            _out_rDetailsIds.push_back(DSID_CONN_DATAINC);
            _out_rDetailsIds.push_back(DSID_CONN_CACHESIZE);
            _out_rDetailsIds.push_back(DSID_CONN_CTRLUSER);
            _out_rDetailsIds.push_back(DSID_CONN_CTRLPWD);
            break;

        case  ::dbaccess::DST_ADO:
            _out_rDetailsIds.push_back(DSID_CHARSET);
            break;

        case  ::dbaccess::DST_ODBC:
            _out_rDetailsIds.push_back(DSID_ADDITIONALOPTIONS);
            _out_rDetailsIds.push_back(DSID_CHARSET);
            _out_rDetailsIds.push_back(DSID_USECATALOG);
            break;

        case  ::dbaccess::DST_MYSQL_NATIVE:
            _out_rDetailsIds.push_back(DSID_CHARSET);
            _out_rDetailsIds.push_back(DSID_CONN_SOCKET);
            break;
        case  ::dbaccess::DST_MYSQL_JDBC:
            _out_rDetailsIds.push_back(DSID_CHARSET);
            _out_rDetailsIds.push_back(DSID_JDBCDRIVERCLASS);
            break;

        case  ::dbaccess::DST_MYSQL_ODBC:
            _out_rDetailsIds.push_back(DSID_CHARSET);
            break;

        case  ::dbaccess::DST_LDAP:
            _out_rDetailsIds.push_back(DSID_CONN_LDAP_BASEDN);
            _out_rDetailsIds.push_back(DSID_CONN_LDAP_ROWCOUNT);
            _out_rDetailsIds.push_back(DSID_CONN_LDAP_USESSL);
            break;

        case  ::dbaccess::DST_JDBC:
            _out_rDetailsIds.push_back(DSID_JDBCDRIVERCLASS);
            break;

        case  ::dbaccess::DST_ORACLE_JDBC:
            _out_rDetailsIds.push_back(DSID_JDBCDRIVERCLASS);
            _out_rDetailsIds.push_back(DSID_IGNORECURRENCY);
            break;

        default:
            break;
    }
}
