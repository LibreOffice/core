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

#include <osl/diagnose.h>
#include "mysqlc_propertyids.hxx"


namespace connectivity
{
namespace mysqlc
{
const sal_Char* getPROPERTY_QUERYTIMEOUT()          { return "QueryTimeOut"; }
const sal_Char* getPROPERTY_MAXFIELDSIZE()          { return "MaxFieldSize"; }
const sal_Char* getPROPERTY_MAXROWS()               { return "MaxRows"; }
const sal_Char* getPROPERTY_CURSORNAME()            { return "CursorName"; }
const sal_Char* getPROPERTY_RESULTSETCONCURRENCY()  { return "ResultSetConcurrency"; }
const sal_Char* getPROPERTY_RESULTSETTYPE()         { return "ResultSetType"; }
const sal_Char* getPROPERTY_FETCHDIRECTION()        { return "FetchDirection"; }
const sal_Char* getPROPERTY_FETCHSIZE()             { return "FetchSize"; }
const sal_Char* getPROPERTY_ESCAPEPROCESSING()      { return "EscapeProcessing"; }
const sal_Char* getPROPERTY_USEBOOKMARKS()          { return "UseBookmarks"; }

const sal_Char* getPROPERTY_NAME()                  { return "Name"; }
const sal_Char* getPROPERTY_TYPE()                  { return "Type"; }
const sal_Char* getPROPERTY_TYPENAME()              { return "TypeName"; }
const sal_Char* getPROPERTY_PRECISION()             { return "Precision"; }
const sal_Char* getPROPERTY_SCALE()                 { return "Scale"; }
const sal_Char* getPROPERTY_ISNULLABLE()            { return "IsNullable"; }
const sal_Char* getPROPERTY_ISAUTOINCREMENT()       { return "IsAutoIncrement"; }
const sal_Char* getPROPERTY_ISROWVERSION()          { return "IsRowVersion"; }
const sal_Char* getPROPERTY_DESCRIPTION()           { return "Description"; }
const sal_Char* getPROPERTY_DEFAULTVALUE()          { return "DefaultValue"; }

const sal_Char* getPROPERTY_REFERENCEDTABLE()       { return "ReferencedTable"; }
const sal_Char* getPROPERTY_UPDATERULE()            { return "UpdateRule"; }
const sal_Char* getPROPERTY_DELETERULE()            { return "DeleteRule"; }
const sal_Char* getPROPERTY_CATALOG()               { return "Catalog"; }
const sal_Char* getPROPERTY_ISUNIQUE()              { return "IsUnique"; }
const sal_Char* getPROPERTY_ISPRIMARYKEYINDEX()     { return "IsPrimaryKeyIndex"; }
const sal_Char* getPROPERTY_ISCLUSTERED()           { return "IsClustered"; }
const sal_Char* getPROPERTY_ISASCENDING()           { return "IsAscending"; }
const sal_Char* getPROPERTY_SCHEMANAME()            { return "SchemaName"; }
const sal_Char* getPROPERTY_CATALOGNAME()           { return "CatalogName"; }
const sal_Char* getPROPERTY_COMMAND()               { return "Command"; }
const sal_Char* getPROPERTY_CHECKOPTION()           { return "CheckOption"; }
const sal_Char* getPROPERTY_PASSWORD()              { return "Password"; }
const sal_Char* getPROPERTY_RELATEDCOLUMN()         { return "RelatedColumn"; }

const sal_Char* getSTAT_INVALID_INDEX()             { return "Invalid descriptor index"; }

const sal_Char* getPROPERTY_FUNCTION()              { return "Function"; }
const sal_Char* getPROPERTY_TABLENAME()             { return "TableName"; }
const sal_Char* getPROPERTY_REALNAME()              { return "RealName"; }
const sal_Char* getPROPERTY_DBASEPRECISIONCHANGED() { return "DbasePrecisionChanged"; }
const sal_Char* getPROPERTY_ISCURRENCY()            { return "IsCurrency"; }
const sal_Char* getPROPERTY_ISBOOKMARKABLE()        { return "IsBookmarkable"; }

const sal_Char* getPROPERTY_FORMATKEY()             { return "FormatKey"; }
const sal_Char* getPROPERTY_LOCALE()                { return "Locale"; }

const sal_Char* getPROPERTY_AUTOINCREMENTCREATION() { return "AutoIncrementCreation"; }
const sal_Char* getPROPERTY_PRIVILEGES()            { return "Privileges"; }
    //============================================================
    //= error messages
    //============================================================
const sal_Char* getERRORMSG_SEQUENCE()              { return "Function sequence error"; }
const sal_Char* getSQLSTATE_SEQUENCE()              { return "HY010"; }
const sal_Char* getSQLSTATE_GENERAL()               { return "HY0000"; }
const sal_Char* getSTR_DELIMITER()                  { return "/"; }



/* {{{ OPropertyMap::~OPropertyMap() -I- */
OPropertyMap::~OPropertyMap()
{
    ::std::map<sal_Int32 , rtl_uString*>::iterator aIter = m_aPropertyMap.begin();
    for(; aIter != m_aPropertyMap.end(); ++aIter) {
        if (aIter->second) {
            rtl_uString_release(aIter->second);
        }
    }
}
/* }}} */


/* {{{ OPropertyMap::getNameByIndex() -I- */
OUString OPropertyMap::getNameByIndex(sal_Int32 idx) const
{
    OUString sRet;
    ::std::map<sal_Int32 , rtl_uString*>::const_iterator aIter = m_aPropertyMap.find(idx);
    if (aIter == m_aPropertyMap.end()) {
        sRet = const_cast<OPropertyMap*>(this)->fillValue(idx);
    } else {
        sRet = aIter->second;
    }
    return sRet;
}
/* }}} */

typedef const sal_Char * (*property_callback)();

static const property_callback property_callbacks[PROPERTY_ID_LAST] =
{
    NULL, /* PROPERTY_ID_FIRST */
    getPROPERTY_QUERYTIMEOUT,
    getPROPERTY_MAXFIELDSIZE,
    getPROPERTY_MAXROWS,
    getPROPERTY_CURSORNAME,
    getPROPERTY_RESULTSETCONCURRENCY,
    getPROPERTY_RESULTSETTYPE,
    getPROPERTY_FETCHDIRECTION,
    getPROPERTY_FETCHSIZE,
    getPROPERTY_ESCAPEPROCESSING,
    getPROPERTY_USEBOOKMARKS,
    // Column
    getPROPERTY_NAME,
    getPROPERTY_TYPE,
    getPROPERTY_TYPENAME,
    getPROPERTY_PRECISION,
    getPROPERTY_SCALE,
    getPROPERTY_ISNULLABLE,
    getPROPERTY_ISAUTOINCREMENT,
    getPROPERTY_ISROWVERSION,
    getPROPERTY_DESCRIPTION,
    getPROPERTY_DEFAULTVALUE,

    getPROPERTY_REFERENCEDTABLE,
    getPROPERTY_UPDATERULE,
    getPROPERTY_DELETERULE,
    getPROPERTY_CATALOG,
    getPROPERTY_ISUNIQUE,
    getPROPERTY_ISPRIMARYKEYINDEX,
    getPROPERTY_ISCLUSTERED,
    getPROPERTY_ISASCENDING,
    getPROPERTY_SCHEMANAME,
    getPROPERTY_CATALOGNAME,

    getPROPERTY_COMMAND,
    getPROPERTY_CHECKOPTION,
    getPROPERTY_PASSWORD,
    getPROPERTY_RELATEDCOLUMN,

    getPROPERTY_FUNCTION,
    getPROPERTY_TABLENAME,
    getPROPERTY_REALNAME,
    getPROPERTY_DBASEPRECISIONCHANGED,
    getPROPERTY_ISCURRENCY,
    getPROPERTY_ISBOOKMARKABLE,
    getSTAT_INVALID_INDEX,
    getERRORMSG_SEQUENCE,
    getSQLSTATE_SEQUENCE,
    getSQLSTATE_GENERAL,
    getSTR_DELIMITER,
    getPROPERTY_FORMATKEY,
    getPROPERTY_LOCALE,
    getPROPERTY_AUTOINCREMENTCREATION,
    getPROPERTY_PRIVILEGES,
};


/* {{{ OPropertyMap::fillValue() -I- */
OUString OPropertyMap::fillValue(sal_Int32 idx)
{
    rtl_uString* pStr = NULL;
    rtl_uString_newFromAscii(&pStr, property_callbacks[idx]());
    m_aPropertyMap[idx] = pStr;
    return pStr;
}
/* }}} */

} /* mysqlc */
} /* connectivity */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
