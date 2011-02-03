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

#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#define _DBAUI_DATASOURCEITEMS_HXX_

typedef sal_Int32   ItemID;

//========================================================================
//= item ids for the data source administration dialog

#define DSID_NAME                   1           // name of a data source, SfxStringItem
#define DSID_ORIGINALNAME           2           // orginal name, internal, SfxStringItem
#define DSID_CONNECTURL             3           // connection URL, SfxStringItem
#define DSID_TABLEFILTER            4           // table filter, OStringListItem
#define DSID_TYPECOLLECTION         5           // collection of data source types, ODsnTypeCollection
#define DSID_INVALID_SELECTION      6           // is the selection (thus the set data) invalid?, SfxBoolItem
#define DSID_READONLY               7           // is the selection (thus the set data) readonly?, SfxBoolItem
#define DSID_USER                   8           // the user name used for logon, SfxStringItem
#define DSID_PASSWORD               9           // the password used for logon, SfxStringItem
#define DSID_ADDITIONALOPTIONS      10          // additional options used for connecting, SfxStringItem
#define DSID_CHARSET                11          // character set to use, SfxStringItem by now
#define DSID_PASSWORDREQUIRED       12          // is the password required to connect?, SfxBoolItem
#define DSID_SHOWDELETEDROWS        13          // show deleted rows?, SfxBoolItem
#define DSID_ALLOWLONGTABLENAMES    14          // allow tables names longer than 8.3?, SfxBoolItem
#define DSID_JDBCDRIVERCLASS        15          // JDBC driver class, SfxStringItem
#define DSID_FIELDDELIMITER         16          // field delimiter, SfxUInt16Item
#define DSID_TEXTDELIMITER          17          // text delimiter, SfxUInt16Item
#define DSID_DECIMALDELIMITER       18          // decimal delimiter, SfxUInt16Item
#define DSID_THOUSANDSDELIMITER     19          // thousands delimiter, SfxUInt16Item
#define DSID_TEXTFILEEXTENSION      20          // extension for text files, SfxStringItem
#define DSID_TEXTFILEHEADER         21          // the text file contains a header?, SfxBoolItem
#define DSID_PARAMETERNAMESUBST     22
#define DSID_CONN_PORTNUMBER        23
#define DSID_SUPPRESSVERSIONCL      24          // meta data: sal_True if the data source described by the set is to-be-deleted
#define DSID_DATASOURCE_UNO         25          // meta data: OPropertySetItem, the data source the set represents
#define DSID_CONN_SHUTSERVICE       26
#define DSID_CONN_DATAINC           27
#define DSID_CONN_CACHESIZE         28
#define DSID_CONN_CTRLUSER          29
#define DSID_CONN_CTRLPWD           30
#define DSID_USECATALOG             31          // should the driver use the catalog name when the database is filebased
#define DSID_CONN_HOSTNAME          32
#define DSID_CONN_LDAP_BASEDN       33
#define DSID_CONN_LDAP_PORTNUMBER   34
#define DSID_CONN_LDAP_ROWCOUNT     35
#define DSID_SQL92CHECK             36
#define DSID_AUTOINCREMENTVALUE     37
#define DSID_AUTORETRIEVEVALUE      38
#define DSID_AUTORETRIEVEENABLED    39
#define DSID_APPEND_TABLE_ALIAS     40
#define DSID_MYSQL_PORTNUMBER       41
#define DSID_IGNOREDRIVER_PRIV      42
#define DSID_BOOLEANCOMPARISON      43
#define DSID_ORACLE_PORTNUMBER      44
#define DSID_ENABLEOUTERJOIN        45
#define DSID_CATALOG                46
#define DSID_SCHEMA                 47
#define DSID_INDEXAPPENDIX          48
#define DSID_CONN_LDAP_USESSL       49
#define DSID_DOCUMENT_URL           50
#define DSID_DOSLINEENDS            51
#define DSID_DATABASENAME           52
#define DSID_AS_BEFORE_CORRNAME     53
#define DSID_CHECK_REQUIRED_FIELDS  54
#define DSID_IGNORECURRENCY         55
#define DSID_CONN_SOCKET            56
#define DSID_ESCAPE_DATETIME        57
#define DSID_NAMED_PIPE             58
#define DSID_PRIMARY_KEY_SUPPORT    59
#define DSID_MAX_ROW_SCAN           60
    // don't forget to adjust DSID_LAST_ITEM_ID below!


//========================================================================
//= item range. Adjust this if you introduce new items above

#define DSID_FIRST_ITEM_ID  DSID_NAME
#define DSID_LAST_ITEM_ID   DSID_MAX_ROW_SCAN

#endif // _DBAUI_DATASOURCEITEMS_HXX_

