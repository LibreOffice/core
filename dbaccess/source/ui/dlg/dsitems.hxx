/*************************************************************************
 *
 *  $RCSfile: dsitems.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:47:45 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#define _DBAUI_DATASOURCEITEMS_HXX_

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
#define DSID_DOSLINEENDS            50


//========================================================================
//= item range. Adjust this if you introduce new items above

#define DSID_FIRST_ITEM_ID  DSID_NAME
#define DSID_LAST_ITEM_ID   DSID_DOSLINEENDS

#endif // _DBAUI_DATASOURCEITEMS_HXX_

