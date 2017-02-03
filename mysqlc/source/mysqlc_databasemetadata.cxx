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
#include <memory>
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
using mysqlc_sdbc_driver::getStringFromAny;

#include <cppconn/connection.h>
#include <cppconn/resultset.h>
#include <cppconn/metadata.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <sal/macros.h>

static std::string wild("%");


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

ODatabaseMetaData::ODatabaseMetaData(OConnection& _rCon)
    :m_rConnection(_rCon)
    ,m_bUseCatalog(true)
    ,meta(_rCon.getConnectionSettings().cppConnection->getMetaData())
    ,identifier_quote_string_set(false)
{
    osl_atomic_increment(&m_refCount);
    m_bUseCatalog = !(usesLocalFiles() || usesLocalFilePerTable());
    osl_atomic_decrement(&m_refCount);
}

ODatabaseMetaData::~ODatabaseMetaData()
{
}

rtl::OUString ODatabaseMetaData::impl_getStringMetaData(const sal_Char* _methodName, const sql::SQLString& (sql::DatabaseMetaData::*Method)() )
{
    rtl::OUString stringMetaData;
    try {
        stringMetaData = mysqlc_sdbc_driver::convert((meta->*Method)(), m_rConnection.getConnectionEncoding());
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return stringMetaData;
}

rtl::OUString ODatabaseMetaData::impl_getStringMetaData(const sal_Char* _methodName, sql::SQLString (sql::DatabaseMetaData::*Method)() )
{
    rtl::OUString stringMetaData;
    try {
        stringMetaData = mysqlc_sdbc_driver::convert((meta->*Method)(), m_rConnection.getConnectionEncoding());
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return stringMetaData;
}

sal_Int32 ODatabaseMetaData::impl_getInt32MetaData(const sal_Char* _methodName, unsigned int (sql::DatabaseMetaData::*Method)() )
{
    sal_Int32 int32MetaData(0);
    try {
        int32MetaData = (meta->*Method)();
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return int32MetaData;
}

bool ODatabaseMetaData::impl_getBoolMetaData(const sal_Char* _methodName, bool (sql::DatabaseMetaData::*Method)() )
{
    bool boolMetaData(false);
    try {
        boolMetaData = (meta->*Method)();
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return boolMetaData;
}

bool ODatabaseMetaData::impl_getBoolMetaData(const sal_Char* _methodName, bool (sql::DatabaseMetaData::*Method)(int), sal_Int32 _arg )
{
    bool boolMetaData(false);
    try {
        boolMetaData = (meta->*Method)( _arg );
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException(_methodName, *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException(_methodName, *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return boolMetaData;
}

bool ODatabaseMetaData::impl_getRSTypeMetaData(const sal_Char* _methodName, bool (sql::DatabaseMetaData::*Method)(int), sal_Int32 _resultSetType )
{
    int resultSetType(sql::ResultSet::TYPE_FORWARD_ONLY);
    switch ( _resultSetType ) {
        case ResultSetType::SCROLL_INSENSITIVE: resultSetType = sql::ResultSet::TYPE_SCROLL_INSENSITIVE;    break;
        case ResultSetType::SCROLL_SENSITIVE:   resultSetType = sql::ResultSet::TYPE_SCROLL_SENSITIVE;      break;
    }

    return impl_getBoolMetaData(_methodName, Method, resultSetType);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogSeparator()
{
    return impl_getStringMetaData("getCatalogSeparator", &sql::DatabaseMetaData::getCatalogSeparator);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength()
{
    return impl_getInt32MetaData("getMaxBinaryLiteralLength", &sql::DatabaseMetaData::getMaxBinaryLiteralLength);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize()
{
    return impl_getInt32MetaData("getMaxRowSize", &sql::DatabaseMetaData::getMaxRowSize);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength()
{
    return impl_getInt32MetaData("getMaxCatalogNameLength", &sql::DatabaseMetaData::getMaxCatalogNameLength);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength()
{
    return impl_getInt32MetaData("getMaxCharLiteralLength", &sql::DatabaseMetaData::getMaxCharLiteralLength);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength()
{
    return impl_getInt32MetaData("getMaxColumnNameLength", &sql::DatabaseMetaData::getMaxColumnNameLength);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex()
{
    return impl_getInt32MetaData("getMaxColumnsInIndex", &sql::DatabaseMetaData::getMaxColumnsInIndex);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength()
{
    return impl_getInt32MetaData("getMaxCursorNameLength", &sql::DatabaseMetaData::getMaxCursorNameLength);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections()
{
    return impl_getInt32MetaData("getMaxConnections", &sql::DatabaseMetaData::getMaxConnections);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable()
{
    return impl_getInt32MetaData("getMaxColumnsInTable", &sql::DatabaseMetaData::getMaxColumnsInTable);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength()
{
    return impl_getInt32MetaData("getMaxStatementLength", &sql::DatabaseMetaData::getMaxStatementLength);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength()
{
    return impl_getInt32MetaData("getMaxTableNameLength", &sql::DatabaseMetaData::getMaxTableNameLength);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTablesInSelect()
{
    return impl_getInt32MetaData("getMaxTablesInSelect", &sql::DatabaseMetaData::getMaxTablesInSelect);
}

sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs()
{
    return impl_getBoolMetaData("doesMaxRowSizeIncludeBlobs", &sql::DatabaseMetaData::doesMaxRowSizeIncludeBlobs);
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers()
{
    return impl_getBoolMetaData("storesLowerCaseQuotedIdentifiers", &sql::DatabaseMetaData::storesLowerCaseQuotedIdentifiers);
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers()
{
    return impl_getBoolMetaData("storesLowerCaseIdentifiers", &sql::DatabaseMetaData::storesLowerCaseIdentifiers);
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseQuotedIdentifiers()
{
    return impl_getBoolMetaData("storesMixedCaseQuotedIdentifiers", &sql::DatabaseMetaData::storesMixedCaseQuotedIdentifiers);
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers()
{
    return impl_getBoolMetaData("storesMixedCaseIdentifiers", &sql::DatabaseMetaData::storesMixedCaseIdentifiers);
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers()
{
    return impl_getBoolMetaData("storesUpperCaseQuotedIdentifiers", &sql::DatabaseMetaData::storesUpperCaseQuotedIdentifiers);
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers()
{
    return impl_getBoolMetaData("storesUpperCaseIdentifiers", &sql::DatabaseMetaData::storesUpperCaseIdentifiers);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithAddColumn()
{
    return impl_getBoolMetaData("supportsAlterTableWithAddColumn", &sql::DatabaseMetaData::supportsAlterTableWithAddColumn);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithDropColumn()
{
    return impl_getBoolMetaData("supportsAlterTableWithDropColumn", &sql::DatabaseMetaData::supportsAlterTableWithDropColumn);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength()
{
    return impl_getInt32MetaData("getMaxIndexLength", &sql::DatabaseMetaData::getMaxIndexLength);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns()
{
    return impl_getBoolMetaData("supportsNonNullableColumns", &sql::DatabaseMetaData::supportsNonNullableColumns);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogTerm()
{
    return impl_getStringMetaData("getCatalogTerm", &sql::DatabaseMetaData::getCatalogTerm);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getIdentifierQuoteString()
{
    if (!identifier_quote_string_set) {
        identifier_quote_string = impl_getStringMetaData("getIdentifierQuoteString", &sql::DatabaseMetaData::getIdentifierQuoteString);
        identifier_quote_string_set = true;
    }
    return identifier_quote_string;
}

rtl::OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters()
{
    return impl_getStringMetaData("getExtraNameCharacters", &sql::DatabaseMetaData::getExtraNameCharacters);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames()
{
    return impl_getBoolMetaData("supportsDifferentTableCorrelationNames", &sql::DatabaseMetaData::supportsDifferentTableCorrelationNames);
}

sal_Bool SAL_CALL ODatabaseMetaData::isCatalogAtStart()
{
    return impl_getBoolMetaData("isCatalogAtStart", &sql::DatabaseMetaData::isCatalogAtStart);
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions()
{
    return impl_getBoolMetaData("dataDefinitionIgnoredInTransactions", &sql::DatabaseMetaData::dataDefinitionIgnoredInTransactions);
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit()
{
    return impl_getBoolMetaData("dataDefinitionCausesTransactionCommit", &sql::DatabaseMetaData::dataDefinitionCausesTransactionCommit);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly()
{
    return impl_getBoolMetaData("supportsDataManipulationTransactionsOnly", &sql::DatabaseMetaData::supportsDataManipulationTransactionsOnly);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions()
{
    return impl_getBoolMetaData("supportsDataDefinitionAndDataManipulationTransactions", &sql::DatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete()
{
    return impl_getBoolMetaData("supportsPositionedDelete", &sql::DatabaseMetaData::supportsPositionedDelete);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate()
{
    return impl_getBoolMetaData("supportsPositionedUpdate", &sql::DatabaseMetaData::supportsPositionedUpdate);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback()
{
    return impl_getBoolMetaData("supportsOpenStatementsAcrossRollback", &sql::DatabaseMetaData::supportsOpenStatementsAcrossRollback);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit()
{
    return impl_getBoolMetaData("supportsOpenStatementsAcrossCommit", &sql::DatabaseMetaData::supportsOpenStatementsAcrossCommit);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit()
{
    return impl_getBoolMetaData("supportsOpenCursorsAcrossCommit", &sql::DatabaseMetaData::supportsOpenCursorsAcrossCommit);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback()
{
    return impl_getBoolMetaData("supportsOpenCursorsAcrossRollback", &sql::DatabaseMetaData::supportsOpenCursorsAcrossRollback);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel(sal_Int32 level)
{
    return impl_getBoolMetaData("supportsTransactionIsolationLevel", &sql::DatabaseMetaData::supportsTransactionIsolationLevel, level);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInDataManipulation()
{
    return impl_getBoolMetaData("supportsSchemasInDataManipulation", &sql::DatabaseMetaData::supportsSchemasInDataManipulation);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL()
{
    return impl_getBoolMetaData("supportsANSI92FullSQL", &sql::DatabaseMetaData::supportsANSI92FullSQL);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL()
{
    return impl_getBoolMetaData("supportsANSI92EntryLevelSQL", &sql::DatabaseMetaData::supportsANSI92EntryLevelSQL);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility()
{
    return impl_getBoolMetaData("supportsIntegrityEnhancementFacility", &sql::DatabaseMetaData::supportsIntegrityEnhancementFacility);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions()
{
    return impl_getBoolMetaData("supportsSchemasInIndexDefinitions", &sql::DatabaseMetaData::supportsSchemasInIndexDefinitions);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInTableDefinitions()
{
    return impl_getBoolMetaData("supportsSchemasInTableDefinitions", &sql::DatabaseMetaData::supportsSchemasInTableDefinitions);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInTableDefinitions()
{
    return impl_getBoolMetaData("supportsCatalogsInTableDefinitions", &sql::DatabaseMetaData::supportsCatalogsInTableDefinitions);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions()
{
    return impl_getBoolMetaData("supportsCatalogsInIndexDefinitions", &sql::DatabaseMetaData::supportsCatalogsInIndexDefinitions);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInDataManipulation()
{
    return impl_getBoolMetaData("supportsCatalogsInDataManipulation", &sql::DatabaseMetaData::supportsCatalogsInDataManipulation);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins()
{
    return impl_getBoolMetaData("supportsOuterJoins", &sql::DatabaseMetaData::supportsOuterJoins);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatements()
{
    return impl_getInt32MetaData("getMaxStatements", &sql::DatabaseMetaData::getMaxStatements);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength()
{
    return impl_getInt32MetaData("getMaxProcedureNameLength", &sql::DatabaseMetaData::getMaxProcedureNameLength);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength()
{
    return impl_getInt32MetaData("getMaxSchemaNameLength", &sql::DatabaseMetaData::getMaxSchemaNameLength);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions()
{
    return impl_getBoolMetaData("supportsTransactions", &sql::DatabaseMetaData::supportsTransactions);
}

sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable()
{
    return impl_getBoolMetaData("allProceduresAreCallable", &sql::DatabaseMetaData::allProceduresAreCallable);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures()
{
    return impl_getBoolMetaData("supportsStoredProcedures", &sql::DatabaseMetaData::supportsStoredProcedures);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate()
{
    return impl_getBoolMetaData("supportsSelectForUpdate", &sql::DatabaseMetaData::supportsSelectForUpdate);
}

sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable()
{
    return impl_getBoolMetaData("allTablesAreSelectable", &sql::DatabaseMetaData::allTablesAreSelectable);
}

sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly()
{
    return impl_getBoolMetaData("isReadOnly", &sql::DatabaseMetaData::isReadOnly);
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles()
{
    return impl_getBoolMetaData("usesLocalFiles", &sql::DatabaseMetaData::usesLocalFiles);
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable()
{
    return impl_getBoolMetaData("usesLocalFilePerTable", &sql::DatabaseMetaData::usesLocalFilePerTable);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion()
{
    return impl_getBoolMetaData("supportsTypeConversion", &sql::DatabaseMetaData::supportsTypeConversion);
}

sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull()
{
    return impl_getBoolMetaData("nullPlusNonNullIsNull", &sql::DatabaseMetaData::nullPlusNonNullIsNull);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing()
{
    return impl_getBoolMetaData("supportsColumnAliasing", &sql::DatabaseMetaData::supportsColumnAliasing);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames()
{
    return impl_getBoolMetaData("supportsTableCorrelationNames", &sql::DatabaseMetaData::supportsTableCorrelationNames);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert(sal_Int32 /* fromType */, sal_Int32 /* toType */)
{
    try {
        /* ToDo -> use supportsConvert( fromType, toType) */
        return meta->supportsConvert();
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::supportsConvert", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::supportsConvert", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy()
{
    return impl_getBoolMetaData("supportsExpressionsInOrderBy", &sql::DatabaseMetaData::supportsExpressionsInOrderBy);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy()
{
    return impl_getBoolMetaData("supportsGroupBy", &sql::DatabaseMetaData::supportsGroupBy);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect()
{
    return impl_getBoolMetaData("supportsGroupByBeyondSelect", &sql::DatabaseMetaData::supportsGroupByBeyondSelect);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated()
{
    return impl_getBoolMetaData("supportsGroupByUnrelated", &sql::DatabaseMetaData::supportsGroupByUnrelated);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions()
{
    return impl_getBoolMetaData("supportsMultipleTransactions", &sql::DatabaseMetaData::supportsMultipleTransactions);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets()
{
    return impl_getBoolMetaData("supportsMultipleResultSets", &sql::DatabaseMetaData::supportsMultipleResultSets);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause()
{
    return impl_getBoolMetaData("supportsLikeEscapeClause", &sql::DatabaseMetaData::supportsLikeEscapeClause);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated()
{
    return impl_getBoolMetaData("supportsOrderByUnrelated", &sql::DatabaseMetaData::supportsOrderByUnrelated);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion()
{
    return impl_getBoolMetaData("supportsUnion", &sql::DatabaseMetaData::supportsUnion);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll()
{
    return impl_getBoolMetaData("supportsUnionAll", &sql::DatabaseMetaData::supportsUnionAll);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers()
{
    return impl_getBoolMetaData("supportsMixedCaseIdentifiers", &sql::DatabaseMetaData::supportsMixedCaseIdentifiers);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseQuotedIdentifiers()
{
    return impl_getBoolMetaData("supportsMixedCaseQuotedIdentifiers", &sql::DatabaseMetaData::supportsMixedCaseQuotedIdentifiers);
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd()
{
    return impl_getBoolMetaData("nullsAreSortedAtEnd", &sql::DatabaseMetaData::nullsAreSortedAtEnd);
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart()
{
    return impl_getBoolMetaData("nullsAreSortedAtStart", &sql::DatabaseMetaData::nullsAreSortedAtStart);
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh()
{
    return impl_getBoolMetaData("nullsAreSortedHigh", &sql::DatabaseMetaData::nullsAreSortedHigh);
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow()
{
    return impl_getBoolMetaData("nullsAreSortedLow", &sql::DatabaseMetaData::nullsAreSortedLow);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls()
{
    return impl_getBoolMetaData("supportsSchemasInProcedureCalls", &sql::DatabaseMetaData::supportsSchemasInProcedureCalls);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions()
{
    return impl_getBoolMetaData("supportsSchemasInPrivilegeDefinitions", &sql::DatabaseMetaData::supportsSchemasInPrivilegeDefinitions);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls()
{
    return impl_getBoolMetaData("supportsCatalogsInProcedureCalls", &sql::DatabaseMetaData::supportsCatalogsInProcedureCalls);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions()
{
    return impl_getBoolMetaData("supportsCatalogsInPrivilegeDefinitions", &sql::DatabaseMetaData::supportsCatalogsInPrivilegeDefinitions);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries()
{
    return impl_getBoolMetaData("supportsCorrelatedSubqueries", &sql::DatabaseMetaData::supportsCorrelatedSubqueries);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons()
{
    return impl_getBoolMetaData("supportsSubqueriesInComparisons", &sql::DatabaseMetaData::supportsSubqueriesInComparisons);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists()
{
    return impl_getBoolMetaData("supportsSubqueriesInExists", &sql::DatabaseMetaData::supportsSubqueriesInExists);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns()
{
    return impl_getBoolMetaData("supportsSubqueriesInIns", &sql::DatabaseMetaData::supportsSubqueriesInIns);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds()
{
    return impl_getBoolMetaData("supportsSubqueriesInQuantifieds", &sql::DatabaseMetaData::supportsSubqueriesInQuantifieds);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL()
{
    return impl_getBoolMetaData("supportsANSI92IntermediateSQL", &sql::DatabaseMetaData::supportsANSI92IntermediateSQL);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getURL()
{
    return m_rConnection.getConnectionSettings().connectionURL;
}

rtl::OUString SAL_CALL ODatabaseMetaData::getUserName()
{
    return impl_getStringMetaData("getUserName", &sql::DatabaseMetaData::getUserName);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getDriverName()
{
    rtl::OUString aValue( "MySQL Connector/OO.org" );
    return aValue;
}

rtl::OUString SAL_CALL ODatabaseMetaData::getDriverVersion()
{
    return rtl::OUString( "0.9.2" );
}

rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion()
{
    return impl_getStringMetaData("getDatabaseProductVersion", &sql::DatabaseMetaData::getDatabaseProductVersion);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName()
{
    return impl_getStringMetaData("getDatabaseProductName", &sql::DatabaseMetaData::getDatabaseProductName);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getProcedureTerm()
{
    return impl_getStringMetaData("getProcedureTerm", &sql::DatabaseMetaData::getProcedureTerm);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getSchemaTerm()
{
    return impl_getStringMetaData("getSchemaTerm", &sql::DatabaseMetaData::getSchemaTerm);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion()
{
    return MARIADBC_VERSION_MAJOR;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation()
{
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

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion()
{
    return MARIADBC_VERSION_MINOR;
}

rtl::OUString SAL_CALL ODatabaseMetaData::getSQLKeywords()
{
    return impl_getStringMetaData("getSQLKeywords", &sql::DatabaseMetaData::getSQLKeywords);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape()
{
    return impl_getStringMetaData("getSearchStringEscape", &sql::DatabaseMetaData::getSearchStringEscape);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getStringFunctions()
{
    return impl_getStringMetaData("getStringFunctions", &sql::DatabaseMetaData::getStringFunctions);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions()
{
    return impl_getStringMetaData("getTimeDateFunctions", &sql::DatabaseMetaData::getTimeDateFunctions);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getSystemFunctions()
{
    return impl_getStringMetaData("getSystemFunctions", &sql::DatabaseMetaData::getSystemFunctions);
}

rtl::OUString SAL_CALL ODatabaseMetaData::getNumericFunctions()
{
    return impl_getStringMetaData("getNumericFunctions", &sql::DatabaseMetaData::getNumericFunctions);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar()
{
    return impl_getBoolMetaData("supportsExtendedSQLGrammar", &sql::DatabaseMetaData::supportsExtendedSQLGrammar);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar()
{
    return impl_getBoolMetaData("supportsCoreSQLGrammar", &sql::DatabaseMetaData::supportsCoreSQLGrammar);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar()
{
    return impl_getBoolMetaData("supportsMinimumSQLGrammar", &sql::DatabaseMetaData::supportsMinimumSQLGrammar);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins()
{
    return impl_getBoolMetaData("supportsFullOuterJoins", &sql::DatabaseMetaData::supportsFullOuterJoins);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins()
{
    return impl_getBoolMetaData("supportsLimitedOuterJoins", &sql::DatabaseMetaData::supportsLimitedOuterJoins);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy()
{
    return impl_getInt32MetaData("getMaxColumnsInGroupBy", &sql::DatabaseMetaData::getMaxColumnsInGroupBy);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy()
{
    return impl_getInt32MetaData("getMaxColumnsInOrderBy", &sql::DatabaseMetaData::getMaxColumnsInOrderBy);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect()
{
    return impl_getInt32MetaData("getMaxColumnsInSelect", &sql::DatabaseMetaData::getMaxColumnsInSelect);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength()
{
    return impl_getInt32MetaData("getMaxUserNameLength", &sql::DatabaseMetaData::getMaxUserNameLength);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType(sal_Int32 setType)
{
    return impl_getRSTypeMetaData("supportsResultSetType", &sql::DatabaseMetaData::supportsResultSetType, setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency(sal_Int32 setType, sal_Int32 concurrency)
{
    /* TODO: Check this out */
    try {
        return meta->supportsResultSetConcurrency(setType, concurrency==css::sdbc::TransactionIsolation::READ_COMMITTED?
                                                    sql::TRANSACTION_READ_COMMITTED:
                                                    (concurrency == css::sdbc::TransactionIsolation::SERIALIZABLE?
                                                        sql::TRANSACTION_SERIALIZABLE:sql::TRANSACTION_SERIALIZABLE));
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::supportsResultSetConcurrency", *this);
    } catch (const sql::InvalidArgumentException &) {
        mysqlc_sdbc_driver::throwInvalidArgumentException("ODatabaseMetaData::supportsResultSetConcurrency", *this);
    } catch (const sql::SQLException& e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_rConnection.getConnectionEncoding());
    }
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible(sal_Int32 setType)
{
    return impl_getRSTypeMetaData("ownUpdatesAreVisible", &sql::DatabaseMetaData::ownUpdatesAreVisible, setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible(sal_Int32 setType)
{
    return impl_getRSTypeMetaData("ownDeletesAreVisible", &sql::DatabaseMetaData::ownDeletesAreVisible, setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible(sal_Int32 setType)
{
    return impl_getRSTypeMetaData("ownInsertsAreVisible", &sql::DatabaseMetaData::ownInsertsAreVisible, setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible(sal_Int32 setType)
{
    return impl_getRSTypeMetaData("othersUpdatesAreVisible", &sql::DatabaseMetaData::othersUpdatesAreVisible, setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible(sal_Int32 setType)
{
    return impl_getRSTypeMetaData("othersDeletesAreVisible", &sql::DatabaseMetaData::othersDeletesAreVisible, setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible(sal_Int32 setType)
{
    return impl_getRSTypeMetaData("othersInsertsAreVisible", &sql::DatabaseMetaData::othersInsertsAreVisible, setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected(sal_Int32 setType)
{
    return impl_getRSTypeMetaData("updatesAreDetected", &sql::DatabaseMetaData::updatesAreDetected, setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected(sal_Int32 setType)
{
    return impl_getRSTypeMetaData("deletesAreDetected", &sql::DatabaseMetaData::deletesAreDetected, setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected(sal_Int32 setType)
{
    return impl_getRSTypeMetaData("insertsAreDetected", &sql::DatabaseMetaData::insertsAreDetected, setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates()
{
    return impl_getBoolMetaData("supportsBatchUpdates", &sql::DatabaseMetaData::supportsBatchUpdates);
}

Reference< XConnection > SAL_CALL ODatabaseMetaData::getConnection()
{
    return &m_rConnection;
}

/*
  Here follow all methods which return(a resultset
  the first methods is an example implementation how to use this resultset
  of course you could implement it on your and you should do this because
  the general way is more memory expensive
*/

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes()
{
    const char * table_types[] = {"TABLE", "VIEW"};
    sal_Int32 requiredVersion[] = {0, 50000};

    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();

    for (sal_uInt32 i = 0; i < 2; i++) {
        if (m_rConnection.getMysqlVersion() >= requiredVersion[i]) {
            std::vector< Any > aRow { Any() };
            aRow.push_back(makeAny(mysqlc_sdbc_driver::convert(table_types[i], encoding)));
            rRows.push_back(aRow);
        }
    }
    lcl_setRows_throw(xResultSet, 5 ,rRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTypeInfo()
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);

    std::vector< std::vector< Any > > rRows;

    rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
    unsigned int i = 0;
    while (mysqlc_types[i].typeName) {
        std::vector< Any > aRow { Any() };

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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCatalogs()
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::unique_ptr< sql::ResultSet> rset( meta->getCatalogs());
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas()
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::unique_ptr< sql::ResultSet> rset( meta->getSchemas());
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
        const Any& catalog,
        const rtl::OUString& schema,
        const rtl::OUString& table,
        const rtl::OUString& columnNamePattern)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? rtl::OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(rtl::OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(rtl::OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr()),
                cNamePattern(rtl::OUStringToOString(columnNamePattern, m_rConnection.getConnectionEncoding()).getStr());
    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::unique_ptr< sql::ResultSet> rset( meta->getColumnPrivileges(cat, sch, tab, cNamePattern.compare("")? cNamePattern:wild));

        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
        const Any& catalog,
        const rtl::OUString& schemaPattern,
        const rtl::OUString& tableNamePattern,
        const rtl::OUString& columnNamePattern)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    std::string cat(catalog.hasValue()? rtl::OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sPattern(rtl::OUStringToOString(schemaPattern, m_rConnection.getConnectionEncoding()).getStr()),
                tNamePattern(rtl::OUStringToOString(tableNamePattern, m_rConnection.getConnectionEncoding()).getStr()),
                cNamePattern(rtl::OUStringToOString(columnNamePattern, m_rConnection.getConnectionEncoding()).getStr());

    try {
        std::unique_ptr< sql::ResultSet> rset( meta->getColumns(cat,
                                                sPattern.compare("")? sPattern:wild,
                                                tNamePattern.compare("")? tNamePattern:wild,
                                                cNamePattern.compare("")? cNamePattern:wild));
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
        const Any& catalog,
        const rtl::OUString& schemaPattern,
        const rtl::OUString& tableNamePattern,
        const Sequence< rtl::OUString >& types )
{
    sal_Int32 nLength = types.getLength();

    Reference< XResultSet > xResultSet(getOwnConnection().
        getDriver().getFactory()->createInstance(
                "org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? rtl::OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sPattern(rtl::OUStringToOString(schemaPattern, m_rConnection.getConnectionEncoding()).getStr()),
                tNamePattern(rtl::OUStringToOString(tableNamePattern, m_rConnection.getConnectionEncoding()).getStr());

    std::list<sql::SQLString> tabTypes;
    for (const rtl::OUString *pStart = types.getConstArray(), *p = pStart, *pEnd = pStart + nLength; p != pEnd; ++p) {
        tabTypes.push_back(rtl::OUStringToOString(*p, m_rConnection.getConnectionEncoding()).getStr());
    }

    try {
        std::unique_ptr< sql::ResultSet> rset( meta->getTables(cat,
                                               sPattern.compare("")? sPattern:wild,
                                               tNamePattern.compare("")? tNamePattern:wild,
                                               tabTypes));

        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
        const Any& /* catalog */,
        const rtl::OUString& /* schemaPattern */,
        const rtl::OUString& /* procedureNamePattern */,
        const rtl::OUString& /* columnNamePattern */)
{
    // Currently there is no information available
    return nullptr;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
        const Any& catalog,
        const rtl::OUString& schemaPattern,
        const rtl::OUString& procedureNamePattern)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? rtl::OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sPattern(rtl::OUStringToOString(schemaPattern, m_rConnection.getConnectionEncoding()).getStr()),
                procNamePattern(rtl::OUStringToOString(procedureNamePattern, m_rConnection.getConnectionEncoding()).getStr());


    try {
        std::unique_ptr< sql::ResultSet> rset( meta->getProcedures(cat,
                                                   sPattern.compare("")? sPattern:wild,
                                                   procNamePattern.compare("")? procNamePattern:wild));

        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getVersionColumns(
        const Any& /* catalog */,
        const rtl::OUString& /* schema */,
        const rtl::OUString& /* table */)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    lcl_setRows_throw(xResultSet, 16,rRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
        const Any&  catalog ,
        const rtl::OUString&  schema ,
        const rtl::OUString&  table )
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    std::string cat(catalog.hasValue()? rtl::OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(rtl::OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(rtl::OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::unique_ptr< sql::ResultSet> rset( meta->getExportedKeys(cat, sch, tab));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
        const Any& catalog,
        const rtl::OUString& schema,
        const rtl::OUString& table)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? rtl::OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(rtl::OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(rtl::OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::unique_ptr< sql::ResultSet> rset( meta->getImportedKeys(cat, sch, tab));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
        const Any& catalog,
        const rtl::OUString& schema,
        const rtl::OUString& table)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? rtl::OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(rtl::OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(rtl::OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::unique_ptr< sql::ResultSet> rset( meta->getPrimaryKeys(cat, sch, tab));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
        const Any& catalog,
        const rtl::OUString& schema,
        const rtl::OUString& table,
        sal_Bool unique,
        sal_Bool approximate)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? rtl::OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(rtl::OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(rtl::OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::unique_ptr< sql::ResultSet> rset( meta->getIndexInfo(cat, sch, tab, unique, approximate));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getBestRowIdentifier(
        const Any& catalog,
        const rtl::OUString& schema,
        const rtl::OUString& table,
        sal_Int32 scope,
        sal_Bool nullable)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? rtl::OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sch(rtl::OUStringToOString(schema, m_rConnection.getConnectionEncoding()).getStr()),
                tab(rtl::OUStringToOString(table, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::unique_ptr< sql::ResultSet> rset( meta->getBestRowIdentifier(cat, sch, tab, scope, nullable));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
        const Any& catalog,
        const rtl::OUString& schemaPattern,
        const rtl::OUString& tableNamePattern)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string cat(catalog.hasValue()? rtl::OUStringToOString(getStringFromAny(catalog), m_rConnection.getConnectionEncoding()).getStr():""),
                sPattern(rtl::OUStringToOString(schemaPattern, m_rConnection.getConnectionEncoding()).getStr()),
                tPattern(rtl::OUStringToOString(tableNamePattern, m_rConnection.getConnectionEncoding()).getStr());

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
                aRow.push_back(makeAny( rtl::OUString::createFromAscii( allPrivileges[i] ) ));  // PRIVILEGE
                aRow.push_back(Any());                                                            // IS_GRANTABLE

                rRows.push_back(aRow);
            }
        } else {
            rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
            std::unique_ptr< sql::ResultSet> rset( meta->getTablePrivileges(cat, sPattern.compare("")? sPattern:wild, tPattern.compare("")? tPattern:wild));
            sql::ResultSetMetaData * rs_meta = rset->getMetaData();
            sal_uInt32 columns = rs_meta->getColumnCount();
            while (rset->next()) {
                std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
        const Any& primaryCatalog,
        const rtl::OUString& primarySchema_,
        const rtl::OUString& primaryTable_,
        const Any& foreignCatalog,
        const rtl::OUString& foreignSchema,
        const rtl::OUString& foreignTable)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    std::string primaryCat(primaryCatalog.hasValue()? rtl::OUStringToOString(getStringFromAny(primaryCatalog), m_rConnection.getConnectionEncoding()).getStr():""),
                foreignCat(foreignCatalog.hasValue()? rtl::OUStringToOString(getStringFromAny(foreignCatalog), m_rConnection.getConnectionEncoding()).getStr():""),
                primarySchema(rtl::OUStringToOString(primarySchema_, m_rConnection.getConnectionEncoding()).getStr()),
                primaryTable(rtl::OUStringToOString(primaryTable_, m_rConnection.getConnectionEncoding()).getStr()),
                fSchema(rtl::OUStringToOString(foreignSchema, m_rConnection.getConnectionEncoding()).getStr()),
                fTable(rtl::OUStringToOString(foreignTable, m_rConnection.getConnectionEncoding()).getStr());

    try {
        rtl_TextEncoding encoding = m_rConnection.getConnectionEncoding();
        std::unique_ptr< sql::ResultSet> rset( meta->getCrossReference(primaryCat, primarySchema, primaryTable, foreignCat, fSchema, fTable));
        sql::ResultSetMetaData * rs_meta = rset->getMetaData();
        sal_uInt32 columns = rs_meta->getColumnCount();
        while (rset->next()) {
            std::vector< Any > aRow { Any() };
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs(
        const Any& /* catalog */,
        const rtl::OUString& /* schemaPattern */,
        const rtl::OUString& /* typeNamePattern */,
        const Sequence< sal_Int32 >& /* types */)
{
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("ODatabaseMetaData::getUDTs", *this);
    return nullptr;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
