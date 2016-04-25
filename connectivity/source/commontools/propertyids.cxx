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


#include "propertyids.hxx"

namespace dbtools
{
    OPropertyMap::OPropertyMap()
    {
        // MSVC complains about ambiguous operator=
        m_aPropertyMap.insert({
                {PROPERTY_ID_QUERYTIMEOUT,         "QueryTimeOut"},
                {PROPERTY_ID_MAXFIELDSIZE,         "MaxFieldSize"},
                {PROPERTY_ID_MAXROWS,              "MaxRows"},
                {PROPERTY_ID_CURSORNAME,           "CursorName"},
                {PROPERTY_ID_RESULTSETCONCURRENCY, "ResultSetConcurrency"},

                {PROPERTY_ID_RESULTSETTYPE,        "ResultSetType"},
                {PROPERTY_ID_FETCHDIRECTION,       "FetchDirection"},
                {PROPERTY_ID_FETCHSIZE,            "FetchSize"},
                {PROPERTY_ID_ESCAPEPROCESSING,     "EscapeProcessing"},
                {PROPERTY_ID_USEBOOKMARKS,         "UseBookmarks"},
                // Column
                {PROPERTY_ID_NAME,                 "Name"},
                {PROPERTY_ID_TYPE,                 "Type"},
                {PROPERTY_ID_TYPENAME,             "TypeName"},
                {PROPERTY_ID_PRECISION,            "Precision"},
                {PROPERTY_ID_SCALE,                "Scale"},
                {PROPERTY_ID_ISNULLABLE,           "IsNullable"},
                {PROPERTY_ID_ISAUTOINCREMENT,      "IsAutoIncrement"},
                {PROPERTY_ID_ISROWVERSION,         "IsRowVersion"},
                {PROPERTY_ID_DESCRIPTION,          "Description"},
                {PROPERTY_ID_DEFAULTVALUE,         "DefaultValue"},

                {PROPERTY_ID_REFERENCEDTABLE,      "ReferencedTable"},
                {PROPERTY_ID_UPDATERULE,           "UpdateRule"},
                {PROPERTY_ID_DELETERULE,           "DeleteRule"},
                {PROPERTY_ID_CATALOG,              "Catalog"},
                {PROPERTY_ID_ISUNIQUE,             "IsUnique"},
                {PROPERTY_ID_ISPRIMARYKEYINDEX,    "IsPrimaryKeyIndex"},
                {PROPERTY_ID_ISCLUSTERED,          "IsClustered"},
                {PROPERTY_ID_ISASCENDING,          "IsAscending"},
                {PROPERTY_ID_SCHEMANAME,           "SchemaName"},
                {PROPERTY_ID_CATALOGNAME,          "CatalogName"},

                {PROPERTY_ID_COMMAND,              "Command"},
                {PROPERTY_ID_CHECKOPTION,          "CheckOption"},
                {PROPERTY_ID_PASSWORD,             "Password"},
                {PROPERTY_ID_RELATEDCOLUMN,        "RelatedColumn"},

                {PROPERTY_ID_FUNCTION,             "Function"},
                {PROPERTY_ID_AGGREGATEFUNCTION,    "AggregateFunction"},
                {PROPERTY_ID_TABLENAME,            "TableName"},
                {PROPERTY_ID_REALNAME,             "RealName"},
                {PROPERTY_ID_DBASEPRECISIONCHANGED,"DbasePrecisionChanged"},
                {PROPERTY_ID_ISCURRENCY,           "IsCurrency"},
                {PROPERTY_ID_ISBOOKMARKABLE,       "IsBookmarkable"},
                {PROPERTY_ID_HY010,                "HY010"}, // error messages
                {PROPERTY_ID_DELIMITER,            "/"},
                {PROPERTY_ID_FORMATKEY,            "FormatKey"},
                {PROPERTY_ID_LOCALE,               "Locale"},
                {PROPERTY_ID_AUTOINCREMENTCREATION, "AutoIncrementCreation"},
                {PROPERTY_ID_PRIVILEGES,           "Privileges"},
                {PROPERTY_ID_HAVINGCLAUSE,         "HavingClause"},
                {PROPERTY_ID_ISSIGNED,             "IsSigned"},
                {PROPERTY_ID_ISSEARCHABLE,         "IsSearchable"},
                {PROPERTY_ID_LABEL,                "Label"},
                {PROPERTY_ID_APPLYFILTER,          "ApplyFilter"},
                {PROPERTY_ID_FILTER,               "Filter"},
                {PROPERTY_ID_MASTERFIELDS,         "MasterFields"},
                {PROPERTY_ID_DETAILFIELDS,         "DetailFields"},
                {PROPERTY_ID_FIELDTYPE,            "FieldType"},
                {PROPERTY_ID_VALUE,                "Value"},
                {PROPERTY_ID_ACTIVE_CONNECTION,    "ActiveConnection"},
            } );
    }

    const OUString& OPropertyMap::getNameByIndex(sal_Int32 _nIndex) const
    {
        ::std::map<sal_Int32, OUString>::const_iterator aIter = m_aPropertyMap.find(_nIndex);
        return aIter->second;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
