/*************************************************************************
 *
 *  $RCSfile: dsitems.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: oj $ $Date: 2002-07-25 07:00:13 $
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
#define DSID_NEWDATASOURCE          22          // meta data: sal_True if the data source described by the set is new
#define DSID_DELETEDDATASOURCE      23          // meta data: sal_True if the data source described by the set is to-be-deleted
#define DSID_SUPPRESSVERSIONCL      24          // meta data: sal_True if the data source described by the set is to-be-deleted
#define DSID_DATASOURCE_UNO         25          // meta data: OPropertySetItem, the data source the set represents
#define DSID_CONN_SHUTSERVICE       26
#define DSID_CONN_DATAINC           27
#define DSID_CONN_CACHESIZE         28
#define DSID_CONN_CTRLUSER          29
#define DSID_CONN_CTRLPWD           30
#define DSID_USECATALOG             31          // should the driver use the catalog name when the database is filebased
#define DSID_CONN_LDAP_HOSTNAME     32
#define DSID_CONN_LDAP_BASEDN       33
#define DSID_CONN_LDAP_PORTNUMBER   34
#define DSID_CONN_LDAP_ROWCOUNT     35
#define DSID_SQL92CHECK             36
#define DSID_AUTOINCREMENTVALUE     37
#define DSID_AUTORETRIEVEVALUE      38
#define DSID_AUTORETRIEVEENABLED    39

//========================================================================
//= item range. Adjust this if you introduce new items above

#define DSID_FIRST_ITEM_ID  DSID_NAME
#define DSID_LAST_ITEM_ID   DSID_AUTORETRIEVEENABLED

#endif // _DBAUI_DATASOURCEITEMS_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.11  2002/07/09 12:39:11  oj
 *  #99921# check if datasource allows to check names
 *
 *  Revision 1.10  2001/06/25 08:27:36  oj
 *  #88699# new control for ldap rowcount
 *
 *  Revision 1.9  2001/05/29 13:11:52  oj
 *  #87149# addressbook ui impl
 *
 *  Revision 1.8  2001/04/20 13:38:06  oj
 *  #85736# new checkbox for odbc
 *
 *  Revision 1.7  2001/03/27 07:34:29  oj
 *  impl new page for adabas
 *
 *  Revision 1.6  2001/02/05 13:57:41  fs
 *  #83430# +DSID_DATASOURCE_UNO
 *
 *  Revision 1.5  2000/10/20 09:53:17  fs
 *  handling for the SuppresVersionColumns property of a data source
 *
 *  Revision 1.4  2000/10/12 16:20:42  fs
 *  new implementations ... still under construction
 *
 *  Revision 1.3  2000/10/11 11:31:03  fs
 *  new implementations - still under construction
 *
 *  Revision 1.2  2000/10/09 12:39:29  fs
 *  some (a lot of) new imlpementations - still under development
 *
 *  Revision 1.1  2000/10/05 10:05:55  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 22.09.00 08:10:45  fs
 ************************************************************************/

