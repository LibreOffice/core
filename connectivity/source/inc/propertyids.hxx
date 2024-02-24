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

// this define has to be set to split the names into different dll's or so's
// every dll has his own set of property names
#include <rtl/ustring.hxx>
#include <map>
#include <connectivity/dbtoolsdllapi.hxx>

namespace dbtools
{
    class OOO_DLLPUBLIC_DBTOOLS OPropertyMap
    {
        std::map<sal_Int32, OUString> m_aPropertyMap;
    public:
        OPropertyMap();
        const OUString& getNameByIndex(sal_Int32 _nIndex) const;
    };
}

enum PropertyId
{
    PROPERTY_ID_QUERYTIMEOUT = 1,
    PROPERTY_ID_MAXFIELDSIZE = 2,
    PROPERTY_ID_MAXROWS = 3,
    PROPERTY_ID_CURSORNAME = 4,
    PROPERTY_ID_RESULTSETCONCURRENCY = 5,
    PROPERTY_ID_RESULTSETTYPE = 6,
    PROPERTY_ID_FETCHDIRECTION = 7,
    PROPERTY_ID_FETCHSIZE = 8,
    PROPERTY_ID_ESCAPEPROCESSING = 9,
    PROPERTY_ID_USEBOOKMARKS = 10,

    // Column
    PROPERTY_ID_NAME = 11,
    PROPERTY_ID_TYPE = 12,
    PROPERTY_ID_TYPENAME = 13,
    PROPERTY_ID_PRECISION = 14,
    PROPERTY_ID_SCALE = 15,
    PROPERTY_ID_ISNULLABLE = 16,
    PROPERTY_ID_ISAUTOINCREMENT = 17,
    PROPERTY_ID_ISROWVERSION = 18,
    PROPERTY_ID_DESCRIPTION = 19,
    PROPERTY_ID_DEFAULTVALUE = 20,

    PROPERTY_ID_REFERENCEDTABLE = 21,
    PROPERTY_ID_UPDATERULE = 22,
    PROPERTY_ID_DELETERULE = 23,
    PROPERTY_ID_CATALOG = 24,
    PROPERTY_ID_ISUNIQUE = 25,
    PROPERTY_ID_ISPRIMARYKEYINDEX = 26,
    PROPERTY_ID_ISCLUSTERED = 27,
    PROPERTY_ID_ISASCENDING = 28,
    PROPERTY_ID_SCHEMANAME = 29,
    PROPERTY_ID_CATALOGNAME = 30,

    PROPERTY_ID_COMMAND = 31,
    PROPERTY_ID_CHECKOPTION = 32,
    PROPERTY_ID_PASSWORD = 33,
    PROPERTY_ID_RELATEDCOLUMN = 34,

    PROPERTY_ID_FUNCTION = 35,
    PROPERTY_ID_TABLENAME = 36,
    PROPERTY_ID_REALNAME = 37,
    PROPERTY_ID_DBASEPRECISIONCHANGED = 38,
    PROPERTY_ID_ISCURRENCY = 39,
    PROPERTY_ID_ISBOOKMARKABLE = 40,

    PROPERTY_ID_INVALID_INDEX = 41,
    PROPERTY_ID_HY010 = 43,
    PROPERTY_ID_LABEL = 44,
    PROPERTY_ID_DELIMITER = 45,
    PROPERTY_ID_FORMATKEY = 46,
    PROPERTY_ID_LOCALE = 47,
    PROPERTY_ID_IM001 = 48,

    PROPERTY_ID_AUTOINCREMENTCREATION = 49,

    PROPERTY_ID_PRIVILEGES = 50,
    PROPERTY_ID_HAVINGCLAUSE = 51,

    PROPERTY_ID_ISSIGNED = 52,
    PROPERTY_ID_AGGREGATEFUNCTION = 53,
    PROPERTY_ID_ISSEARCHABLE = 54,

    PROPERTY_ID_APPLYFILTER = 55,
    PROPERTY_ID_FILTER = 56,
    PROPERTY_ID_MASTERFIELDS = 57,
    PROPERTY_ID_DETAILFIELDS = 58,
    PROPERTY_ID_FIELDTYPE = 59,
    PROPERTY_ID_VALUE = 60,
    PROPERTY_ID_ACTIVE_CONNECTION = 61
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
