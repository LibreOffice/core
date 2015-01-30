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
        OPropertyMap::~OPropertyMap()
        {
            ::std::map<sal_Int32 , rtl_uString*>::iterator aIter = m_aPropertyMap.begin();
            for(;aIter != m_aPropertyMap.end();++aIter)
                if(aIter->second)
                    rtl_uString_release(aIter->second);
        }

        OUString OPropertyMap::getNameByIndex(sal_Int32 _nIndex) const
        {
            OUString sRet;
            ::std::map<sal_Int32 , rtl_uString*>::const_iterator aIter = m_aPropertyMap.find(_nIndex);
            if(aIter == m_aPropertyMap.end())
                sRet = const_cast<OPropertyMap*>(this)->fillValue(_nIndex);
            else
                sRet = aIter->second;
            return sRet;
        }

        OUString OPropertyMap::fillValue(sal_Int32 _nIndex)
        {
            rtl_uString* pStr = NULL;
            switch(_nIndex)
            {
                case PROPERTY_ID_QUERYTIMEOUT:              { rtl_uString_newFromAscii(&pStr, "QueryTimeOut"         ); break; }
                case PROPERTY_ID_MAXFIELDSIZE:              { rtl_uString_newFromAscii(&pStr, "MaxFieldSize"         ); break; }
                case PROPERTY_ID_MAXROWS:                   { rtl_uString_newFromAscii(&pStr, "MaxRows"              ); break; }
                case PROPERTY_ID_CURSORNAME:                { rtl_uString_newFromAscii(&pStr, "CursorName"           ); break; }
                case PROPERTY_ID_RESULTSETCONCURRENCY:      { rtl_uString_newFromAscii(&pStr, "ResultSetConcurrency" ); break; }
                case PROPERTY_ID_RESULTSETTYPE:             { rtl_uString_newFromAscii(&pStr, "ResultSetType"        ); break; }
                case PROPERTY_ID_FETCHDIRECTION:            { rtl_uString_newFromAscii(&pStr, "FetchDirection"       ); break; }
                case PROPERTY_ID_FETCHSIZE:                 { rtl_uString_newFromAscii(&pStr, "FetchSize"            ); break; }
                case PROPERTY_ID_ESCAPEPROCESSING:          { rtl_uString_newFromAscii(&pStr, "EscapeProcessing"     ); break; }
                case PROPERTY_ID_USEBOOKMARKS:              { rtl_uString_newFromAscii(&pStr, "UseBookmarks"         ); break; }
                // Column
                case PROPERTY_ID_NAME:                      { rtl_uString_newFromAscii(&pStr, "Name"                 ); break; }
                case PROPERTY_ID_TYPE:                      { rtl_uString_newFromAscii(&pStr, "Type"                 ); break; }
                case PROPERTY_ID_TYPENAME:                  { rtl_uString_newFromAscii(&pStr, "TypeName"             ); break; }
                case PROPERTY_ID_PRECISION:                 { rtl_uString_newFromAscii(&pStr, "Precision"            ); break; }
                case PROPERTY_ID_SCALE:                     { rtl_uString_newFromAscii(&pStr, "Scale"                ); break; }
                case PROPERTY_ID_ISNULLABLE:                { rtl_uString_newFromAscii(&pStr, "IsNullable"           ); break; }
                case PROPERTY_ID_ISAUTOINCREMENT:           { rtl_uString_newFromAscii(&pStr, "IsAutoIncrement"      ); break; }
                case PROPERTY_ID_ISROWVERSION:              { rtl_uString_newFromAscii(&pStr, "IsRowVersion"         ); break; }
                case PROPERTY_ID_DESCRIPTION:               { rtl_uString_newFromAscii(&pStr, "Description"          ); break; }
                case PROPERTY_ID_DEFAULTVALUE:              { rtl_uString_newFromAscii(&pStr, "DefaultValue"         ); break; }

                case PROPERTY_ID_REFERENCEDTABLE:           { rtl_uString_newFromAscii(&pStr, "ReferencedTable"      ); break; }
                case PROPERTY_ID_UPDATERULE:                { rtl_uString_newFromAscii(&pStr, "UpdateRule"           ); break; }
                case PROPERTY_ID_DELETERULE:                { rtl_uString_newFromAscii(&pStr, "DeleteRule"           ); break; }
                case PROPERTY_ID_CATALOG:                   { rtl_uString_newFromAscii(&pStr, "Catalog"              ); break; }
                case PROPERTY_ID_ISUNIQUE:                  { rtl_uString_newFromAscii(&pStr, "IsUnique"             ); break; }
                case PROPERTY_ID_ISPRIMARYKEYINDEX:         { rtl_uString_newFromAscii(&pStr, "IsPrimaryKeyIndex"    ); break; }
                case PROPERTY_ID_ISCLUSTERED:               { rtl_uString_newFromAscii(&pStr, "IsClustered"          ); break; }
                case PROPERTY_ID_ISASCENDING:               { rtl_uString_newFromAscii(&pStr, "IsAscending"          ); break; }
                case PROPERTY_ID_SCHEMANAME:                { rtl_uString_newFromAscii(&pStr, "SchemaName"           ); break; }
                case PROPERTY_ID_CATALOGNAME:               { rtl_uString_newFromAscii(&pStr, "CatalogName"          ); break; }

                case PROPERTY_ID_COMMAND:                   { rtl_uString_newFromAscii(&pStr, "Command"              ); break; }
                case PROPERTY_ID_CHECKOPTION:               { rtl_uString_newFromAscii(&pStr, "CheckOption"          ); break; }
                case PROPERTY_ID_PASSWORD:                  { rtl_uString_newFromAscii(&pStr, "Password"             ); break; }
                case PROPERTY_ID_RELATEDCOLUMN:             { rtl_uString_newFromAscii(&pStr, "RelatedColumn"        ); break; }

                case PROPERTY_ID_FUNCTION:                  { rtl_uString_newFromAscii(&pStr, "Function"             ); break; }
                case PROPERTY_ID_AGGREGATEFUNCTION:         { rtl_uString_newFromAscii(&pStr, "AggregateFunction"    ); break; }
                case PROPERTY_ID_TABLENAME:                 { rtl_uString_newFromAscii(&pStr, "TableName"            ); break; }
                case PROPERTY_ID_REALNAME:                  { rtl_uString_newFromAscii(&pStr, "RealName"             ); break; }
                case PROPERTY_ID_DBASEPRECISIONCHANGED:     { rtl_uString_newFromAscii(&pStr, "DbasePrecisionChanged"); break; }
                case PROPERTY_ID_ISCURRENCY:                { rtl_uString_newFromAscii(&pStr, "IsCurrency"           ); break; }
                case PROPERTY_ID_ISBOOKMARKABLE:            { rtl_uString_newFromAscii(&pStr, "IsBookmarkable"       ); break; }
                case PROPERTY_ID_HY010:                     { rtl_uString_newFromAscii(&pStr, "HY010"                ); break; }
                case PROPERTY_ID_DELIMITER:                 { rtl_uString_newFromAscii(&pStr, "/"                    ); break; }
                case PROPERTY_ID_FORMATKEY:                 { rtl_uString_newFromAscii(&pStr, "FormatKey"            ); break; }
                case PROPERTY_ID_LOCALE:                    { rtl_uString_newFromAscii(&pStr, "Locale"               ); break; }
                case PROPERTY_ID_AUTOINCREMENTCREATION:     { rtl_uString_newFromAscii(&pStr, "AutoIncrementCreation"); break; }
                case PROPERTY_ID_PRIVILEGES:                { rtl_uString_newFromAscii(&pStr, "Privileges"           ); break; }
                case PROPERTY_ID_HAVINGCLAUSE:              { rtl_uString_newFromAscii(&pStr, "HavingClause"         ); break; }
                case PROPERTY_ID_ISSIGNED:                  { rtl_uString_newFromAscii(&pStr, "IsSigned"             ); break; }
                case PROPERTY_ID_ISSEARCHABLE:              { rtl_uString_newFromAscii(&pStr, "IsSearchable"         ); break; }
                case PROPERTY_ID_LABEL:                     { rtl_uString_newFromAscii(&pStr, "Label"                ); break; }
                case PROPERTY_ID_APPLYFILTER:               { rtl_uString_newFromAscii(&pStr, "ApplyFilter"          ); break; }
                case PROPERTY_ID_FILTER:                    { rtl_uString_newFromAscii(&pStr, "Filter"               ); break; }
                case PROPERTY_ID_MASTERFIELDS:              { rtl_uString_newFromAscii(&pStr, "MasterFields"         ); break; }
                case PROPERTY_ID_DETAILFIELDS:              { rtl_uString_newFromAscii(&pStr, "DetailFields"         ); break; }
                case PROPERTY_ID_FIELDTYPE:                 { rtl_uString_newFromAscii(&pStr, "FieldType"            ); break; }
                case PROPERTY_ID_VALUE:                     { rtl_uString_newFromAscii(&pStr, "Value"                ); break; }
                case PROPERTY_ID_ACTIVE_CONNECTION:         { rtl_uString_newFromAscii(&pStr, "ActiveConnection"     ); break; }
            }
            m_aPropertyMap[_nIndex] = pStr;
            return pStr ? OUString(pStr) : OUString();
        }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
