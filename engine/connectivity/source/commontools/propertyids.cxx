/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include <propertyids.hxx>

namespace dbtools
{
    OPropertyMap::OPropertyMap()
    {
        // MSVC complains about ambiguous operator=
        m_aPropertyMap.insert({
                {PROPERTY_ID_QUERYTIMEOUT,         u"QueryTimeOut"_ustr},
                {PROPERTY_ID_MAXFIELDSIZE,         u"MaxFieldSize"_ustr},
                {PROPERTY_ID_MAXROWS,              u"MaxRows"_ustr},
                {PROPERTY_ID_CURSORNAME,           u"CursorName"_ustr},
                {PROPERTY_ID_RESULTSETCONCURRENCY, u"ResultSetConcurrency"_ustr},

                {PROPERTY_ID_RESULTSETTYPE,        u"ResultSetType"_ustr},
                {PROPERTY_ID_FETCHDIRECTION,       u"FetchDirection"_ustr},
                {PROPERTY_ID_FETCHSIZE,            u"FetchSize"_ustr},
                {PROPERTY_ID_ESCAPEPROCESSING,     u"EscapeProcessing"_ustr},
                {PROPERTY_ID_USEBOOKMARKS,         u"UseBookmarks"_ustr},
                // Column
                {PROPERTY_ID_NAME,                 u"Name"_ustr},
                {PROPERTY_ID_TYPE,                 u"Type"_ustr},
                {PROPERTY_ID_TYPENAME,             u"TypeName"_ustr},
                {PROPERTY_ID_PRECISION,            u"Precision"_ustr},
                {PROPERTY_ID_SCALE,                u"Scale"_ustr},
                {PROPERTY_ID_ISNULLABLE,           u"IsNullable"_ustr},
                {PROPERTY_ID_ISAUTOINCREMENT,      u"IsAutoIncrement"_ustr},
                {PROPERTY_ID_ISROWVERSION,         u"IsRowVersion"_ustr},
                {PROPERTY_ID_DESCRIPTION,          u"Description"_ustr},
                {PROPERTY_ID_DEFAULTVALUE,         u"DefaultValue"_ustr},

                {PROPERTY_ID_REFERENCEDTABLE,      u"ReferencedTable"_ustr},
                {PROPERTY_ID_UPDATERULE,           u"UpdateRule"_ustr},
                {PROPERTY_ID_DELETERULE,           u"DeleteRule"_ustr},
                {PROPERTY_ID_CATALOG,              u"Catalog"_ustr},
                {PROPERTY_ID_ISUNIQUE,             u"IsUnique"_ustr},
                {PROPERTY_ID_ISPRIMARYKEYINDEX,    u"IsPrimaryKeyIndex"_ustr},
                {PROPERTY_ID_ISCLUSTERED,          u"IsClustered"_ustr},
                {PROPERTY_ID_ISASCENDING,          u"IsAscending"_ustr},
                {PROPERTY_ID_SCHEMANAME,           u"SchemaName"_ustr},
                {PROPERTY_ID_CATALOGNAME,          u"CatalogName"_ustr},

                {PROPERTY_ID_COMMAND,              u"Command"_ustr},
                {PROPERTY_ID_CHECKOPTION,          u"CheckOption"_ustr},
                {PROPERTY_ID_PASSWORD,             u"Password"_ustr},
                {PROPERTY_ID_RELATEDCOLUMN,        u"RelatedColumn"_ustr},

                {PROPERTY_ID_FUNCTION,             u"Function"_ustr},
                {PROPERTY_ID_AGGREGATEFUNCTION,    u"AggregateFunction"_ustr},
                {PROPERTY_ID_TABLENAME,            u"TableName"_ustr},
                {PROPERTY_ID_REALNAME,             u"RealName"_ustr},
                {PROPERTY_ID_DBASEPRECISIONCHANGED,u"DbasePrecisionChanged"_ustr},
                {PROPERTY_ID_ISCURRENCY,           u"IsCurrency"_ustr},
                {PROPERTY_ID_ISBOOKMARKABLE,       u"IsBookmarkable"_ustr},
                {PROPERTY_ID_HY010,                u"HY010"_ustr}, // error messages
                {PROPERTY_ID_DELIMITER,            u"/"_ustr},
                {PROPERTY_ID_FORMATKEY,            u"FormatKey"_ustr},
                {PROPERTY_ID_LOCALE,               u"Locale"_ustr},
                {PROPERTY_ID_AUTOINCREMENTCREATION, u"AutoIncrementCreation"_ustr},
                {PROPERTY_ID_PRIVILEGES,           u"Privileges"_ustr},
                {PROPERTY_ID_HAVINGCLAUSE,         u"HavingClause"_ustr},
                {PROPERTY_ID_ISSIGNED,             u"IsSigned"_ustr},
                {PROPERTY_ID_ISSEARCHABLE,         u"IsSearchable"_ustr},
                {PROPERTY_ID_LABEL,                u"Label"_ustr},
                {PROPERTY_ID_APPLYFILTER,          u"ApplyFilter"_ustr},
                {PROPERTY_ID_FILTER,               u"Filter"_ustr},
                {PROPERTY_ID_MASTERFIELDS,         u"MasterFields"_ustr},
                {PROPERTY_ID_DETAILFIELDS,         u"DetailFields"_ustr},
                {PROPERTY_ID_FIELDTYPE,            u"FieldType"_ustr},
                {PROPERTY_ID_VALUE,                u"Value"_ustr},
                {PROPERTY_ID_ACTIVE_CONNECTION,    u"ActiveConnection"_ustr},
            } );
    }

    const OUString& OPropertyMap::getNameByIndex(sal_Int32 _nIndex) const
    {
        std::map<sal_Int32, OUString>::const_iterator aIter = m_aPropertyMap.find(_nIndex);
        assert(aIter != m_aPropertyMap.end());
        return aIter->second;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
