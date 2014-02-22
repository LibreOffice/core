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
    const sal_Char* getPROPERTY_QUERYTIMEOUT()          { return    "QueryTimeOut"; }
        const sal_Char* getPROPERTY_MAXFIELDSIZE()          { return    "MaxFieldSize"; }
        const sal_Char* getPROPERTY_MAXROWS()               { return    "MaxRows"; }
        const sal_Char* getPROPERTY_CURSORNAME()            { return    "CursorName"; }
        const sal_Char* getPROPERTY_RESULTSETCONCURRENCY()  { return    "ResultSetConcurrency"; }
        const sal_Char* getPROPERTY_RESULTSETTYPE()         { return    "ResultSetType"; }
        const sal_Char* getPROPERTY_FETCHDIRECTION()        { return    "FetchDirection"; }
        const sal_Char* getPROPERTY_FETCHSIZE()             { return    "FetchSize"; }
        const sal_Char* getPROPERTY_ESCAPEPROCESSING()      { return    "EscapeProcessing"; }
        const sal_Char* getPROPERTY_USEBOOKMARKS()          { return    "UseBookmarks"; }

        const sal_Char* getPROPERTY_NAME()                  { return    "Name"; }
        const sal_Char* getPROPERTY_TYPE()                  { return    "Type"; }
        const sal_Char* getPROPERTY_TYPENAME()              { return    "TypeName"; }
        const sal_Char* getPROPERTY_PRECISION()             { return    "Precision"; }
        const sal_Char* getPROPERTY_SCALE()                 { return    "Scale"; }
        const sal_Char* getPROPERTY_ISNULLABLE()            { return    "IsNullable"; }
        const sal_Char* getPROPERTY_ISAUTOINCREMENT()       { return    "IsAutoIncrement"; }
        const sal_Char* getPROPERTY_ISROWVERSION()          { return    "IsRowVersion"; }
        const sal_Char* getPROPERTY_DESCRIPTION()           { return    "Description"; }
        const sal_Char* getPROPERTY_DEFAULTVALUE()          { return    "DefaultValue"; }

        const sal_Char* getPROPERTY_REFERENCEDTABLE()       { return    "ReferencedTable"; }
        const sal_Char* getPROPERTY_UPDATERULE()            { return    "UpdateRule"; }
        const sal_Char* getPROPERTY_DELETERULE()            { return    "DeleteRule"; }
        const sal_Char* getPROPERTY_CATALOG()               { return    "Catalog"; }
        const sal_Char* getPROPERTY_ISUNIQUE()              { return    "IsUnique"; }
        const sal_Char* getPROPERTY_ISPRIMARYKEYINDEX()     { return    "IsPrimaryKeyIndex"; }
        const sal_Char* getPROPERTY_ISCLUSTERED()           { return    "IsClustered"; }
        const sal_Char* getPROPERTY_ISASCENDING()           { return    "IsAscending"; }
        const sal_Char* getPROPERTY_SCHEMANAME()            { return    "SchemaName"; }
        const sal_Char* getPROPERTY_CATALOGNAME()           { return    "CatalogName"; }
        const sal_Char* getPROPERTY_COMMAND()               { return    "Command"; }
        const sal_Char* getPROPERTY_CHECKOPTION()           { return    "CheckOption"; }
        const sal_Char* getPROPERTY_PASSWORD()              { return    "Password"; }
        const sal_Char* getPROPERTY_RELATEDCOLUMN()         { return    "RelatedColumn"; }

        const sal_Char* getPROPERTY_FUNCTION()              { return    "Function"; }
        const sal_Char* getPROPERTY_AGGREGATEFUNCTION()     { return    "AggregateFunction"; }
        const sal_Char* getPROPERTY_TABLENAME()             { return    "TableName"; }
        const sal_Char* getPROPERTY_REALNAME()              { return    "RealName"; }
        const sal_Char* getPROPERTY_DBASEPRECISIONCHANGED() { return    "DbasePrecisionChanged"; }
        const sal_Char* getPROPERTY_ISCURRENCY()            { return    "IsCurrency"; }
        const sal_Char* getPROPERTY_ISBOOKMARKABLE()        { return    "IsBookmarkable"; }

        const sal_Char* getPROPERTY_FORMATKEY()             { return    "FormatKey"; }
        const sal_Char* getPROPERTY_LOCALE()                { return    "Locale"; }

        const sal_Char* getPROPERTY_AUTOINCREMENTCREATION() { return    "AutoIncrementCreation"; }
        const sal_Char* getPROPERTY_PRIVILEGES()            { return    "Privileges"; }
        const sal_Char* getPROPERTY_ID_HAVINGCLAUSE()       { return    "HavingClause"; }
        const sal_Char* getPROPERTY_ID_ISSIGNED()           { return    "IsSigned"; }
        const sal_Char* getPROPERTY_ID_ISSEARCHABLE()       { return    "IsSearchable"; }

        const sal_Char* getPROPERTY_ID_APPLYFILTER()        { return    "ApplyFilter"; }
        const sal_Char* getPROPERTY_ID_FILTER()             { return    "Filter"; }
        const sal_Char* getPROPERTY_ID_MASTERFIELDS()       { return    "MasterFields"; }
        const sal_Char* getPROPERTY_ID_DETAILFIELDS()       { return    "DetailFields"; }
        const sal_Char* getPROPERTY_ID_FIELDTYPE()          { return    "FieldType"; }
        const sal_Char* getPROPERTY_ID_VALUE()              { return    "Value"; }
        const sal_Char* getPROPERTY_ID_ACTIVE_CONNECTION()  { return    "ActiveConnection"; }
        const sal_Char* getPROPERTY_ID_LABEL()              { return    "Label"; }

    //============================================================
    //= error messages
    //============================================================
        const sal_Char* getSQLSTATE_SEQUENCE()              { return    "HY010"; }
        const sal_Char* getSTR_DELIMITER()                  { return    "/"; }



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
                case PROPERTY_ID_QUERYTIMEOUT:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_QUERYTIMEOUT()         ); break; }
                case PROPERTY_ID_MAXFIELDSIZE:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_MAXFIELDSIZE()         ); break; }
                case PROPERTY_ID_MAXROWS:                   { rtl_uString_newFromAscii(&pStr,getPROPERTY_MAXROWS()              ); break;       }
                case PROPERTY_ID_CURSORNAME:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_CURSORNAME()           ); break;   }
                case PROPERTY_ID_RESULTSETCONCURRENCY:      { rtl_uString_newFromAscii(&pStr,getPROPERTY_RESULTSETCONCURRENCY() ); break; }
                case PROPERTY_ID_RESULTSETTYPE:             { rtl_uString_newFromAscii(&pStr,getPROPERTY_RESULTSETTYPE()            ); break; }
                case PROPERTY_ID_FETCHDIRECTION:            { rtl_uString_newFromAscii(&pStr,getPROPERTY_FETCHDIRECTION()       ); break; }
                case PROPERTY_ID_FETCHSIZE:                 { rtl_uString_newFromAscii(&pStr,getPROPERTY_FETCHSIZE()                ); break;   }
                case PROPERTY_ID_ESCAPEPROCESSING:          { rtl_uString_newFromAscii(&pStr,getPROPERTY_ESCAPEPROCESSING()     ); break; }
                case PROPERTY_ID_USEBOOKMARKS:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_USEBOOKMARKS()         ); break; }
                // Column
                case PROPERTY_ID_NAME:                      { rtl_uString_newFromAscii(&pStr,getPROPERTY_NAME()             ); break; }
                case PROPERTY_ID_TYPE:                      { rtl_uString_newFromAscii(&pStr,getPROPERTY_TYPE()             ); break; }
                case PROPERTY_ID_TYPENAME:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_TYPENAME()         ); break; }
                case PROPERTY_ID_PRECISION:                 { rtl_uString_newFromAscii(&pStr,getPROPERTY_PRECISION()            ); break; }
                case PROPERTY_ID_SCALE:                     { rtl_uString_newFromAscii(&pStr,getPROPERTY_SCALE()                ); break; }
                case PROPERTY_ID_ISNULLABLE:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISNULLABLE()       ); break; }
                case PROPERTY_ID_ISAUTOINCREMENT:           { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISAUTOINCREMENT()  ); break; }
                case PROPERTY_ID_ISROWVERSION:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISROWVERSION()     ); break; }
                case PROPERTY_ID_DESCRIPTION:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_DESCRIPTION()      ); break; }
                case PROPERTY_ID_DEFAULTVALUE:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_DEFAULTVALUE()     ); break; }

                case PROPERTY_ID_REFERENCEDTABLE:           { rtl_uString_newFromAscii(&pStr,getPROPERTY_REFERENCEDTABLE()  ); break; }
                case PROPERTY_ID_UPDATERULE:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_UPDATERULE()       ); break; }
                case PROPERTY_ID_DELETERULE:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_DELETERULE()       ); break; }
                case PROPERTY_ID_CATALOG:                   { rtl_uString_newFromAscii(&pStr,getPROPERTY_CATALOG()          ); break; }
                case PROPERTY_ID_ISUNIQUE:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISUNIQUE()         ); break; }
                case PROPERTY_ID_ISPRIMARYKEYINDEX:         { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISPRIMARYKEYINDEX()    ); break; }
                case PROPERTY_ID_ISCLUSTERED:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISCLUSTERED()          ); break; }
                case PROPERTY_ID_ISASCENDING:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISASCENDING()          ); break; }
                case PROPERTY_ID_SCHEMANAME:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_SCHEMANAME()           ); break; }
                case PROPERTY_ID_CATALOGNAME:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_CATALOGNAME()          ); break; }

                case PROPERTY_ID_COMMAND:                   { rtl_uString_newFromAscii(&pStr,getPROPERTY_COMMAND()              ); break; }
                case PROPERTY_ID_CHECKOPTION:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_CHECKOPTION()          ); break; }
                case PROPERTY_ID_PASSWORD:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_PASSWORD()             ); break; }
                case PROPERTY_ID_RELATEDCOLUMN:             { rtl_uString_newFromAscii(&pStr,getPROPERTY_RELATEDCOLUMN()        ); break;  }

                case PROPERTY_ID_FUNCTION:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_FUNCTION()             ); break; }
                case PROPERTY_ID_AGGREGATEFUNCTION:          { rtl_uString_newFromAscii(&pStr,getPROPERTY_AGGREGATEFUNCTION()   ); break; }
                case PROPERTY_ID_TABLENAME:                 { rtl_uString_newFromAscii(&pStr,getPROPERTY_TABLENAME()            ); break; }
                case PROPERTY_ID_REALNAME:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_REALNAME()             ); break; }
                case PROPERTY_ID_DBASEPRECISIONCHANGED:     { rtl_uString_newFromAscii(&pStr,getPROPERTY_DBASEPRECISIONCHANGED()); break; }
                case PROPERTY_ID_ISCURRENCY:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISCURRENCY()           ); break; }
                case PROPERTY_ID_ISBOOKMARKABLE:            { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISBOOKMARKABLE()       ); break; }
                case PROPERTY_ID_HY010:                     { rtl_uString_newFromAscii(&pStr,getSQLSTATE_SEQUENCE()             ); break; }
                case PROPERTY_ID_DELIMITER:                 { rtl_uString_newFromAscii(&pStr,getSTR_DELIMITER()                 ); break; }
                case PROPERTY_ID_FORMATKEY:                 { rtl_uString_newFromAscii(&pStr,getPROPERTY_FORMATKEY()            ); break; }
                case PROPERTY_ID_LOCALE:                    { rtl_uString_newFromAscii(&pStr,getPROPERTY_LOCALE()               ); break; }
                case PROPERTY_ID_AUTOINCREMENTCREATION:     { rtl_uString_newFromAscii(&pStr,getPROPERTY_AUTOINCREMENTCREATION()); break; }
                case PROPERTY_ID_PRIVILEGES:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_PRIVILEGES()           ); break; }
                case PROPERTY_ID_HAVINGCLAUSE:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_HAVINGCLAUSE()      ); break; }
                case PROPERTY_ID_ISSIGNED:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_ISSIGNED()          ); break; }
                case PROPERTY_ID_ISSEARCHABLE:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_ISSEARCHABLE()      ); break; }
                case PROPERTY_ID_LABEL:                     { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_LABEL()             ); break; }
                case PROPERTY_ID_APPLYFILTER:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_APPLYFILTER()       ); break; }
                case PROPERTY_ID_FILTER:                    { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_FILTER()            ); break; }
                case PROPERTY_ID_MASTERFIELDS:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_MASTERFIELDS()      ); break; }
                case PROPERTY_ID_DETAILFIELDS:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_DETAILFIELDS()      ); break; }
                case PROPERTY_ID_FIELDTYPE:                 { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_FIELDTYPE()         ); break; }
                case PROPERTY_ID_VALUE:                     { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_VALUE()             ); break; }
                case PROPERTY_ID_ACTIVE_CONNECTION:         { rtl_uString_newFromAscii(&pStr,getPROPERTY_ID_ACTIVE_CONNECTION() ); break; }
            }
            m_aPropertyMap[_nIndex] = pStr;
            return pStr;
        }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
