/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <sal/types.h>
#include <svl/typedwhich.hxx>

class OptionalBoolItem;
class SfxBoolItem;
class SfxInt32Item;
class SfxStringItem;

typedef sal_Int32   ItemID;

// item ids for the data source administration dialog

#define DSID_NAME                   TypedWhichId<SfxStringItem>(1) // name of a data source, SfxStringItem
#define DSID_ORIGINALNAME           TypedWhichId<SfxStringItem>(2) // original name, internal, SfxStringItem
#define DSID_CONNECTURL             TypedWhichId<SfxStringItem>(3) // connection URL, SfxStringItem
#define DSID_TABLEFILTER            4           // table filter, OStringListItem
#define DSID_TYPECOLLECTION         5           // collection of data source types, ODsnTypeCollection
#define DSID_INVALID_SELECTION      TypedWhichId<SfxBoolItem>(6) // is the selection (thus the set data) invalid?, SfxBoolItem
#define DSID_READONLY               TypedWhichId<SfxBoolItem>(7) // is the selection (thus the set data) readonly?, SfxBoolItem
#define DSID_USER                   TypedWhichId<SfxStringItem>(8) // the user name used for logon, SfxStringItem
#define DSID_PASSWORD               TypedWhichId<SfxStringItem>(9) // the password used for logon, SfxStringItem
#define DSID_ADDITIONALOPTIONS      TypedWhichId<SfxStringItem>(10) // additional options used for connecting, SfxStringItem
#define DSID_CHARSET                TypedWhichId<SfxStringItem>(11) // character set to use, SfxStringItem by now
#define DSID_PASSWORDREQUIRED       TypedWhichId<SfxBoolItem>(12) // is the password required to connect?, SfxBoolItem
#define DSID_SHOWDELETEDROWS        TypedWhichId<SfxBoolItem>(13) // show deleted rows?, SfxBoolItem
#define DSID_ALLOWLONGTABLENAMES    TypedWhichId<SfxBoolItem>(14) // allow tables names longer than 8.3?, SfxBoolItem
#define DSID_JDBCDRIVERCLASS        TypedWhichId<SfxStringItem>(15) // JDBC driver class, SfxStringItem
#define DSID_FIELDDELIMITER         TypedWhichId<SfxStringItem>(16) // field delimiter, SfxUInt16Item
#define DSID_TEXTDELIMITER          TypedWhichId<SfxStringItem>(17) // text delimiter, SfxUInt16Item
#define DSID_DECIMALDELIMITER       TypedWhichId<SfxStringItem>(18) // decimal delimiter, SfxUInt16Item
#define DSID_THOUSANDSDELIMITER     TypedWhichId<SfxStringItem>(19) // thousands delimiter, SfxUInt16Item
#define DSID_TEXTFILEEXTENSION      TypedWhichId<SfxStringItem>(20) // extension for text files, SfxStringItem
#define DSID_TEXTFILEHEADER         TypedWhichId<SfxBoolItem>(21) // the text file contains a header?, SfxBoolItem
#define DSID_PARAMETERNAMESUBST     TypedWhichId<SfxBoolItem>(22)
#define DSID_CONN_PORTNUMBER        TypedWhichId<SfxInt32Item>(23)
#define DSID_SUPPRESSVERSIONCL      TypedWhichId<SfxBoolItem>(24) // meta data: sal_True if the data source described by the set is to-be-deleted
#define DSID_CONN_SHUTSERVICE       TypedWhichId<SfxBoolItem>(25)
#define DSID_CONN_DATAINC           TypedWhichId<SfxInt32Item>(26)
#define DSID_CONN_CACHESIZE         TypedWhichId<SfxInt32Item>(27)
#define DSID_CONN_CTRLUSER          TypedWhichId<SfxStringItem>(28)
#define DSID_CONN_CTRLPWD           TypedWhichId<SfxStringItem>(29)
#define DSID_USECATALOG             TypedWhichId<SfxBoolItem>(30) // should the driver use the catalog name when the database is filebased
#define DSID_CONN_HOSTNAME          TypedWhichId<SfxStringItem>(31)
#define DSID_CONN_LDAP_BASEDN       TypedWhichId<SfxStringItem>(32)
#define DSID_CONN_LDAP_PORTNUMBER   TypedWhichId<SfxInt32Item>(33)
#define DSID_CONN_LDAP_ROWCOUNT     TypedWhichId<SfxInt32Item>(34)
#define DSID_SQL92CHECK             TypedWhichId<SfxBoolItem>(35)
#define DSID_AUTOINCREMENTVALUE     TypedWhichId<SfxStringItem>(36)
#define DSID_AUTORETRIEVEVALUE      TypedWhichId<SfxStringItem>(37)
#define DSID_AUTORETRIEVEENABLED    TypedWhichId<SfxBoolItem>(38)
#define DSID_APPEND_TABLE_ALIAS     TypedWhichId<SfxBoolItem>(39)
#define DSID_MYSQL_PORTNUMBER       TypedWhichId<SfxInt32Item>(40)
#define DSID_IGNOREDRIVER_PRIV      TypedWhichId<SfxBoolItem>(41)
#define DSID_BOOLEANCOMPARISON      TypedWhichId<SfxInt32Item>(42)
#define DSID_ORACLE_PORTNUMBER      TypedWhichId<SfxInt32Item>(43)
#define DSID_ENABLEOUTERJOIN        TypedWhichId<SfxBoolItem>(44)
#define DSID_CATALOG                TypedWhichId<SfxBoolItem>(45)
#define DSID_SCHEMA                 TypedWhichId<SfxBoolItem>(46)
#define DSID_INDEXAPPENDIX          TypedWhichId<SfxBoolItem>(47)
#define DSID_CONN_LDAP_USESSL       TypedWhichId<SfxBoolItem>(48)
#define DSID_DOCUMENT_URL           TypedWhichId<SfxStringItem>(49)
#define DSID_DOSLINEENDS            TypedWhichId<SfxBoolItem>(50)
#define DSID_DATABASENAME           TypedWhichId<SfxStringItem>(51)
#define DSID_AS_BEFORE_CORRNAME     TypedWhichId<SfxBoolItem>(52)
#define DSID_CHECK_REQUIRED_FIELDS  TypedWhichId<SfxBoolItem>(53)
#define DSID_IGNORECURRENCY         TypedWhichId<SfxBoolItem>(54)
#define DSID_CONN_SOCKET            TypedWhichId<SfxStringItem>(55)
#define DSID_ESCAPE_DATETIME        TypedWhichId<SfxBoolItem>(56)
#define DSID_NAMED_PIPE             TypedWhichId<SfxStringItem>(57)
#define DSID_PRIMARY_KEY_SUPPORT    TypedWhichId<OptionalBoolItem>(58)
#define DSID_MAX_ROW_SCAN           TypedWhichId<SfxInt32Item>(59)
#define DSID_RESPECTRESULTSETTYPE   TypedWhichId<SfxBoolItem>(60)
#define DSID_POSTGRES_PORTNUMBER    TypedWhichId<SfxInt32Item>(61)
    // don't forget to adjust DSID_LAST_ITEM_ID below!

// item range. Adjust this if you introduce new items above

#define DSID_FIRST_ITEM_ID  DSID_NAME
#define DSID_LAST_ITEM_ID   DSID_POSTGRES_PORTNUMBER

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
