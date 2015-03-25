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

#ifndef INCLUDED_MYSQLC_SOURCE_MYSQLC_PROPERTYIDS_HXX
#define INCLUDED_MYSQLC_SOURCE_MYSQLC_PROPERTYIDS_HXX

// this define has to be set to split the names into different dll's or so's
// every dll has his own set of property names
#include <rtl/ustring.hxx>
#include <map>

namespace connectivity
{
namespace mysqlc
{
enum
{
    PROPERTY_ID_FIRST = 0,
    PROPERTY_ID_QUERYTIMEOUT,
    PROPERTY_ID_MAXFIELDSIZE,
    PROPERTY_ID_MAXROWS,
    PROPERTY_ID_CURSORNAME,
    PROPERTY_ID_RESULTSETCONCURRENCY,
    PROPERTY_ID_RESULTSETTYPE,
    PROPERTY_ID_FETCHDIRECTION,
    PROPERTY_ID_FETCHSIZE,
    PROPERTY_ID_ESCAPEPROCESSING,
    PROPERTY_ID_USEBOOKMARKS,
// Column
    PROPERTY_ID_NAME,
    PROPERTY_ID_TYPE,
    PROPERTY_ID_TYPENAME,
    PROPERTY_ID_PRECISION,
    PROPERTY_ID_SCALE,
    PROPERTY_ID_ISNULLABLE,
    PROPERTY_ID_ISAUTOINCREMENT,
    PROPERTY_ID_ISROWVERSION,
    PROPERTY_ID_DESCRIPTION,
    PROPERTY_ID_DEFAULTVALUE,

    PROPERTY_ID_REFERENCEDTABLE,
    PROPERTY_ID_UPDATERULE,
    PROPERTY_ID_DELETERULE,
    PROPERTY_ID_CATALOG,
    PROPERTY_ID_ISUNIQUE,
    PROPERTY_ID_ISPRIMARYKEYINDEX,
    PROPERTY_ID_ISCLUSTERED,
    PROPERTY_ID_ISASCENDING,
    PROPERTY_ID_SCHEMANAME,
    PROPERTY_ID_CATALOGNAME,

    PROPERTY_ID_COMMAND,
    PROPERTY_ID_CHECKOPTION,
    PROPERTY_ID_PASSWORD,
    PROPERTY_ID_RELATEDCOLUMN,

    PROPERTY_ID_FUNCTION,
    PROPERTY_ID_TABLENAME,
    PROPERTY_ID_REALNAME,
    PROPERTY_ID_DBASEPRECISIONCHANGED,
    PROPERTY_ID_ISCURRENCY,
    PROPERTY_ID_ISBOOKMARKABLE,

    PROPERTY_ID_INVALID_INDEX,
    PROPERTY_ID_ERRORMSG_SEQUENCE,
    PROPERTY_ID_HY010,
    PROPERTY_ID_HY0000,
    PROPERTY_ID_DELIMITER,
    PROPERTY_ID_FORMATKEY,
    PROPERTY_ID_LOCALE,
    PROPERTY_ID_IM001,

    PROPERTY_ID_AUTOINCREMENTCREATION,

    PROPERTY_ID_PRIVILEGES,

    PROPERTY_ID_LAST
};
}/* mysqlc */
}/* connectivity */

#endif // INCLUDED_MYSQLC_SOURCE_MYSQLC_PROPERTYIDS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
