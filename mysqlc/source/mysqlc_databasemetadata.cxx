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
#include "mysqlc_databasemetadata.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/Deferrability.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <com/sun/star/sdbc/BestRowScope.hpp>
#include <com/sun/star/sdbc/ColumnType.hpp>
#include <com/sun/star/lang/XInitialization.hpp>


#include "mysqlc_general.hxx"
#include "mysqlc_statement.hxx"
#include "mysqlc_driver.hxx"
#include "mysqlc_preparedstatement.hxx"

#include <stdio.h>

using namespace connectivity::mysqlc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using ::rtl::OUString;
using mysqlc_sdbc_driver::getStringFromAny;

#include <cppconn/connection.h>
#include <cppconn/resultset.h>
#include <cppconn/metadata.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <sal/macros.h>

static std::string wild("%");

using ::rtl::OUStringToOString;

// -----------------------------------------------------------------------------
void lcl_setRows_throw(const Reference< XResultSet >& _xResultSet,sal_Int32 _nType,const std::vector< std::vector< Any > >& _rRows)
{
    Reference< XInitialization> xIni(_xResultSet,UNO_QUERY);
    Sequence< Any > aArgs(2);
    aArgs[0] <<= _nType;

    Sequence< Sequence< Any > > aRows(_rRows.size());

    std::vector< std::vector< Any > >::const_iterator aIter = _rRows.begin();
    Sequence< Any > * pRowsIter = aRows.getArray();
    Sequence< Any > * pRowsEnd  = pRowsIter + aRows.getLength();
    for (; pRowsIter != pRowsEnd;++pRowsIter,++aIter) {
        if (!aIter->empty()) {
            Sequence<Any> aSeq(&(*aIter->begin()),aIter->size());
            (*pRowsIter) = aSeq;
        }
    }
    aArgs[1] <<= aRows;
    xIni->initialize(aArgs);
}


/* {{{ ODatabaseMetaData::ODatabaseMetaData() -I- */
ODatabaseMetaData::ODatabaseMetaData(OConnection& _rCon)
    :m_rConnection(_rCon)
    ,m_bUseCatalog(sal_True)
    ,meta(_rCon.getConnectionSettings().cppConnection->getMetaData())
    ,identifier_quote_string_set(false)
{
    OSL_TRACE("ODatabaseMetaData::ODatabaseMetaData");
    if (!m_rConnection.isCatalogUsed())
    {
        osl_atomic_increment(&m_refCount);
        m_bUseCatalog = !(usesLocalFiles() || usesLocalFilePerTable());
        osl_atomic_decrement(&m_refCount);
    }
}
/* }}} */


/* {{{ ODatabaseMetaData::~ODatabaseMetaData() -I- */
ODatabaseMetaData::~ODatabaseMetaData()
{
    OSL_TRACE("ODatabaseMetaData::~ODatabaseMetaData");
}
/* }}} */


/* {{{ ODatabaseMetaData::impl_getStringMetaData() -I- */
OUString ODatabaseMetaData::impl_getStringMetaData(const sal_Char* _methodName, const std::string& (sql::DatabaseMetaData::*_Method)() )
{
    OSL_TRACE( "mysqlc::ODatabaseMetaData::%s", _methodName);
    OUString stringMetaData;
    try {
        stringMetaData = mysqlc_sdbc_driver::convert((meta->*_Method)(), m_rConnection.getConnectionEncoding());
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return stringMetaData;
}
/* }}} */


/* {{{ ODatabaseMetaData::impl_getStringMetaData() -I- */
OUString ODatabaseMetaData::impl_getStringMetaData(const sal_Char* _methodName, std::string (sql::DatabaseMetaData::*_Method)() )
{
    OSL_TRACE( "mysqlc::ODatabaseMetaData::%s", _methodName);
    OUString stringMetaData;
    try {
        stringMetaData = mysqlc_sdbc_driver::convert((meta->*_Method)(), m_rConnection.getConnectionEncoding());
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return stringMetaData;
}
/* }}} */


/* {{{ ODatabaseMetaData::impl_getStringMetaData() -I- */
OUString ODatabaseMetaData::impl_getStringMetaData(const sal_Char* _methodName, const sql::SQLString& (sql::DatabaseMetaData::*_Method)() )
{
    OSL_TRACE( "mysqlc::ODatabaseMetaData::%s", _methodName);
    OUString stringMetaData;
    try {
        stringMetaData = mysqlc_sdbc_driver::convert((meta->*_Method)(), m_rConnection.getConnectionEncoding());
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return stringMetaData;
}
/* }}} */


/* {{{ ODatabaseMetaData::impl_getStringMetaData() -I- */
OUString ODatabaseMetaData::impl_getStringMetaData(const sal_Char* _methodName, sql::SQLString (sql::DatabaseMetaData::*_Method)() )
{
    OSL_TRACE( "mysqlc::ODatabaseMetaData::%s", _methodName);
    OUString stringMetaData;
    try {
        stringMetaData = mysqlc_sdbc_driver::convert((meta->*_Method)(), m_rConnection.getConnectionEncoding());
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return stringMetaData;
}
/* }}} */


/* {{{ ODatabaseMetaData::impl_getInt32MetaData() -I- */
sal_Int32 ODatabaseMetaData::impl_getInt32MetaData(const sal_Char* _methodName, unsigned int (sql::DatabaseMetaData::*_Method)() )
{
    OSL_TRACE( "mysqlc::ODatabaseMetaData::%s", _methodName);
    sal_Int32 int32MetaData(0);
    try {
        int32MetaData = (meta->*_Method)();
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return int32MetaData;
}
/* }}} */


/* {{{ ODatabaseMetaData::impl_getBoolMetaData() -I- */
sal_Bool ODatabaseMetaData::impl_getBoolMetaData(const sal_Char* _methodName, bool (sql::DatabaseMetaData::*_Method)() )
{
    OSL_TRACE( "mysqlc::ODatabaseMetaData::%s", _methodName);
    sal_Bool boolMetaData(0);
    try {
        boolMetaData = (meta->*_Method)() ? sal_True : sal_False;
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return boolMetaData;
}
/* }}} */


/* {{{ ODatabaseMetaData::impl_getBoolMetaData() -I- */
sal_Bool ODatabaseMetaData::impl_getBoolMetaData(const sal_Char* _methodName, bool (sql::DatabaseMetaData::*_Method)(int), sal_Int32 _arg )
{
    OSL_TRACE( "mysqlc::ODatabaseMetaData::%s", _methodName);
    sal_Bool boolMetaData(0);
    try {
        boolMetaData = (meta->*_Method)( _arg ) ? sal_True : sal_False;
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return boolMetaData;
}
/* }}} */


/* {{{ ODatabaseMetaData::impl_getRSTypeMetaData() -I- */
sal_Bool ODatabaseMetaData::impl_getRSTypeMetaData(const sal_Char* _methodName, bool (sql::DatabaseMetaData::*_Method)(int), sal_Int32 _resultSetType )
{
    int resultSetType(sql::ResultSet::TYPE_FORWARD_ONLY);
    switch ( _resultSetType ) {
        case ResultSetType::SCROLL_INSENSITIVE: resultSetType = sql::ResultSet::TYPE_SCROLL_INSENSITIVE;    break;
        case ResultSetType::SCROLL_SENSITIVE:   resultSetType = sql::ResultSet::TYPE_SCROLL_SENSITIVE;      break;
    }

    return impl_getBoolMetaData(_methodName, _Method, resultSetType);
}
/* }}} */


/* {{{ ODatabaseMetaData::getCatalogSeparator() -I- */
OUString SAL_CALL ODatabaseMetaData::getCatalogSeparator()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getCatalogSeparator", &sql::DatabaseMetaData::getCatalogSeparator);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxBinaryLiteralLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxBinaryLiteralLength", &sql::DatabaseMetaData::getMaxBinaryLiteralLength);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxRowSize() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxRowSize", &sql::DatabaseMetaData::getMaxRowSize);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxCatalogNameLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxCatalogNameLength", &sql::DatabaseMetaData::getMaxCatalogNameLength);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxCharLiteralLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxCharLiteralLength", &sql::DatabaseMetaData::getMaxCharLiteralLength);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxColumnNameLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxColumnNameLength", &sql::DatabaseMetaData::getMaxColumnNameLength);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxColumnsInIndex() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxColumnsInIndex", &sql::DatabaseMetaData::getMaxColumnsInIndex);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxCursorNameLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxCursorNameLength", &sql::DatabaseMetaData::getMaxCursorNameLength);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxConnections() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxConnections", &sql::DatabaseMetaData::getMaxConnections);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxColumnsInTable() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxColumnsInTable", &sql::DatabaseMetaData::getMaxColumnsInTable);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxStatementLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxStatementLength", &sql::DatabaseMetaData::getMaxStatementLength);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxTableNameLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxTableNameLength", &sql::DatabaseMetaData::getMaxTableNameLength);
}
/* }}} */

/* {{{ ODatabaseMetaData::getMaxTablesInSelect() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTablesInSelect()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxTablesInSelect", &sql::DatabaseMetaData::getMaxTablesInSelect);
}
/* }}} */


/* {{{ ODatabaseMetaData::doesMaxRowSizeIncludeBlobs() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("doesMaxRowSizeIncludeBlobs", &sql::DatabaseMetaData::doesMaxRowSizeIncludeBlobs);
}
/* }}} */


/* {{{ ODatabaseMetaData::storesLowerCaseQuotedIdentifiers() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("storesLowerCaseQuotedIdentifiers", &sql::DatabaseMetaData::storesLowerCaseQuotedIdentifiers);
}
/* }}} */


/* {{{ ODatabaseMetaData::storesLowerCaseIdentifiers() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("storesLowerCaseIdentifiers", &sql::DatabaseMetaData::storesLowerCaseIdentifiers);
}
/* }}} */


/* {{{ ODatabaseMetaData::storesMixedCaseQuotedIdentifiers() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseQuotedIdentifiers()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("storesMixedCaseQuotedIdentifiers", &sql::DatabaseMetaData::storesMixedCaseQuotedIdentifiers);
}
/* }}} */


/* {{{ ODatabaseMetaData::storesMixedCaseIdentifiers() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("storesMixedCaseIdentifiers", &sql::DatabaseMetaData::storesMixedCaseIdentifiers);
}
/* }}} */


/* {{{ ODatabaseMetaData::storesUpperCaseQuotedIdentifiers() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("storesUpperCaseQuotedIdentifiers", &sql::DatabaseMetaData::storesUpperCaseQuotedIdentifiers);
}
/* }}} */


/* {{{ ODatabaseMetaData::storesUpperCaseIdentifiers() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("storesUpperCaseIdentifiers", &sql::DatabaseMetaData::storesUpperCaseIdentifiers);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsAlterTableWithAddColumn() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithAddColumn()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsAlterTableWithAddColumn", &sql::DatabaseMetaData::supportsAlterTableWithAddColumn);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsAlterTableWithDropColumn() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithDropColumn()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsAlterTableWithDropColumn", &sql::DatabaseMetaData::supportsAlterTableWithDropColumn);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxIndexLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxIndexLength", &sql::DatabaseMetaData::getMaxIndexLength);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsNonNullableColumns() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsNonNullableColumns", &sql::DatabaseMetaData::supportsNonNullableColumns);
}
/* }}} */


/* {{{ ODatabaseMetaData::getCatalogTerm() -I- */
OUString SAL_CALL ODatabaseMetaData::getCatalogTerm()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getCatalogTerm", &sql::DatabaseMetaData::getCatalogTerm);
}
/* }}} */


/* {{{ ODatabaseMetaData::getIdentifierQuoteString() -I- */
OUString SAL_CALL ODatabaseMetaData::getIdentifierQuoteString()
    throw(SQLException, RuntimeException)
{
    if (identifier_quote_string_set == false) {
        identifier_quote_string = impl_getStringMetaData("getIdentifierQuoteString", &sql::DatabaseMetaData::getIdentifierQuoteString);
        identifier_quote_string_set = true;
    }
    return identifier_quote_string;
}
/* }}} */


/* {{{ ODatabaseMetaData::getExtraNameCharacters() -I- */
OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getExtraNameCharacters", &sql::DatabaseMetaData::getExtraNameCharacters);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsDifferentTableCorrelationNames() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsDifferentTableCorrelationNames", &sql::DatabaseMetaData::supportsDifferentTableCorrelationNames);
}
/* }}} */


/* {{{ ODatabaseMetaData::isCatalogAtStart() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::isCatalogAtStart()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("isCatalogAtStart", &sql::DatabaseMetaData::isCatalogAtStart);
}
/* }}} */


/* {{{ ODatabaseMetaData::dataDefinitionIgnoredInTransactions() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("dataDefinitionIgnoredInTransactions", &sql::DatabaseMetaData::dataDefinitionIgnoredInTransactions);
}
/* }}} */


/* {{{ ODatabaseMetaData::dataDefinitionCausesTransactionCommit() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("dataDefinitionCausesTransactionCommit", &sql::DatabaseMetaData::dataDefinitionCausesTransactionCommit);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsDataManipulationTransactionsOnly() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsDataManipulationTransactionsOnly", &sql::DatabaseMetaData::supportsDataManipulationTransactionsOnly);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsDataDefinitionAndDataManipulationTransactions", &sql::DatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsPositionedDelete() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsPositionedDelete", &sql::DatabaseMetaData::supportsPositionedDelete);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsPositionedUpdate() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsPositionedUpdate", &sql::DatabaseMetaData::supportsPositionedUpdate);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsOpenStatementsAcrossRollback() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsOpenStatementsAcrossRollback", &sql::DatabaseMetaData::supportsOpenStatementsAcrossRollback);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsOpenStatementsAcrossCommit() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsOpenStatementsAcrossCommit", &sql::DatabaseMetaData::supportsOpenStatementsAcrossCommit);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsOpenCursorsAcrossCommit() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsOpenCursorsAcrossCommit", &sql::DatabaseMetaData::supportsOpenCursorsAcrossCommit);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsOpenCursorsAcrossRollback() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsOpenCursorsAcrossRollback", &sql::DatabaseMetaData::supportsOpenCursorsAcrossRollback);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsTransactionIsolationLevel() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel(sal_Int32 level)
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsTransactionIsolationLevel", &sql::DatabaseMetaData::supportsTransactionIsolationLevel, level);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsSchemasInDataManipulation() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInDataManipulation()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsSchemasInDataManipulation", &sql::DatabaseMetaData::supportsSchemasInDataManipulation);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsANSI92FullSQL() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsANSI92FullSQL", &sql::DatabaseMetaData::supportsANSI92FullSQL);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsANSI92EntryLevelSQL() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsANSI92EntryLevelSQL", &sql::DatabaseMetaData::supportsANSI92EntryLevelSQL);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsIntegrityEnhancementFacility() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsIntegrityEnhancementFacility", &sql::DatabaseMetaData::supportsIntegrityEnhancementFacility);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsSchemasInIndexDefinitions() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsSchemasInIndexDefinitions", &sql::DatabaseMetaData::supportsSchemasInIndexDefinitions);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsSchemasInTableDefinitions() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInTableDefinitions()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsSchemasInTableDefinitions", &sql::DatabaseMetaData::supportsSchemasInTableDefinitions);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsCatalogsInTableDefinitions() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInTableDefinitions()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsCatalogsInTableDefinitions", &sql::DatabaseMetaData::supportsCatalogsInTableDefinitions);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsCatalogsInIndexDefinitions() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsCatalogsInIndexDefinitions", &sql::DatabaseMetaData::supportsCatalogsInIndexDefinitions);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsCatalogsInDataManipulation() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInDataManipulation()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsCatalogsInDataManipulation", &sql::DatabaseMetaData::supportsCatalogsInDataManipulation);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsOuterJoins() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsOuterJoins", &sql::DatabaseMetaData::supportsOuterJoins);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxStatements() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatements()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxStatements", &sql::DatabaseMetaData::getMaxStatements);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxProcedureNameLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxProcedureNameLength", &sql::DatabaseMetaData::getMaxProcedureNameLength);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxSchemaNameLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxSchemaNameLength", &sql::DatabaseMetaData::getMaxSchemaNameLength);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsTransactions() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsTransactions", &sql::DatabaseMetaData::supportsTransactions);
}
/* }}} */


/* {{{ ODatabaseMetaData::allProceduresAreCallable() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("allProceduresAreCallable", &sql::DatabaseMetaData::allProceduresAreCallable);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsStoredProcedures() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsStoredProcedures", &sql::DatabaseMetaData::supportsStoredProcedures);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsSelectForUpdate() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsSelectForUpdate", &sql::DatabaseMetaData::supportsSelectForUpdate);
}
/* }}} */


/* {{{ ODatabaseMetaData::allTablesAreSelectable() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("allTablesAreSelectable", &sql::DatabaseMetaData::allTablesAreSelectable);
}
/* }}} */


/* {{{ ODatabaseMetaData::isReadOnly() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("isReadOnly", &sql::DatabaseMetaData::isReadOnly);
}
/* }}} */


/* {{{ ODatabaseMetaData::usesLocalFiles() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("usesLocalFiles", &sql::DatabaseMetaData::usesLocalFiles);
}
/* }}} */


/* {{{ ODatabaseMetaData::usesLocalFilePerTable() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("usesLocalFilePerTable", &sql::DatabaseMetaData::usesLocalFilePerTable);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsTypeConversion() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsTypeConversion", &sql::DatabaseMetaData::supportsTypeConversion);
}
/* }}} */


/* {{{ ODatabaseMetaData::nullPlusNonNullIsNull() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("nullPlusNonNullIsNull", &sql::DatabaseMetaData::nullPlusNonNullIsNull);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsColumnAliasing() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsColumnAliasing", &sql::DatabaseMetaData::supportsColumnAliasing);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsTableCorrelationNames() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsTableCorrelationNames", &sql::DatabaseMetaData::supportsTableCorrelationNames);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsConvert() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert(sal_Int32 /* fromType */, sal_Int32 /* toType */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::supportsConvert");
    try {
        /* ToDo -> use supportsConvert( fromType, toType) */
        return meta->supportsConvert()? sal_True:sal_False;
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::supportsConvert", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::supportsConvert", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return sal_False;
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsExpressionsInOrderBy() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsExpressionsInOrderBy", &sql::DatabaseMetaData::supportsExpressionsInOrderBy);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsGroupBy() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsGroupBy", &sql::DatabaseMetaData::supportsGroupBy);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsGroupByBeyondSelect() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsGroupByBeyondSelect", &sql::DatabaseMetaData::supportsGroupByBeyondSelect);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsGroupByUnrelated() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsGroupByUnrelated", &sql::DatabaseMetaData::supportsGroupByUnrelated);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsMultipleTransactions() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsMultipleTransactions", &sql::DatabaseMetaData::supportsMultipleTransactions);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsMultipleResultSets() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsMultipleResultSets", &sql::DatabaseMetaData::supportsMultipleResultSets);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsLikeEscapeClause() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsLikeEscapeClause", &sql::DatabaseMetaData::supportsLikeEscapeClause);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsOrderByUnrelated() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsOrderByUnrelated", &sql::DatabaseMetaData::supportsOrderByUnrelated);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsUnion() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsUnion", &sql::DatabaseMetaData::supportsUnion);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsUnionAll() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsUnionAll", &sql::DatabaseMetaData::supportsUnionAll);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsMixedCaseIdentifiers() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsMixedCaseIdentifiers", &sql::DatabaseMetaData::supportsMixedCaseIdentifiers);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsMixedCaseQuotedIdentifiers() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseQuotedIdentifiers()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsMixedCaseQuotedIdentifiers", &sql::DatabaseMetaData::supportsMixedCaseQuotedIdentifiers);
}
/* }}} */


/* {{{ ODatabaseMetaData::nullsAreSortedAtEnd() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("nullsAreSortedAtEnd", &sql::DatabaseMetaData::nullsAreSortedAtEnd);
}
/* }}} */


/* {{{ ODatabaseMetaData::nullsAreSortedAtStart() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("nullsAreSortedAtStart", &sql::DatabaseMetaData::nullsAreSortedAtStart);
}
/* }}} */


/* {{{ ODatabaseMetaData::nullsAreSortedHigh() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("nullsAreSortedHigh", &sql::DatabaseMetaData::nullsAreSortedHigh);
}
/* }}} */


/* {{{ ODatabaseMetaData::nullsAreSortedLow() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("nullsAreSortedLow", &sql::DatabaseMetaData::nullsAreSortedLow);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsSchemasInProcedureCalls() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsSchemasInProcedureCalls", &sql::DatabaseMetaData::supportsSchemasInProcedureCalls);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsSchemasInPrivilegeDefinitions", &sql::DatabaseMetaData::supportsSchemasInPrivilegeDefinitions);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsCatalogsInProcedureCalls() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsCatalogsInProcedureCalls", &sql::DatabaseMetaData::supportsCatalogsInProcedureCalls);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsCatalogsInPrivilegeDefinitions", &sql::DatabaseMetaData::supportsCatalogsInPrivilegeDefinitions);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsCorrelatedSubqueries() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsCorrelatedSubqueries", &sql::DatabaseMetaData::supportsCorrelatedSubqueries);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsSubqueriesInComparisons() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsSubqueriesInComparisons", &sql::DatabaseMetaData::supportsSubqueriesInComparisons);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsSubqueriesInExists() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsSubqueriesInExists", &sql::DatabaseMetaData::supportsSubqueriesInExists);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsSubqueriesInIns() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsSubqueriesInIns", &sql::DatabaseMetaData::supportsSubqueriesInIns);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsSubqueriesInQuantifieds() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsSubqueriesInQuantifieds", &sql::DatabaseMetaData::supportsSubqueriesInQuantifieds);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsANSI92IntermediateSQL() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsANSI92IntermediateSQL", &sql::DatabaseMetaData::supportsANSI92IntermediateSQL);
}
/* }}} */


/* {{{ ODatabaseMetaData::getURL() -I- */
OUString SAL_CALL ODatabaseMetaData::getURL()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getURL");
    return m_rConnection.getConnectionSettings().connectionURL;
}
/* }}} */


/* {{{ ODatabaseMetaData::getUserName() -I- */
OUString SAL_CALL ODatabaseMetaData::getUserName()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getUserName", &sql::DatabaseMetaData::getUserName);
}
/* }}} */


/* {{{ ODatabaseMetaData::getDriverName() -I- */
OUString SAL_CALL ODatabaseMetaData::getDriverName()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getDriverName");
    OUString aValue( RTL_CONSTASCII_USTRINGPARAM( "MySQL Connector/OO.org" ) );
    return aValue;
}
/* }}} */


/* {{{ ODatabaseMetaData::getDriverVersion() -I- */
OUString SAL_CALL ODatabaseMetaData::getDriverVersion()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getDriverVersion");
    static const OUString sVersion( RTL_CONSTASCII_USTRINGPARAM( "0.9.2" ) );
    return sVersion;
}
/* }}} */


/* {{{ ODatabaseMetaData::getDatabaseProductVersion() -I- */
OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getDatabaseProductVersion", &sql::DatabaseMetaData::getDatabaseProductVersion);
}
/* }}} */


/* {{{ ODatabaseMetaData::getDatabaseProductName() -I- */
OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getDatabaseProductName", &sql::DatabaseMetaData::getDatabaseProductName);
}
/* }}} */


/* {{{ ODatabaseMetaData::getProcedureTerm() -I- */
OUString SAL_CALL ODatabaseMetaData::getProcedureTerm()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getProcedureTerm", &sql::DatabaseMetaData::getProcedureTerm);
}
/* }}} */


/* {{{ ODatabaseMetaData::getSchemaTerm() -I- */
OUString SAL_CALL ODatabaseMetaData::getSchemaTerm()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getSchemaTerm", &sql::DatabaseMetaData::getSchemaTerm);
}
/* }}} */


/* {{{ ODatabaseMetaData::getDriverMajorVersion() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion()
    throw(RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getDriverMajorVersion");
    return MYSQLC_VERSION_MAJOR;
}
/* }}} */


/* {{{ ODatabaseMetaData::getDefaultTransactionIsolation() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getDefaultTransactionIsolation");
    try {
        switch (meta->getDefaultTransactionIsolation()) {
            case sql::TRANSACTION_SERIALIZABLE:     return TransactionIsolation::SERIALIZABLE;
            case sql::TRANSACTION_REPEATABLE_READ:  return TransactionIsolation::REPEATABLE_READ;
            case sql::TRANSACTION_READ_COMMITTED:   return TransactionIsolation::READ_COMMITTED;
            case sql::TRANSACTION_READ_UNCOMMITTED: return TransactionIsolation::READ_UNCOMMITTED;
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getDriverMajorVersion", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getDriverMajorVersion", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return TransactionIsolation::NONE;
}
/* }}} */


/* {{{ ODatabaseMetaData::getDriverMinorVersion() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion()
    throw(RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getDriverMinorVersion");
    return MYSQLC_VERSION_MINOR;
}
/* }}} */


/* {{{ ODatabaseMetaData::getSQLKeywords() -I- */
OUString SAL_CALL ODatabaseMetaData::getSQLKeywords()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getSQLKeywords", &sql::DatabaseMetaData::getSQLKeywords);
}
/* }}} */


/* {{{ ODatabaseMetaData::getSearchStringEscape() -I- */
OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getSearchStringEscape", &sql::DatabaseMetaData::getSearchStringEscape);
}
/* }}} */


/* {{{ ODatabaseMetaData::getStringFunctions() -I- */
OUString SAL_CALL ODatabaseMetaData::getStringFunctions()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getStringFunctions", &sql::DatabaseMetaData::getStringFunctions);
}
/* }}} */


/* {{{ ODatabaseMetaData::getTimeDateFunctions() -I- */
OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getTimeDateFunctions", &sql::DatabaseMetaData::getTimeDateFunctions);
}
/* }}} */


/* {{{ ODatabaseMetaData::getSystemFunctions() -I- */
OUString SAL_CALL ODatabaseMetaData::getSystemFunctions()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getSystemFunctions", &sql::DatabaseMetaData::getSystemFunctions);
}
/* }}} */


/* {{{ ODatabaseMetaData::getNumericFunctions() -I- */
OUString SAL_CALL ODatabaseMetaData::getNumericFunctions()
    throw(SQLException, RuntimeException)
{
    return impl_getStringMetaData("getNumericFunctions", &sql::DatabaseMetaData::getNumericFunctions);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsExtendedSQLGrammar() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsExtendedSQLGrammar", &sql::DatabaseMetaData::supportsExtendedSQLGrammar);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsCoreSQLGrammar() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsCoreSQLGrammar", &sql::DatabaseMetaData::supportsCoreSQLGrammar);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsMinimumSQLGrammar() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsMinimumSQLGrammar", &sql::DatabaseMetaData::supportsMinimumSQLGrammar);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsFullOuterJoins() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsFullOuterJoins", &sql::DatabaseMetaData::supportsFullOuterJoins);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsLimitedOuterJoins() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsLimitedOuterJoins", &sql::DatabaseMetaData::supportsLimitedOuterJoins);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxColumnsInGroupBy() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxColumnsInGroupBy", &sql::DatabaseMetaData::getMaxColumnsInGroupBy);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxColumnsInOrderBy() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxColumnsInOrderBy", &sql::DatabaseMetaData::getMaxColumnsInOrderBy);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxColumnsInSelect() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxColumnsInSelect", &sql::DatabaseMetaData::getMaxColumnsInSelect);
}
/* }}} */


/* {{{ ODatabaseMetaData::getMaxUserNameLength() -I- */
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength()
    throw(SQLException, RuntimeException)
{
    return impl_getInt32MetaData("getMaxUserNameLength", &sql::DatabaseMetaData::getMaxUserNameLength);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsResultSetType() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    return impl_getRSTypeMetaData("supportsResultSetType", &sql::DatabaseMetaData::supportsResultSetType, setType);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsResultSetConcurrency() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency(sal_Int32 setType, sal_Int32 concurrency)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::supportsResultSetConcurrency");
    /* TODO: Check this out */
    try {
        return meta->supportsResultSetConcurrency(setType, concurrency==com::sun::star::sdbc::TransactionIsolation::READ_COMMITTED?
                                                    sql::TRANSACTION_READ_COMMITTED:
                                                    (concurrency == com::sun::star::sdbc::TransactionIsolation::SERIALIZABLE?
                                                        sql::TRANSACTION_SERIALIZABLE:sql::TRANSACTION_SERIALIZABLE))? sal_True:sal_False;
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::supportsResultSetConcurrency", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::supportsResultSetConcurrency", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return sal_False;
}
/* }}} */


/* {{{ ODatabaseMetaData::ownUpdatesAreVisible() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    return impl_getRSTypeMetaData("ownUpdatesAreVisible", &sql::DatabaseMetaData::ownUpdatesAreVisible, setType);
}
/* }}} */


/* {{{ ODatabaseMetaData::ownDeletesAreVisible() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    return impl_getRSTypeMetaData("ownDeletesAreVisible", &sql::DatabaseMetaData::ownDeletesAreVisible, setType);
}
/* }}} */


/* {{{ ODatabaseMetaData::ownInsertsAreVisible() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    return impl_getRSTypeMetaData("ownInsertsAreVisible", &sql::DatabaseMetaData::ownInsertsAreVisible, setType);
}
/* }}} */


/* {{{ ODatabaseMetaData::othersUpdatesAreVisible() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    return impl_getRSTypeMetaData("othersUpdatesAreVisible", &sql::DatabaseMetaData::othersUpdatesAreVisible, setType);
}
/* }}} */


/* {{{ ODatabaseMetaData::othersDeletesAreVisible() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    return impl_getRSTypeMetaData("othersDeletesAreVisible", &sql::DatabaseMetaData::othersDeletesAreVisible, setType);
}
/* }}} */


/* {{{ ODatabaseMetaData::othersInsertsAreVisible() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    return impl_getRSTypeMetaData("othersInsertsAreVisible", &sql::DatabaseMetaData::othersInsertsAreVisible, setType);
}
/* }}} */


/* {{{ ODatabaseMetaData::updatesAreDetected() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    return impl_getRSTypeMetaData("updatesAreDetected", &sql::DatabaseMetaData::updatesAreDetected, setType);
}
/* }}} */


/* {{{ ODatabaseMetaData::deletesAreDetected() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    return impl_getRSTypeMetaData("deletesAreDetected", &sql::DatabaseMetaData::deletesAreDetected, setType);
}
/* }}} */


/* {{{ ODatabaseMetaData::insertsAreDetected() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    return impl_getRSTypeMetaData("insertsAreDetected", &sql::DatabaseMetaData::insertsAreDetected, setType);
}
/* }}} */


/* {{{ ODatabaseMetaData::supportsBatchUpdates() -I- */
sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates()
    throw(SQLException, RuntimeException)
{
    return impl_getBoolMetaData("supportsBatchUpdates", &sql::DatabaseMetaData::supportsBatchUpdates);
}
/* }}} */


/* {{{ ODatabaseMetaData::getConnection() -I- */
Reference< XConnection > SAL_CALL ODatabaseMetaData::getConnection()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getConnection");
    return (Reference< XConnection >)&m_rConnection;
}
/* }}} */


/*
  Here follow all methods which return(a resultset
  the first methods is an example implementation how to use this resultset
  of course you could implement it on your and you should do this because
  the general way is more memory expensive
*/

/* {{{ ODatabaseMetaData::getTableTypes() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getTableTypes");
    const char * table_types[] = {"TABLE", "VIEW"};
    sal_Int32 requiredVersion[] = {0, 50000};

    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();

    for (sal_uInt32 i = 0; i < 2; i++) {
        if (m_rConnection.getMysqlVersion() >= requiredVersion[i]) {
            std::vector< Any > aRow(1);
            aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(table_types[i], encoding)));
            rRows.push_back(aRow);
        }
    }
    lcl_setRows_throw(xResultSet, 5 ,rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getTypeInfo() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTypeInfo()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getTypeInfo");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);

    std::vector< std::vector< Any > > rRows;

    rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
    unsigned int i = 0;
    while (mysqlc_types[i].typeName) {
        std::vector< Any > aRow(1);

        aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(mysqlc_types[i].typeName, encoding)));
        aRow.push_back(makeAny(mysqlc_types[i].dataType));
        aRow.push_back(makeAny(mysqlc_types[i].precision));
        aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(mysqlc_types[i].literalPrefix, encoding)));
        aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(mysqlc_types[i].literalSuffix, encoding)));
        aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(mysqlc_types[i].createParams, encoding)));
        aRow.push_back(makeAny(mysqlc_types[i].nullable));
        aRow.push_back(makeAny(mysqlc_types[i].caseSensitive));
        aRow.push_back(makeAny(mysqlc_types[i].searchable));
        aRow.push_back(makeAny(mysqlc_types[i].isUnsigned));
        aRow.push_back(makeAny(mysqlc_types[i].fixedPrecScale));
        aRow.push_back(makeAny(mysqlc_types[i].autoIncrement));
        aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(mysqlc_types[i].localTypeName, encoding)));
        aRow.push_back(makeAny(mysqlc_types[i].minScale));
        aRow.push_back(makeAny(mysqlc_types[i].maxScale));
        aRow.push_back(makeAny(sal_Int32(0)));
        aRow.push_back(makeAny(sal_Int32(0)));
        aRow.push_back(makeAny(sal_Int32(10)));

        rRows.push_back(aRow);
        i++;
    }

    lcl_setRows_throw(xResultSet, 14, rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getCatalogs() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCatalogs()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getCatalogs");

    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::auto_ptr< sql::ResultSet> rset( meta->getCatalogs());
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            for (sal_uInt32 i = 1; i <= columns; i++) {
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
            }
            rRows.push_back(aRow);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getCatalogs", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getCatalogs", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet, 0, rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getSchemas() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getSchemas");

    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::auto_ptr< sql::ResultSet> rset( meta->getSchemas());
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            bool informationSchema = false;
            for (sal_uInt32 i = 1; i <= columns; i++) {
                sql::SQLString columnStringValue = rset->getString(i);
                if (i == 1) {   // TABLE_SCHEM
                    informationSchema = (0 == columnStringValue.compare("information_schema"));
                }
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(columnStringValue, encoding)));
            }
            if (!informationSchema ) {
                rRows.push_back(aRow);
            }
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getSchemas", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getSchemas", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet, 1, rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getColumnPrivileges() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
        const Any& catalog,
        const OUString& schema,
        const OUString& table,
        const OUString& columnNamePattern)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getColumnPrivileges");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr()),
                cNamePattern(OUStringToOString(columnNamePattern, m_rConnection.getConnectionEncoding()).getStr());
    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::auto_ptr< sql::ResultSet> rset( meta->getColumnPrivileges(cat, sch, tab, cNamePattern.compare("")? cNamePattern:wild));

        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            for (sal_uInt32 i = 1; i <= columns; i++) {
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
            }
            rRows.push_back(aRow);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getColumnPrivileges", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getColumnPrivileges", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet, 2, rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getColumns() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
        const Any& catalog,
        const OUString& schemaPattern,
        const OUString& tableNamePattern,
        const OUString& columnNamePattern)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getColumns");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    std::string cat(catalog.hasValue()? OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sPattern(OUStringToOString(schemaPattern, m_rConnection.getConnectionEncoding()).getStr()),
                tNamePattern(OUStringToOString(tableNamePattern, m_rConnection.getConnectionEncoding()).getStr()),
                cNamePattern(OUStringToOString(columnNamePattern, m_rConnection.getConnectionEncoding()).getStr());

    try {
        std::auto_ptr< sql::ResultSet> rset( meta->getColumns(cat,
                                                sPattern.compare("")? sPattern:wild,
                                                tNamePattern.compare("")? tNamePattern:wild,
                                                cNamePattern.compare("")? cNamePattern:wild));
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            for (sal_uInt32 i = 1; i <= columns; i++) {
                if (i == 5) { // ColumnType
                    sal_Int32 sdbc_type = mysqlc_sdbc_driver::mysqlToOOOType(atoi(rset->getString(i).c_str()));
                    aRow.push_back(makeAny(sdbc_type));
                } else {
                    aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
                }
            }
            rRows.push_back(aRow);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getColumns", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getColumns", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    lcl_setRows_throw(xResultSet, 3, rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getTables() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
        const Any& catalog,
        const OUString& schemaPattern,
        const OUString& tableNamePattern,
        const Sequence< OUString >& types )
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getTables");
    sal_Int32 nLength = types.getLength();

    Reference< XResultSet > xResultSet(getOwnConnection().
        getDriver().getFactory()->createInstance(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sPattern(OUStringToOString(schemaPattern, m_rConnection.getConnectionEncoding()).getStr()),
                tNamePattern(OUStringToOString(tableNamePattern, m_rConnection.getConnectionEncoding()).getStr());

    std::list<sql::SQLString> tabTypes;
    for (const OUString *pStart = types.getConstArray(), *p = pStart, *pEnd = pStart + nLength; p != pEnd; ++p) {
        tabTypes.push_back(OUStringToOString(*p, m_rConnection.getConnectionEncoding()).getStr());
    }

    try {
        std::auto_ptr< sql::ResultSet> rset( meta->getTables(cat,
                                               sPattern.compare("")? sPattern:wild,
                                               tNamePattern.compare("")? tNamePattern:wild,
                                               tabTypes));

        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            bool informationSchema = false;
            for (sal_uInt32 i = 1; (i <= columns) && !informationSchema; ++i) {
                sql::SQLString columnStringValue = rset->getString(i);
                if (i == 2) {   // TABLE_SCHEM
                    informationSchema = ( 0 == columnStringValue.compare("information_schema"));
                }
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(columnStringValue, encoding)));
            }
            if (!informationSchema) {
                rRows.push_back(aRow);
            }
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getTables", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getTables", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet, 4, rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getProcedureColumns() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
        const Any& /* catalog */,
        const OUString& /* schemaPattern */,
        const OUString& /* procedureNamePattern */,
        const OUString& /* columnNamePattern */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getProcedureColumns");
    // Currently there is no information available
    return NULL;
}
/* }}} */


/* {{{ ODatabaseMetaData::getProcedures() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
        const Any& catalog,
        const OUString& schemaPattern,
        const OUString& procedureNamePattern)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getProcedures");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sPattern(OUStringToOString(schemaPattern, m_rConnection.getConnectionEncoding()).getStr()),
                pNamePattern(OUStringToOString(procedureNamePattern, m_rConnection.getConnectionEncoding()).getStr());


    try {
        std::auto_ptr< sql::ResultSet> rset( meta->getProcedures(cat,
                                                   sPattern.compare("")? sPattern:wild,
                                                   pNamePattern.compare("")? pNamePattern:wild));

        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            for (sal_uInt32 i = 1; i <= columns; i++) {
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
            }
            rRows.push_back(aRow);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getProcedures", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getProcedures", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet, 7,rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getVersionColumns() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getVersionColumns(
        const Any& /* catalog */,
        const OUString& /* schema */,
        const OUString& /* table */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getVersionColumns");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    lcl_setRows_throw(xResultSet, 16,rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getExportedKeys() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
        const Any&  catalog ,
        const OUString&  schema ,
        const OUString&  table )
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getExportedKeys");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    std::string cat(catalog.hasValue()? OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::auto_ptr< sql::ResultSet> rset( meta->getExportedKeys(cat, sch, tab));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            for (sal_uInt32 i = 1; i <= columns; i++) {
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
            }
            rRows.push_back(aRow);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getExportedKeys", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getExportedKeys", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet, 8, rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getImportedKeys() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
        const Any& catalog,
        const OUString& schema,
        const OUString& table)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getImportedKeys");

    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::auto_ptr< sql::ResultSet> rset( meta->getImportedKeys(cat, sch, tab));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            for (sal_uInt32 i = 1; i <= columns; i++) {
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
            }
            rRows.push_back(aRow);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getImportedKeys", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getImportedKeys", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet,9,rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getPrimaryKeys() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
        const Any& catalog,
        const OUString& schema,
        const OUString& table)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getPrimaryKeys");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::auto_ptr< sql::ResultSet> rset( meta->getPrimaryKeys(cat, sch, tab));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            for (sal_uInt32 i = 1; i <= columns; i++) {
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
            }
            rRows.push_back(aRow);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getPrimaryKeys", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getPrimaryKeys", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet, 10, rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getIndexInfo() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
        const Any& catalog,
        const OUString& schema,
        const OUString& table,
        sal_Bool unique,
        sal_Bool approximate)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getIndexInfo");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::auto_ptr< sql::ResultSet> rset( meta->getIndexInfo(cat, sch, tab, unique, approximate));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            for (sal_uInt32 i = 1; i <= columns; i++) {
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
            }
            rRows.push_back(aRow);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getIndexInfo", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getIndexInfo", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet, 11, rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getBestRowIdentifier() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getBestRowIdentifier(
        const Any& catalog,
        const OUString& schema,
        const OUString& table,
        sal_Int32 scope,
        sal_Bool nullable)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getBestRowIdentifier");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::auto_ptr< sql::ResultSet> rset( meta->getBestRowIdentifier(cat, sch, tab, scope, nullable));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            for (sal_uInt32 i = 1; i <= columns; i++) {
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
            }
            rRows.push_back(aRow);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getBestRowIdentifier", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getBestRowIdentifier", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet, 15, rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getTablePrivileges() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
        const Any& catalog,
        const OUString& schemaPattern,
        const OUString& tableNamePattern)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getTablePrivileges");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sPattern(OUStringToOString(schemaPattern, m_rConnection.getConnectionEncoding()).getStr()),
                tPattern(OUStringToOString(tableNamePattern, m_rConnection.getConnectionEncoding()).getStr());

    try {
        static bool fakeTablePrivileges = false;
        if (fakeTablePrivileges) {
            static const sal_Char* allPrivileges[] = {
                "ALTER", "DELETE", "DROP", "INDEX", "INSERT", "LOCK TABLES", "SELECT", "UPDATE"
            };
            Any userName; userName <<= getUserName();
            for (size_t i = 0; i < SAL_N_ELEMENTS( allPrivileges ); ++i) {
                std::vector< Any > aRow;
                aRow.push_back(makeAny( sal_Int32( i ) ));
                aRow.push_back(catalog);                                                          // TABLE_CAT
                aRow.push_back(makeAny( schemaPattern ));                                         // TABLE_SCHEM
                aRow.push_back(makeAny( tableNamePattern ));                                      // TABLE_NAME
                aRow.push_back(Any());                                                            // GRANTOR
                aRow.push_back(userName);                                                         // GRANTEE
                aRow.push_back(makeAny( ::rtl::OUString::createFromAscii( allPrivileges[i] ) ));  // PRIVILEGE
                aRow.push_back(Any());                                                            // IS_GRANTABLE

                rRows.push_back(aRow);
            }
        } else {
            rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
            std::auto_ptr< sql::ResultSet> rset( meta->getTablePrivileges(cat, sPattern.compare("")? sPattern:wild, tPattern.compare("")? tPattern:wild));
            sql::ResultSetMetaData * rs_meta = rset->getMetaData();
            sal_uInt32 columns = rs_meta->getColumnCount();
            while (rset->next()) {
                std::vector< Any > aRow(1);
                for (sal_uInt32 i = 1; i <= columns; i++) {
                    aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
                }
                rRows.push_back(aRow);
            }
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getTablePrivileges", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getTablePrivileges", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet,12,rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getCrossReference() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
        const Any& primaryCatalog,
        const OUString& primarySchema,
        const OUString& primaryTable,
        const Any& foreignCatalog,
        const OUString& foreignSchema,
        const OUString& foreignTable)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getCrossReference");
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.DatabaseMetaDataResultSet"))),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string primaryCat(primaryCatalog.hasValue()? OUStringToOString(getStringFromAny(primaryCatalog), m_rConnection.getConnectionEncoding()).getStr():""),
                foreignCat(foreignCatalog.hasValue()? OUStringToOString(getStringFromAny(foreignCatalog), m_rConnection.getConnectionEncoding()).getStr():""),
                pSchema(OUStringToOString(primarySchema, m_rConnection.getConnectionEncoding()).getStr()),
                pTable(OUStringToOString(primaryTable, m_rConnection.getConnectionEncoding()).getStr()),
                fSchema(OUStringToOString(foreignSchema, m_rConnection.getConnectionEncoding()).getStr()),
                fTable(OUStringToOString(foreignTable, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::auto_ptr< sql::ResultSet> rset( meta->getCrossReference(primaryCat, pSchema, pTable, foreignCat, fSchema, fTable));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow(1);
            for (sal_uInt32 i = 1; i <= columns; i++) {
                aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(rset->getString(i), encoding)));
            }
            rRows.push_back(aRow);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getCrossReference", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::getCrossReference", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }

    lcl_setRows_throw(xResultSet,13,rRows);
    return xResultSet;
}
/* }}} */


/* {{{ ODatabaseMetaData::getUDTs() -I- */
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs(
        const Any& /* catalog */,
        const OUString& /* schemaPattern */,
        const OUString& /* typeNamePattern */,
        const Sequence< sal_Int32 >& /* types */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("ODatabaseMetaData::getUDTs");
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getUDTs", *this);
    return NULL;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
