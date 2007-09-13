/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DriverSettings.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:58:30 $
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

#ifndef DBAUI_DRIVERSETTINGS_HXX
#include "DriverSettings.hxx"
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif


using namespace dbaui;
void ODriversSettings::fillDetailIds(DATASOURCE_TYPE _eType,::std::vector< sal_Int32>& _rDetailsIds)
{
    switch ( _eType )
    {
        case DST_DBASE:
            _rDetailsIds.push_back(DSID_SHOWDELETEDROWS);
            _rDetailsIds.push_back(DSID_CHARSET);
            _rDetailsIds.push_back(DSID_SQL92CHECK);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;
        case DST_FLAT:
            _rDetailsIds.push_back(DSID_FIELDDELIMITER);
            _rDetailsIds.push_back(DSID_TEXTDELIMITER);
            _rDetailsIds.push_back(DSID_DECIMALDELIMITER);
            _rDetailsIds.push_back(DSID_THOUSANDSDELIMITER);
            _rDetailsIds.push_back(DSID_TEXTFILEEXTENSION);
            _rDetailsIds.push_back(DSID_TEXTFILEHEADER);
            _rDetailsIds.push_back(DSID_CHARSET);
            _rDetailsIds.push_back(DSID_SQL92CHECK);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_MSACCESS:
            _rDetailsIds.push_back(DSID_SQL92CHECK);
            _rDetailsIds.push_back(DSID_ENABLEOUTERJOIN);
            _rDetailsIds.push_back(DSID_APPEND_TABLE_ALIAS);
            _rDetailsIds.push_back(DSID_BOOLEANCOMPARISON);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_ADABAS:
            _rDetailsIds.push_back(DSID_CHARSET);
            _rDetailsIds.push_back(DSID_SQL92CHECK);
            _rDetailsIds.push_back(DSID_CONN_SHUTSERVICE);
            _rDetailsIds.push_back(DSID_CONN_DATAINC);
            _rDetailsIds.push_back(DSID_CONN_CACHESIZE);
            _rDetailsIds.push_back(DSID_CONN_CTRLUSER);
            _rDetailsIds.push_back(DSID_CONN_CTRLPWD);
            _rDetailsIds.push_back(DSID_APPEND_TABLE_ALIAS);
            _rDetailsIds.push_back(DSID_BOOLEANCOMPARISON);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_ADO:
            _rDetailsIds.push_back(DSID_SQL92CHECK);
            _rDetailsIds.push_back(DSID_PARAMETERNAMESUBST);
            _rDetailsIds.push_back(DSID_CHARSET);
            _rDetailsIds.push_back(DSID_SUPPRESSVERSIONCL);
            _rDetailsIds.push_back(DSID_ENABLEOUTERJOIN);
            _rDetailsIds.push_back(DSID_APPEND_TABLE_ALIAS);
            _rDetailsIds.push_back(DSID_AS_BEFORE_CORRNAME);
            _rDetailsIds.push_back(DSID_BOOLEANCOMPARISON);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;
        case DST_ODBC:
            _rDetailsIds.push_back(DSID_ADDITIONALOPTIONS);
            _rDetailsIds.push_back(DSID_CHARSET);
            _rDetailsIds.push_back(DSID_SQL92CHECK);
            _rDetailsIds.push_back(DSID_USECATALOG);
            _rDetailsIds.push_back(DSID_AUTOINCREMENTVALUE);
            _rDetailsIds.push_back(DSID_AUTORETRIEVEVALUE);
            _rDetailsIds.push_back(DSID_AUTORETRIEVEENABLED);
            _rDetailsIds.push_back(DSID_PARAMETERNAMESUBST);
            _rDetailsIds.push_back(DSID_SUPPRESSVERSIONCL);
            _rDetailsIds.push_back(DSID_ENABLEOUTERJOIN);
            _rDetailsIds.push_back(DSID_CATALOG);
            _rDetailsIds.push_back(DSID_SCHEMA);
            _rDetailsIds.push_back(DSID_APPEND_TABLE_ALIAS);
            _rDetailsIds.push_back(DSID_AS_BEFORE_CORRNAME);
            _rDetailsIds.push_back(DSID_BOOLEANCOMPARISON);
            _rDetailsIds.push_back(DSID_IGNOREDRIVER_PRIV);
            _rDetailsIds.push_back(DSID_INDEXAPPENDIX);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_MYSQL_JDBC:
            _rDetailsIds.push_back(DSID_CHARSET);
            _rDetailsIds.push_back(DSID_JDBCDRIVERCLASS);
            _rDetailsIds.push_back(DSID_SUPPRESSVERSIONCL);
            _rDetailsIds.push_back(DSID_ENABLEOUTERJOIN);
            _rDetailsIds.push_back(DSID_APPEND_TABLE_ALIAS);
            _rDetailsIds.push_back(DSID_BOOLEANCOMPARISON);
            _rDetailsIds.push_back(DSID_PARAMETERNAMESUBST);
            _rDetailsIds.push_back(DSID_IGNOREDRIVER_PRIV);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_MYSQL_ODBC:
            _rDetailsIds.push_back(DSID_CHARSET);
            _rDetailsIds.push_back(DSID_SUPPRESSVERSIONCL);
            _rDetailsIds.push_back(DSID_ENABLEOUTERJOIN);
            _rDetailsIds.push_back(DSID_APPEND_TABLE_ALIAS);
            _rDetailsIds.push_back(DSID_BOOLEANCOMPARISON);
            _rDetailsIds.push_back(DSID_PARAMETERNAMESUBST);
            _rDetailsIds.push_back(DSID_IGNOREDRIVER_PRIV);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_LDAP:
            _rDetailsIds.push_back(DSID_CONN_LDAP_BASEDN);
            _rDetailsIds.push_back(DSID_CONN_LDAP_ROWCOUNT);
            _rDetailsIds.push_back(DSID_CONN_LDAP_USESSL);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_MOZILLA:
        case DST_THUNDERBIRD:
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;
        case DST_EVOLUTION:
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;
        case DST_KAB:
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;
        case DST_MACAB:
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;
        case DST_OUTLOOK:
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_OUTLOOKEXP:
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_JDBC:
            _rDetailsIds.push_back(DSID_JDBCDRIVERCLASS);
            _rDetailsIds.push_back(DSID_SQL92CHECK);
            _rDetailsIds.push_back(DSID_AUTOINCREMENTVALUE);
            _rDetailsIds.push_back(DSID_AUTORETRIEVEVALUE);
            _rDetailsIds.push_back(DSID_AUTORETRIEVEENABLED);
            _rDetailsIds.push_back(DSID_IGNOREDRIVER_PRIV);
            _rDetailsIds.push_back(DSID_PARAMETERNAMESUBST);
            _rDetailsIds.push_back(DSID_SUPPRESSVERSIONCL);
            _rDetailsIds.push_back(DSID_ENABLEOUTERJOIN);
            _rDetailsIds.push_back(DSID_CATALOG);
            _rDetailsIds.push_back(DSID_SCHEMA);
            _rDetailsIds.push_back(DSID_APPEND_TABLE_ALIAS);
            _rDetailsIds.push_back(DSID_AS_BEFORE_CORRNAME);
            _rDetailsIds.push_back(DSID_BOOLEANCOMPARISON);
            _rDetailsIds.push_back(DSID_INDEXAPPENDIX);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_ORACLE_JDBC:
            _rDetailsIds.push_back(DSID_JDBCDRIVERCLASS);
            _rDetailsIds.push_back(DSID_SQL92CHECK);
            _rDetailsIds.push_back(DSID_AUTOINCREMENTVALUE);
            _rDetailsIds.push_back(DSID_AUTORETRIEVEVALUE);
            _rDetailsIds.push_back(DSID_AUTORETRIEVEENABLED);
            _rDetailsIds.push_back(DSID_IGNOREDRIVER_PRIV);
            _rDetailsIds.push_back(DSID_PARAMETERNAMESUBST);
            _rDetailsIds.push_back(DSID_SUPPRESSVERSIONCL);
            _rDetailsIds.push_back(DSID_ENABLEOUTERJOIN);
            _rDetailsIds.push_back(DSID_CATALOG);
            _rDetailsIds.push_back(DSID_SCHEMA);
            _rDetailsIds.push_back(DSID_APPEND_TABLE_ALIAS);
            _rDetailsIds.push_back(DSID_AS_BEFORE_CORRNAME);
            _rDetailsIds.push_back(DSID_BOOLEANCOMPARISON);
            _rDetailsIds.push_back(DSID_INDEXAPPENDIX);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;

        case DST_USERDEFINE1:   /// first user defined driver
        case DST_USERDEFINE2:
        case DST_USERDEFINE3:
        case DST_USERDEFINE4:
        case DST_USERDEFINE5:
        case DST_USERDEFINE6:
        case DST_USERDEFINE7:
        case DST_USERDEFINE8:
        case DST_USERDEFINE9:
        case DST_USERDEFINE10:
        default:
            _rDetailsIds.push_back(DSID_ADDITIONALOPTIONS);
            _rDetailsIds.push_back(DSID_CHARSET);
            _rDetailsIds.push_back(DSID_SQL92CHECK);
            _rDetailsIds.push_back(DSID_USECATALOG);
            _rDetailsIds.push_back(DSID_AUTOINCREMENTVALUE);
            _rDetailsIds.push_back(DSID_AUTORETRIEVEVALUE);
            _rDetailsIds.push_back(DSID_AUTORETRIEVEENABLED);
            _rDetailsIds.push_back(DSID_PARAMETERNAMESUBST);
            _rDetailsIds.push_back(DSID_SUPPRESSVERSIONCL);
            _rDetailsIds.push_back(DSID_ENABLEOUTERJOIN);
            _rDetailsIds.push_back(DSID_CATALOG);
            _rDetailsIds.push_back(DSID_SCHEMA);
            _rDetailsIds.push_back(DSID_APPEND_TABLE_ALIAS);
            _rDetailsIds.push_back(DSID_AS_BEFORE_CORRNAME);
            _rDetailsIds.push_back(DSID_BOOLEANCOMPARISON);
            _rDetailsIds.push_back(DSID_INDEXAPPENDIX);
            _rDetailsIds.push_back(DSID_IGNOREDRIVER_PRIV);
            _rDetailsIds.push_back(DSID_DOSLINEENDS);
            break;
    }
}
