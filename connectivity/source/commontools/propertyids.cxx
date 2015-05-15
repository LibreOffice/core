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
        const OUString& OPropertyMap::getNameByIndex(sal_Int32 _nIndex) const
        {
            ::std::map<sal_Int32, OUString>::const_iterator aIter = m_aPropertyMap.find(_nIndex);
            if(aIter == m_aPropertyMap.end())
            {
                const_cast<OPropertyMap*>(this)->fillValue(_nIndex);
                aIter = m_aPropertyMap.find(_nIndex);
            }
            return aIter->second;
        }

        void OPropertyMap::fillValue(sal_Int32 _nIndex)
        {
            OUString pStr;
            switch(_nIndex)
            {
                case PROPERTY_ID_QUERYTIMEOUT:              pStr = "QueryTimeOut"; break;
                case PROPERTY_ID_MAXFIELDSIZE:              pStr = "MaxFieldSize"; break;
                case PROPERTY_ID_MAXROWS:                   pStr = "MaxRows"; break;
                case PROPERTY_ID_CURSORNAME:                pStr = "CursorName"; break;
                case PROPERTY_ID_RESULTSETCONCURRENCY:      pStr = "ResultSetConcurrency"; break;

                case PROPERTY_ID_RESULTSETTYPE:             pStr = "ResultSetType"; break;
                case PROPERTY_ID_FETCHDIRECTION:            pStr = "FetchDirection"; break;
                case PROPERTY_ID_FETCHSIZE:                 pStr = "FetchSize"; break;
                case PROPERTY_ID_ESCAPEPROCESSING:          pStr = "EscapeProcessing"; break;
                case PROPERTY_ID_USEBOOKMARKS:              pStr = "UseBookmarks"; break;
                // Column
                case PROPERTY_ID_NAME:                      pStr = "Name"; break;
                case PROPERTY_ID_TYPE:                      pStr = "Type"; break;
                case PROPERTY_ID_TYPENAME:                  pStr = "TypeName"; break;
                case PROPERTY_ID_PRECISION:                 pStr = "Precision"; break;
                case PROPERTY_ID_SCALE:                     pStr = "Scale"; break;
                case PROPERTY_ID_ISNULLABLE:                pStr = "IsNullable"; break;
                case PROPERTY_ID_ISAUTOINCREMENT:           pStr = "IsAutoIncrement"; break;
                case PROPERTY_ID_ISROWVERSION:              pStr = "IsRowVersion"; break;
                case PROPERTY_ID_DESCRIPTION:               pStr = "Description"; break;
                case PROPERTY_ID_DEFAULTVALUE:              pStr = "DefaultValue"; break;

                case PROPERTY_ID_REFERENCEDTABLE:           pStr = "ReferencedTable"; break;
                case PROPERTY_ID_UPDATERULE:                pStr = "UpdateRule"; break;
                case PROPERTY_ID_DELETERULE:                pStr = "DeleteRule"; break;
                case PROPERTY_ID_CATALOG:                   pStr = "Catalog"; break;
                case PROPERTY_ID_ISUNIQUE:                  pStr = "IsUnique"; break;
                case PROPERTY_ID_ISPRIMARYKEYINDEX:         pStr = "IsPrimaryKeyIndex"; break;
                case PROPERTY_ID_ISCLUSTERED:               pStr = "IsClustered"; break;
                case PROPERTY_ID_ISASCENDING:               pStr = "IsAscending"; break;
                case PROPERTY_ID_SCHEMANAME:                pStr = "SchemaName"; break;
                case PROPERTY_ID_CATALOGNAME:               pStr = "CatalogName"; break;

                case PROPERTY_ID_COMMAND:                   pStr = "Command"; break;
                case PROPERTY_ID_CHECKOPTION:               pStr = "CheckOption"; break;
                case PROPERTY_ID_PASSWORD:                  pStr = "Password"; break;
                case PROPERTY_ID_RELATEDCOLUMN:             pStr = "RelatedColumn"; break;

                case PROPERTY_ID_FUNCTION:                  pStr = "Function"; break;
                case PROPERTY_ID_AGGREGATEFUNCTION:         pStr = "AggregateFunction"; break;
                case PROPERTY_ID_TABLENAME:                 pStr = "TableName"; break;
                case PROPERTY_ID_REALNAME:                  pStr = "RealName"; break;
                case PROPERTY_ID_DBASEPRECISIONCHANGED:     pStr = "DbasePrecisionChanged"; break;
                case PROPERTY_ID_ISCURRENCY:                pStr = "IsCurrency"; break;
                case PROPERTY_ID_ISBOOKMARKABLE:            pStr = "IsBookmarkable"; break;
                case PROPERTY_ID_HY010:                     pStr = "HY010"; break; // error messages
                case PROPERTY_ID_DELIMITER:                 pStr = "/"; break;
                case PROPERTY_ID_FORMATKEY:                 pStr = "FormatKey"; break;
                case PROPERTY_ID_LOCALE:                    pStr = "Locale"; break;
                case PROPERTY_ID_AUTOINCREMENTCREATION:     pStr = "AutoIncrementCreation"; break;
                case PROPERTY_ID_PRIVILEGES:                pStr = "Privileges"; break;
                case PROPERTY_ID_HAVINGCLAUSE:              pStr = "HavingClause"; break;
                case PROPERTY_ID_ISSIGNED:                  pStr = "IsSigned"; break;
                case PROPERTY_ID_ISSEARCHABLE:              pStr = "IsSearchable"; break;
                case PROPERTY_ID_LABEL:                     pStr = "Label"; break;
                case PROPERTY_ID_APPLYFILTER:               pStr = "ApplyFilter"; break;
                case PROPERTY_ID_FILTER:                    pStr = "Filter"; break;
                case PROPERTY_ID_MASTERFIELDS:              pStr = "MasterFields"; break;
                case PROPERTY_ID_DETAILFIELDS:              pStr = "DetailFields"; break;
                case PROPERTY_ID_FIELDTYPE:                 pStr = "FieldType"; break;
                case PROPERTY_ID_VALUE:                     pStr = "Value"; break;
                case PROPERTY_ID_ACTIVE_CONNECTION:         pStr = "ActiveConnection"; break;
            }
            m_aPropertyMap[_nIndex] = pStr;
        }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
