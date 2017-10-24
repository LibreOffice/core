/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package com.sun.star.sdbcx.comp.postgresql.util;

public enum PropertyIds {
    QUERYTIMEOUT(1, "QueryTimeOut"),
    MAXFIELDSIZE(2, "MaxFieldSize"),
    MAXROWS(3, "MaxRows"),
    CURSORNAME(4, "CursorName"),
    RESULTSETCONCURRENCY(5, "ResultSetConcurrency"),
    RESULTSETTYPE(6, "ResultSetType"),
    FETCHDIRECTION(7, "FetchDirection"),
    FETCHSIZE(8, "FetchSize"),
    ESCAPEPROCESSING(9, "EscapeProcessing"),
    USEBOOKMARKS (10, "UseBookmarks"),
    // Column
    NAME (11, "Name"),
    TYPE (12, "Type"),
    TYPENAME (13, "TypeName"),
    PRECISION (14, "Precision"),
    SCALE (15, "Scale"),
    ISNULLABLE (16, "IsNullable"),
    ISAUTOINCREMENT (17, "IsAutoIncrement"),
    ISROWVERSION (18, "IsRowVersion"),
    DESCRIPTION (19, "Description"),
    DEFAULTVALUE (20, "DefaultValue"),
    REFERENCEDTABLE (21, "ReferencedTable"),
    UPDATERULE (22, "UpdateRule"),
    DELETERULE (23, "DeleteRule"),
    CATALOG (24, "Catalog"),
    ISUNIQUE (25, "IsUnique"),
    ISPRIMARYKEYINDEX (26, "IsPrimaryKeyIndex"),
    ISCLUSTERED (27, "IsClustered"),
    ISASCENDING (28, "IsAscending"),
    SCHEMANAME (29, "SchemaName"),
    CATALOGNAME (30, "CatalogName"),
    COMMAND (31, "Command"),
    CHECKOPTION (32, "CheckOption"),
    PASSWORD (33, "Password"),
    RELATEDCOLUMN (34, "RelatedColumn"),
    FUNCTION (35, "Function"),
    TABLENAME (36, "TableName"),
    REALNAME (37, "RealName"),
    DBASEPRECISIONCHANGED (38, "DbasePrecisionChanged"),
    ISCURRENCY (39, "IsCurrency"),
    ISBOOKMARKABLE (40, "IsBookmarkable"),
    INVALID_INDEX (41, ""),
    HY010 (43, "HY010"),
    LABEL (44, "Label"),
    DELIMITER (45, "/"),
    FORMATKEY (46, "FormatKey"),
    LOCALE (47, "Locale"),
    IM001 (48, ""),
    AUTOINCREMENTCREATION (49, "AutoIncrementCreation"),
    PRIVILEGES (50, "Privileges"),
    HAVINGCLAUSE (51, "HavingClause"),
    ISSIGNED (52, "IsSigned"),
    AGGREGATEFUNCTION (53, "AggregateFunction"),
    ISSEARCHABLE (54, "IsSearchable"),
    APPLYFILTER (55, "ApplyFilter"),
    FILTER (56, "Filter"),
    MASTERFIELDS (57, "MasterFields"),
    DETAILFIELDS (58, "DetailFields"),
    FIELDTYPE (59, "FieldType"),
    VALUE (60, "Value"),
    ACTIVE_CONNECTION (61, "ActiveConnection");


    PropertyIds(int id, String name) {
        this.id = id;
        this.name = name;
    }

    public final int id;
    public final String name;
}
