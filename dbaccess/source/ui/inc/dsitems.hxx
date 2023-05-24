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

class SfxBoolItem;
class OptionalBoolItem;

typedef sal_Int32   ItemID;

// item ids for the data source administration dialog

#define DSID_NAME                   1           // name of a data source, SfxStringItem
#define DSID_ORIGINALNAME           2           // original name, internal, SfxStringItem
#define DSID_CONNECTURL             3           // connection URL, SfxStringItem
#define DSID_TABLEFILTER            4           // table filter, OStringListItem
#define DSID_TYPECOLLECTION         5           // collection of data source types, ODsnTypeCollection
#define DSID_INVALID_SELECTION      TypedWhichId<SfxBoolItem>(6) // is the selection (thus the set data) invalid?, SfxBoolItem
#define DSID_READONLY               TypedWhichId<SfxBoolItem>(7) // is the selection (thus the set data) readonly?, SfxBoolItem
#define DSID_USER                   8           // the user name used for logon, SfxStringItem
#define DSID_PASSWORD               9           // the password used for logon, SfxStringItem
#define DSID_ADDITIONALOPTIONS      10          // additional options used for connecting, SfxStringItem
#define DSID_CHARSET                11          // character set to use, SfxStringItem by now
#define DSID_PASSWORDREQUIRED       TypedWhichId<SfxBoolItem>(12) // is the password required to connect?, SfxBoolItem
#define DSID_SHOWDELETEDROWS        TypedWhichId<SfxBoolItem>(13) // show deleted rows?, SfxBoolItem
#define DSID_ALLOWLONGTABLENAMES    TypedWhichId<SfxBoolItem>(14) // allow tables names longer than 8.3?, SfxBoolItem
#define DSID_JDBCDRIVERCLASS        15          // JDBC driver class, SfxStringItem
#define DSID_FIELDDELIMITER         16          // field delimiter, SfxUInt16Item
#define DSID_TEXTDELIMITER          17          // text delimiter, SfxUInt16Item
#define DSID_DECIMALDELIMITER       18          // decimal delimiter, SfxUInt16Item
#define DSID_THOUSANDSDELIMITER     19          // thousands delimiter, SfxUInt16Item
#define DSID_TEXTFILEEXTENSION      20          // extension for text files, SfxStringItem
#define DSID_TEXTFILEHEADER         TypedWhichId<SfxBoolItem>(21) // the text file contains a header?, SfxBoolItem
#define DSID_PARAMETERNAMESUBST     TypedWhichId<SfxBoolItem>(22)
#define DSID_CONN_PORTNUMBER        23
#define DSID_SUPPRESSVERSIONCL      TypedWhichId<SfxBoolItem>(24) // meta data: sal_True if the data source described by the set is to-be-deleted
#define DSID_CONN_SHUTSERVICE       TypedWhichId<SfxBoolItem>(25)
#define DSID_CONN_DATAINC           26
#define DSID_CONN_CACHESIZE         27
#define DSID_CONN_CTRLUSER          28
#define DSID_CONN_CTRLPWD           29
#define DSID_USECATALOG             TypedWhichId<SfxBoolItem>(30) // should the driver use the catalog name when the database is filebased
#define DSID_CONN_HOSTNAME          31
#define DSID_CONN_LDAP_BASEDN       32
#define DSID_CONN_LDAP_PORTNUMBER   33
#define DSID_CONN_LDAP_ROWCOUNT     34
#define DSID_SQL92CHECK             TypedWhichId<SfxBoolItem>(35)
#define DSID_AUTOINCREMENTVALUE     36
#define DSID_AUTORETRIEVEVALUE      37
#define DSID_AUTORETRIEVEENABLED    TypedWhichId<SfxBoolItem>(38)
#define DSID_APPEND_TABLE_ALIAS     TypedWhichId<SfxBoolItem>(39)
#define DSID_MYSQL_PORTNUMBER       40
#define DSID_IGNOREDRIVER_PRIV      TypedWhichId<SfxBoolItem>(41)
#define DSID_BOOLEANCOMPARISON      42
#define DSID_ORACLE_PORTNUMBER      43
#define DSID_ENABLEOUTERJOIN        TypedWhichId<SfxBoolItem>(44)
#define DSID_CATALOG                TypedWhichId<SfxBoolItem>(45)
#define DSID_SCHEMA                 TypedWhichId<SfxBoolItem>(46)
#define DSID_INDEXAPPENDIX          TypedWhichId<SfxBoolItem>(47)
#define DSID_CONN_LDAP_USESSL       TypedWhichId<SfxBoolItem>(48)
#define DSID_DOCUMENT_URL           49
#define DSID_DOSLINEENDS            TypedWhichId<SfxBoolItem>(50)
#define DSID_DATABASENAME           51
#define DSID_AS_BEFORE_CORRNAME     TypedWhichId<SfxBoolItem>(52)
#define DSID_CHECK_REQUIRED_FIELDS  TypedWhichId<SfxBoolItem>(53)
#define DSID_IGNORECURRENCY         TypedWhichId<SfxBoolItem>(54)
#define DSID_CONN_SOCKET            55
#define DSID_ESCAPE_DATETIME        TypedWhichId<SfxBoolItem>(56)
#define DSID_NAMED_PIPE             57
#define DSID_PRIMARY_KEY_SUPPORT    TypedWhichId<OptionalBoolItem>(58)
#define DSID_MAX_ROW_SCAN           59
#define DSID_RESPECTRESULTSETTYPE   TypedWhichId<SfxBoolItem>(60)
#define DSID_POSTGRES_PORTNUMBER    61
    // don't forget to adjust DSID_LAST_ITEM_ID below!

// item range. Adjust this if you introduce new items above

#define DSID_FIRST_ITEM_ID  DSID_NAME
#define DSID_LAST_ITEM_ID   DSID_POSTGRES_PORTNUMBER

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
