/*************************************************************************
 *
 *  $RCSfile: dsitems.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-09 12:39:29 $
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

//========================================================================
//= item range. Adjust this if you introduce new items above

#define DSID_FIRST_ITEM_ID  DSID_NAME
#define DSID_LAST_ITEM_ID   DSID_TEXTFILEHEADER

#endif // _DBAUI_DATASOURCEITEMS_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/05 10:05:55  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 22.09.00 08:10:45  fs
 ************************************************************************/

