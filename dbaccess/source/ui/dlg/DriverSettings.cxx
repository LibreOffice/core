/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DriverSettings.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:55:41 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "DriverSettings.hxx"
#include "dsitems.hxx"
#include "datasourceui.hxx"


using namespace dbaui;
void ODriversSettings::getSupportedIndirectSettings( DATASOURCE_TYPE _eType, ::std::vector< sal_Int32>& _out_rDetailsIds )
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
         0
    };
    for ( const USHORT* pGenericKnowSetting = nGenericKnownSettings; *pGenericKnowSetting; ++pGenericKnowSetting )
        if ( aDSUI.hasSetting( *pGenericKnowSetting ) )
            _out_rDetailsIds.push_back( *pGenericKnowSetting );

    // the rest is hard-coded. On the long run, all of this should be done via DataSourceUI::hasSetting
    switch ( _eType )
    {
        case DST_DBASE:
            _out_rDetailsIds.push_back(DSID_SHOWDELETEDROWS);
            _out_rDetailsIds.push_back(DSID_CHARSET);
            break;

        case DST_FLAT:
            _out_rDetailsIds.push_back(DSID_FIELDDELIMITER);
            _out_rDetailsIds.push_back(DSID_TEXTDELIMITER);
            _out_rDetailsIds.push_back(DSID_DECIMALDELIMITER);
            _out_rDetailsIds.push_back(DSID_THOUSANDSDELIMITER);
            _out_rDetailsIds.push_back(DSID_TEXTFILEEXTENSION);
            _out_rDetailsIds.push_back(DSID_TEXTFILEHEADER);
            _out_rDetailsIds.push_back(DSID_CHARSET);
            break;

        case DST_ADABAS:
            _out_rDetailsIds.push_back(DSID_CHARSET);
            _out_rDetailsIds.push_back(DSID_CONN_SHUTSERVICE);
            _out_rDetailsIds.push_back(DSID_CONN_DATAINC);
            _out_rDetailsIds.push_back(DSID_CONN_CACHESIZE);
            _out_rDetailsIds.push_back(DSID_CONN_CTRLUSER);
            _out_rDetailsIds.push_back(DSID_CONN_CTRLPWD);
            break;

        case DST_ADO:
            _out_rDetailsIds.push_back(DSID_CHARSET);
            break;

        case DST_ODBC:
            _out_rDetailsIds.push_back(DSID_ADDITIONALOPTIONS);
            _out_rDetailsIds.push_back(DSID_CHARSET);
            _out_rDetailsIds.push_back(DSID_USECATALOG);
            break;

        case DST_MYSQL_NATIVE:
            _out_rDetailsIds.push_back(DSID_CHARSET);
            break;
        case DST_MYSQL_JDBC:
            _out_rDetailsIds.push_back(DSID_CHARSET);
            _out_rDetailsIds.push_back(DSID_JDBCDRIVERCLASS);
            break;

        case DST_MYSQL_ODBC:
            _out_rDetailsIds.push_back(DSID_CHARSET);
            break;

        case DST_LDAP:
            _out_rDetailsIds.push_back(DSID_CONN_LDAP_BASEDN);
            _out_rDetailsIds.push_back(DSID_CONN_LDAP_ROWCOUNT);
            _out_rDetailsIds.push_back(DSID_CONN_LDAP_USESSL);
            break;

        case DST_JDBC:
            _out_rDetailsIds.push_back(DSID_JDBCDRIVERCLASS);
            break;

        case DST_ORACLE_JDBC:
            _out_rDetailsIds.push_back(DSID_JDBCDRIVERCLASS);
            _out_rDetailsIds.push_back(DSID_IGNORECURRENCY);
            break;

        default:
            break;
    }
}
