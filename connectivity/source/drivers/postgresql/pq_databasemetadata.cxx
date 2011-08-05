/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  $RCSfile: pq_databasemetadata.cxx,v $
 *
 *  $Revision: 1.1.2.11 $
 *
 *  last change: $Author: jbu $ $Date: 2007/02/15 20:04:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Joerg Budischewski
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *              2011 Lionel Elie Mamane <lionel@mamane.lu>
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Joerg Budischewski
 *
 *  Non-Sun changes:
 *  * august 2011: switch to unordered_map instead of deprecated hash_map
 *  * august 2011: calcSearchable: actually set return value, not fresh variable.
 *
 ************************************************************************/
#include <algorithm>
#include "pq_databasemetadata.hxx"
#include "pq_driver.hxx"
#include "pq_sequenceresultset.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

#include <rtl/ustrbuf.hxx>

#include<com/sun/star/sdbc/TransactionIsolation.hpp>
#include<com/sun/star/sdbc/ResultSetType.hpp>
#include<com/sun/star/sdbc/XPreparedStatement.hpp>
#include<com/sun/star/sdbc/XParameters.hpp>
#include<com/sun/star/sdbc/DataType.hpp>
#include<com/sun/star/sdbc/IndexType.hpp>
#include<com/sun/star/sdbc/ColumnValue.hpp>
#include<com/sun/star/sdbc/ColumnSearch.hpp>

using ::osl::MutexGuard;

using ::rtl::OUString;

using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XPreparedStatement;
//  using com::sun::star::sdbc::IndexType;
//  using com::sun::star::sdbc::DataType;

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;

using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;

namespace pq_sdbc_driver
{
typedef
std::vector
<
    com::sun::star::uno::Sequence< com::sun::star::uno::Any >,
    Allocator< com::sun::star::uno::Sequence< com::sun::star::uno::Any > >
> SequenceAnyVector;


#define ASCII_STR(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )

static const int MAX_COLUMNS_IN_GROUPBY = 16;
static const int MAX_COLUMNS_IN_INDEX = 32;
static const int MAX_COLUMNS_IN_ORDER_BY = 16;
static const int MAX_COLUMNS_IN_SELECT = 1024;
static const int MAX_IDENTIFIER_LENGTH = 63;
static const int MAX_COLUMNS_IN_TABLE = 1024;
static const int MAX_CONNECTIONS = 0xffff;
static const int MAX_STATEMENTS = 0xffff;
static const int MAX_STATEMENT_LENGTH = -1;
static const int MAX_TABLES_IN_SELECT = 0xffff;
static const int MAX_USER_NAME_LENGTH = MAX_IDENTIFIER_LENGTH;


// alphabetically ordered !
static const int PRIVILEGE_CREATE     = 0x1;
static const int PRIVILEGE_DELETE     = 0x2;
static const int PRIVILEGE_EXECUTE    = 0x4;
static const int PRIVILEGE_INSERT     = 0x8;
static const int PRIVILEGE_REFERENCES = 0x10;
static const int PRIVILEGE_RULE       = 0x20;
static const int PRIVILEGE_SELECT     = 0x40;
static const int PRIVILEGE_TEMPORARY  = 0x80;
static const int PRIVILEGE_TRIGGER    = 0x100;
static const int PRIVILEGE_UPDATE     = 0x200;
static const int PRIVILEGE_USAGE      = 0x400;
static const int PRIVILEGE_MAX = PRIVILEGE_USAGE;

void DatabaseMetaData::checkClosed()
        throw (SQLException, RuntimeException)
{
}

DatabaseMetaData::DatabaseMetaData(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings )
  : m_refMutex( refMutex ),
    m_pSettings( pSettings ),
    m_origin( origin )
{
}

sal_Bool DatabaseMetaData::allProceduresAreCallable(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return sal_False;
}

sal_Bool DatabaseMetaData::allTablesAreSelectable(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

OUString DatabaseMetaData::getURL(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return OUString();
}

OUString DatabaseMetaData::getUserName(  ) throw (SQLException, RuntimeException)
{
    return m_pSettings->user;
}

sal_Bool DatabaseMetaData::isReadOnly(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}


sal_Bool DatabaseMetaData::nullsAreSortedHigh(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::nullsAreSortedLow(  ) throw (SQLException, RuntimeException)
{
    return ! nullsAreSortedHigh();
}

sal_Bool DatabaseMetaData::nullsAreSortedAtStart(  ) throw (SQLException, RuntimeException)
{
    return ! nullsAreSortedHigh();
}

sal_Bool DatabaseMetaData::nullsAreSortedAtEnd(  ) throw (SQLException, RuntimeException)
{
    return nullsAreSortedHigh();
}

OUString DatabaseMetaData::getDatabaseProductName(  ) throw (SQLException, RuntimeException)
{
    return ASCII_STR( "postgresql");
}

OUString DatabaseMetaData::getDatabaseProductVersion(  ) throw (SQLException, RuntimeException)
{
    return ASCII_STR( POSTGRESQL_VERSION );
}
OUString DatabaseMetaData::getDriverName(  ) throw (SQLException, RuntimeException)
{
    return ASCII_STR( "postgresql-sdbc" );
}

OUString DatabaseMetaData::getDriverVersion(  ) throw (SQLException, RuntimeException)
{
    return ASCII_STR( PQ_SDBC_DRIVER_VERSION );
}

sal_Int32 DatabaseMetaData::getDriverMajorVersion(  ) throw (RuntimeException)
{
    return PQ_SDBC_MAJOR;
}

sal_Int32 DatabaseMetaData::getDriverMinorVersion(  ) throw (RuntimeException)
{
    return PQ_SDBC_MINOR;
}

sal_Bool DatabaseMetaData::usesLocalFiles(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::usesLocalFilePerTable(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::storesUpperCaseIdentifiers(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::storesLowerCaseIdentifiers(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}


sal_Bool DatabaseMetaData::storesMixedCaseIdentifiers(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}


sal_Bool DatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}


sal_Bool DatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}


sal_Bool DatabaseMetaData::storesMixedCaseQuotedIdentifiers(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}


OUString DatabaseMetaData::getIdentifierQuoteString(  ) throw (SQLException, RuntimeException)
{
    return ASCII_STR( "\"" );
}

OUString DatabaseMetaData::getSQLKeywords(  ) throw (SQLException, RuntimeException)
{
    return ASCII_STR(
        "ANALYZE,"
        "ANALYSE,"
        "DO,"
        "ILIKE,"
        "LIMIT,"
        "NEW,"
        "OFFSET,"
        "OLD,"
        "PLACING" );
}
OUString DatabaseMetaData::getNumericFunctions(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return OUString();
}

OUString DatabaseMetaData::getStringFunctions(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return OUString();
}

OUString DatabaseMetaData::getSystemFunctions(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return OUString();
}
OUString DatabaseMetaData::getTimeDateFunctions(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return OUString();
}
OUString DatabaseMetaData::getSearchStringEscape(  ) throw (SQLException, RuntimeException)
{
    return ASCII_STR( "\\" );
}
OUString DatabaseMetaData::getExtraNameCharacters(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return OUString();
}

sal_Bool DatabaseMetaData::supportsAlterTableWithAddColumn(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsAlterTableWithDropColumn(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsColumnAliasing(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::nullPlusNonNullIsNull(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsTypeConversion(  ) throw (SQLException, RuntimeException)     // TODO, DON'T KNOW
{
    return sal_False;
}

sal_Bool DatabaseMetaData::supportsConvert( sal_Int32 fromType, sal_Int32 toType ) throw (SQLException, RuntimeException)  // TODO
{
    return sal_False;
}

sal_Bool DatabaseMetaData::supportsTableCorrelationNames(  ) throw (SQLException, RuntimeException)     // TODO, don't know
{
    return sal_True;
}


sal_Bool DatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw (SQLException, RuntimeException) // TODO, don't know
{

    return sal_True;
}
sal_Bool DatabaseMetaData::supportsExpressionsInOrderBy(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsOrderByUnrelated(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsGroupBy(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsGroupByUnrelated(  ) throw (SQLException, RuntimeException) // TODO, DONT know
{

    return sal_True;
}

sal_Bool DatabaseMetaData::supportsGroupByBeyondSelect(  ) throw (SQLException, RuntimeException) // TODO, DON'T know
{

    return sal_True;
}

sal_Bool DatabaseMetaData::supportsLikeEscapeClause(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsMultipleResultSets(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsMultipleTransactions(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::supportsNonNullableColumns(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}


sal_Bool DatabaseMetaData::supportsMinimumSQLGrammar(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsCoreSQLGrammar(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsExtendedSQLGrammar(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsANSI92FullSQL(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsOuterJoins(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsFullOuterJoins(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsLimitedOuterJoins(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}


OUString DatabaseMetaData::getSchemaTerm(  ) throw (SQLException, RuntimeException)
{
    return ASCII_STR( "SCHEMA" );
}

OUString DatabaseMetaData::getProcedureTerm(  ) throw (SQLException, RuntimeException)
{
    // don't know
    return OUString();
}

OUString DatabaseMetaData::getCatalogTerm(  ) throw (SQLException, RuntimeException)
{
    // TODO is this correct ?
    return ASCII_STR( "DATABASE" );
}

sal_Bool DatabaseMetaData::isCatalogAtStart(  ) throw (SQLException, RuntimeException)     // TODO don't know
{

    return sal_True;
}

OUString DatabaseMetaData::getCatalogSeparator(  ) throw (SQLException, RuntimeException)
{
    // TODO don't know
    return ASCII_STR( "." );
}

sal_Bool DatabaseMetaData::supportsSchemasInDataManipulation(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsSchemasInTableDefinitions(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsCatalogsInDataManipulation(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::supportsCatalogsInTableDefinitions(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}


sal_Bool DatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}


sal_Bool DatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}


sal_Bool DatabaseMetaData::supportsPositionedDelete(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsPositionedUpdate(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return sal_True;
}


sal_Bool DatabaseMetaData::supportsSelectForUpdate(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return sal_False;
}


sal_Bool DatabaseMetaData::supportsStoredProcedures(  ) throw (SQLException, RuntimeException)
{
    // TODO
    return sal_False;
}


sal_Bool DatabaseMetaData::supportsSubqueriesInComparisons(  ) throw (SQLException, RuntimeException)
{
    // TODO , don't know
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsSubqueriesInExists(  ) throw (SQLException, RuntimeException)
{
    // TODO , don't know
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsSubqueriesInIns(  ) throw (SQLException, RuntimeException)
{
    // TODO , don't know
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw (SQLException, RuntimeException)
{
    // TODO , don't know
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsCorrelatedSubqueries(  ) throw (SQLException, RuntimeException)
{
    // TODO , don't know
    return sal_True;
}
sal_Bool DatabaseMetaData::supportsUnion(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsUnionAll(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw (SQLException, RuntimeException)
{
    // TODO, don't know
    return sal_False;
}

sal_Bool DatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw (SQLException, RuntimeException)
{
    // TODO, don't know
    return sal_False;
}

sal_Bool DatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw (SQLException, RuntimeException)
{
    // TODO, don't know
    return sal_False;
}
sal_Bool DatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw (SQLException, RuntimeException)
{
    // TODO, don't know
    return sal_False;
}

sal_Int32 DatabaseMetaData::getMaxBinaryLiteralLength(  ) throw (SQLException, RuntimeException)
{
    // TODO, don't know
    return -1;
}

sal_Int32 DatabaseMetaData::getMaxCharLiteralLength(  ) throw (SQLException, RuntimeException)
{
    return -1;
}

sal_Int32 DatabaseMetaData::getMaxColumnNameLength(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_IDENTIFIER_LENGTH;
}

sal_Int32 DatabaseMetaData::getMaxColumnsInGroupBy(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_COLUMNS_IN_GROUPBY;
}

sal_Int32 DatabaseMetaData::getMaxColumnsInIndex(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_COLUMNS_IN_INDEX;
}

sal_Int32 DatabaseMetaData::getMaxColumnsInOrderBy(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_COLUMNS_IN_ORDER_BY;
}

sal_Int32 DatabaseMetaData::getMaxColumnsInSelect(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_COLUMNS_IN_SELECT;
}

sal_Int32 DatabaseMetaData::getMaxColumnsInTable(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_COLUMNS_IN_TABLE;
}

sal_Int32 DatabaseMetaData::getMaxConnections(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_CONNECTIONS;
}

sal_Int32 DatabaseMetaData::getMaxCursorNameLength(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_IDENTIFIER_LENGTH;
}

sal_Int32 DatabaseMetaData::getMaxIndexLength(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_IDENTIFIER_LENGTH;
}

sal_Int32 DatabaseMetaData::getMaxSchemaNameLength(  ) throw (SQLException, RuntimeException)
{
    return MAX_IDENTIFIER_LENGTH;
}

sal_Int32 DatabaseMetaData::getMaxProcedureNameLength(  ) throw (SQLException, RuntimeException)
{
    return MAX_IDENTIFIER_LENGTH;
}

sal_Int32 DatabaseMetaData::getMaxCatalogNameLength(  ) throw (SQLException, RuntimeException)
{
    return MAX_IDENTIFIER_LENGTH;
}

sal_Int32 DatabaseMetaData::getMaxRowSize(  ) throw (SQLException, RuntimeException)
{
    return -1;
}

sal_Bool DatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Int32 DatabaseMetaData::getMaxStatementLength(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_STATEMENT_LENGTH;
}

sal_Int32 DatabaseMetaData::getMaxStatements(  ) throw (SQLException, RuntimeException) //TODO, don't know
{
    return MAX_STATEMENTS;
}

sal_Int32 DatabaseMetaData::getMaxTableNameLength(  ) throw (SQLException, RuntimeException)
{
    return MAX_IDENTIFIER_LENGTH;
}

sal_Int32 DatabaseMetaData::getMaxTablesInSelect(  ) throw (SQLException, RuntimeException)
{
    return MAX_TABLES_IN_SELECT;
}

sal_Int32 DatabaseMetaData::getMaxUserNameLength(  ) throw (SQLException, RuntimeException)
{
    return MAX_USER_NAME_LENGTH;
}

sal_Int32 DatabaseMetaData::getDefaultTransactionIsolation(  ) throw (SQLException, RuntimeException)
{
    return com::sun::star::sdbc::TransactionIsolation::READ_COMMITTED;
}

sal_Bool DatabaseMetaData::supportsTransactions(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
    throw (SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool DatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw (SQLException, RuntimeException)
{
    // don't know
    return sal_True;
}

sal_Bool DatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw (SQLException, RuntimeException)
{
    return sal_True;
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getProcedures(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schemaPattern,
    const OUString& procedureNamePattern ) throw (SQLException, RuntimeException)
{
//        1.  PROCEDURE_CAT string =&gt; procedure catalog (may be NULL )
//        2. PROCEDURE_SCHEM string =&gt; procedure schema (may be NULL )
//        3. PROCEDURE_NAME string =&gt; procedure name
//        4. reserved for future use
//        5. reserved for future use
//        6. reserved for future use
//        7. REMARKS string =&gt; explanatory comment on the procedure
//        8. PROCEDURE_TYPE short =&gt; kind of procedure:
//               * UNKNOWN - May return a result
//               * NO - Does not return a result
//               * RETURN - Returns a result

    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return new SequenceResultSet(
        m_refMutex, *this, Sequence< OUString >(), Sequence< Sequence< Any > > (), m_pSettings->tc );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getProcedureColumns(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schemaPattern,
    const OUString& procedureNamePattern,
    const OUString& columnNamePattern ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return new SequenceResultSet(
        m_refMutex, *this, Sequence< OUString >(), Sequence< Sequence< Any > > (), m_pSettings->tc );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getTables(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schemaPattern,
    const OUString& tableNamePattern,
    const ::com::sun::star::uno::Sequence< OUString >& types )
    throw (SQLException, RuntimeException)
{
    Statics &statics = getStatics();

    MutexGuard guard( m_refMutex->mutex );
    checkClosed();

    if( isLog( m_pSettings, LogLevel::INFO ) )
    {
        rtl::OUStringBuffer buf( 128 );
        buf.appendAscii( "DatabaseMetaData::getTables got called with " );
        buf.append( schemaPattern );
        buf.appendAscii( "." );
        buf.append( tableNamePattern );
        log( m_pSettings, LogLevel::INFO, buf.makeStringAndClear() );
    }
    // ignore catalog, as a single pq connection does not support multiple catalogs

    Reference< XPreparedStatement > statement = m_origin->prepareStatement(
        ASCII_STR(
            "SELECT "
            "DISTINCT ON (pg_namespace.nspname, relname ) " // avoid duplicates (pg_settings !)
            "pg_namespace.nspname, relname, relkind, pg_description.description "
            "FROM pg_namespace, pg_class LEFT JOIN pg_description ON pg_class.oid = pg_description.objoid "
            "WHERE relnamespace = pg_namespace.oid "
            "AND ( relkind = 'r' OR relkind = 'v') "
            "AND pg_namespace.nspname LIKE ? "
            "AND relname LIKE ? "
//            "ORDER BY pg_namespace.nspname || relname"
            ) );

    Reference< XParameters > parameters( statement, UNO_QUERY );
    parameters->setString( 1 , schemaPattern );
    parameters->setString( 2 , tableNamePattern );

    Reference< XResultSet > rs = statement->executeQuery();
    Reference< XRow > xRow( rs, UNO_QUERY );
    SequenceAnyVector vec;

    while( rs->next() )
    {
        Sequence< Any > row( 5 );

        row[0] <<= m_pSettings->catalog;
        row[1] <<= xRow->getString( 1 );
        row[2] <<= xRow->getString( 2 );
        OUString type = xRow->getString(3);
        if( 0 == type.compareToAscii( "r" ) )
        {
            if( 0 == xRow->getString(1).compareToAscii( "pg_catalog" ) )
            {
                row[3] <<= statics.SYSTEM_TABLE;
            }
            else
            {
                row[3] <<= statics.TABLE;
            }
        }
        else if( 0 == type.compareToAscii( "v" ) )
        {
            row[3] <<= statics.VIEW;
        }
        else
        {
            row[3] <<= statics.UNKNOWN;
        }
        row[4] <<= xRow->getString(4);

        // no description in postgresql AFAIK
        vec.push_back( row );
    }
    Reference< XCloseable > closeable( statement, UNO_QUERY );
    if( closeable.is() )
        closeable->close();

    return new SequenceResultSet(
        m_refMutex, *this, statics.tablesRowNames,
        Sequence< Sequence< Any > > ( &vec[0],vec.size() ), m_pSettings->tc );
}

struct SortInternalSchemasLastAndPublicFirst
{
    bool operator () ( const Sequence< Any >  & a, const Sequence< Any >  & b )
    {
        OUString valueA;
        OUString valueB;
        a[0] >>= valueA;
        b[0] >>= valueB;
        bool ret = false;
        if( valueA.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "public" ) ) == 0 )
        {
            ret = true;
        }
        else if( valueB.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "public" ) ) == 0 )
        {
            ret = false;
        }
        else if( valueA.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "pg_" ) ) &&
            valueB.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "pg_" ) ) )
        {
            ret = valueA.compareTo( valueB ) < 0; // sorts equal !
        }
        else if( valueA.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "pg_" ) ))
        {
            ret = false; // sorts last !
        }
        else if( valueB.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "pg_" ) ) )
        {
            ret = true; // sorts dorst !

        }
        else
        {
            ret = (valueA.compareTo( valueB ) < 0);
        }
        return ret;
    }
};

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getSchemas(  )
    throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();

    if( isLog( m_pSettings, LogLevel::INFO ) )
    {
        log( m_pSettings, LogLevel::INFO, "DatabaseMetaData::getSchemas() got called" );
    }
    // <b>TABLE_SCHEM</b> string =&amp;gt; schema name
    Reference< XStatement > statement = m_origin->createStatement();
    Reference< XResultSet > rs = statement->executeQuery(
        ASCII_STR("SELECT nspname from pg_namespace") );

    Reference< XRow > xRow( rs, UNO_QUERY );
    SequenceAnyVector vec;
    while( rs->next() )
    {
        Sequence<Any> row(1);
        row[0] <<= xRow->getString(1);
        vec.push_back( row );
    }

    // sort public first, sort internal schemas last, sort rest in alphabetic order
    std::sort( vec.begin(), vec.end(), SortInternalSchemasLastAndPublicFirst() );

    Reference< XCloseable > closeable( statement, UNO_QUERY );
    if( closeable.is() )
        closeable->close();
    return new SequenceResultSet(
        m_refMutex, *this, getStatics().schemaNames,
        Sequence< Sequence< Any > > ( &vec[0], vec.size() ), m_pSettings->tc );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getCatalogs(  )
    throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return new SequenceResultSet(
        m_refMutex, *this, Sequence< OUString >(), Sequence< Sequence< Any > > (), m_pSettings->tc );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getTableTypes(  )
    throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return new SequenceResultSet(
        m_refMutex, *this, getStatics().tableTypeNames, getStatics().tableTypeData,
        m_pSettings->tc );
}


/** returns the constant from sdbc.DataType
 */
sal_Int32 typeNameToDataType( const OUString &typeName, const OUString &typtype )
{
//     sal_Int32 ret = com::sun::star::sdbc::DataType::DISTINCT;
    // map all unknown types to memo (longvarchar). This allows to show them in
    // string representation. Additionally, the edit-table-type-selection-box
    // is not so unuseable anymore.
    sal_Int32 ret = com::sun::star::sdbc::DataType::LONGVARCHAR;
    if( 0 == typtype.compareToAscii( "b" ) )
    {
        // as long as the OOo framework does not support arrays,
        // the user is better of with interpreting arrays as strings !
//         if( typeName.getLength() && '_' == typeName[0] )
//         {
//             its just a naming convention, but as long as we don't have anything better,
//             we take it as granted
//             ret = com::sun::star::sdbc::DataType::ARRAY;
//         }
        // base type
        Statics &statics = getStatics();
        BaseTypeMap::iterator ii = statics.baseTypeMap.find( typeName );
        if( ii != statics.baseTypeMap.end() )
        {
            ret = ii->second;
        }
    }
    else if( 0 == typtype.compareToAscii( "c" ) )
    {
        ret = com::sun::star::sdbc::DataType::STRUCT;
    }
    else if( 0 == typtype.compareToAscii( "d" ) )
    {
        ret = com::sun::star::sdbc::DataType::LONGVARCHAR;
    }
    return ret;
}

static bool isSystemColumn( const OUString &columnName )
{
    return
        columnName.compareToAscii( "oid" ) == 0 ||
        columnName.compareToAscii( "tableoid" ) == 0 ||
        columnName.compareToAscii( "xmin" ) == 0 ||
        columnName.compareToAscii( "cmin" ) == 0 ||
        columnName.compareToAscii( "xmax" ) == 0 ||
        columnName.compareToAscii( "cmax" ) == 0 ||
        columnName.compareToAscii( "ctid" ) == 0;
}

// is not exported by the postgres header
const static int PQ_VARHDRSZ = sizeof( sal_Int32 );

static void extractPrecisionAndScale(
    sal_Int32 dataType, sal_Int32 atttypmod, sal_Int32 *precision, sal_Int32 *scale )
{
    if( atttypmod < PQ_VARHDRSZ )
    {
        *precision = 0;
        *scale = 0;
    }
    else
    {
        switch( dataType )
        {
        case com::sun::star::sdbc::DataType::NUMERIC:
        case com::sun::star::sdbc::DataType::DECIMAL:
        {
            *precision = ( ( atttypmod - PQ_VARHDRSZ ) >> 16 ) & 0xffff;
            *scale = (atttypmod - PQ_VARHDRSZ ) & 0xffff;
            break;
        }
        default:
            *precision = atttypmod - PQ_VARHDRSZ;
            *scale = 0;
        }
    }
}

struct DatabaseTypeDescription
{
    DatabaseTypeDescription()
    {}
    DatabaseTypeDescription( const OUString &name, const OUString & type ) :
        typeName( name ),
        typeType( type )
    {}
    DatabaseTypeDescription( const DatabaseTypeDescription &source ) :
        typeName( source.typeName ),
        typeType( source.typeType )
    {}
    DatabaseTypeDescription & operator = ( const DatabaseTypeDescription & source )
    {
        typeName = source.typeName;
        typeType = source.typeType;
        return *this;
    }
    OUString typeName;
    OUString typeType;
};

typedef std::unordered_map
<
    sal_Int32,
    DatabaseTypeDescription,
    ::std::hash< sal_Int32 >,
    ::std::equal_to< sal_Int32 >,
    Allocator< ::std::pair< sal_Int32, DatabaseTypeDescription > >
> Oid2DatabaseTypeDescriptionMap;

static void columnMetaData2DatabaseTypeDescription(
    Oid2DatabaseTypeDescriptionMap &oidMap,
    const Reference< XResultSet > &rs,
    const Reference< XStatement > &stmt )
{
    Reference< XRow > row( rs, UNO_QUERY );
    int domains = 0;
    rtl::OUStringBuffer queryBuf(128);
    queryBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "SELECT oid,typtype,typname FROM pg_TYPE WHERE " ) );
    while( rs->next() )
    {
        if( row->getString( 9 ).equalsAscii( "d" ) && oidMap.find( row->getInt( 12 ) ) == oidMap.end() )
        {
            oidMap[row->getInt(12)] = DatabaseTypeDescription();
            if( domains )
                queryBuf.appendAscii( " OR " );
            queryBuf.appendAscii( "oid = " );
            queryBuf.append( row->getInt(12 ), 10 );
            domains ++;
        }
    }
    rs->beforeFirst();

    if( domains )
    {
        Reference< XResultSet > rsDomain = stmt->executeQuery( queryBuf.makeStringAndClear() );
        row = Reference< XRow >( rsDomain, UNO_QUERY );
        while( rsDomain->next() )
        {
            oidMap[row->getInt(1)] = DatabaseTypeDescription(row->getString(3), row->getString(2) );
        }
        disposeNoThrow( stmt );
    }

}



::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getColumns(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schemaPattern,
    const OUString& tableNamePattern,
    const OUString& columnNamePattern ) throw (SQLException, RuntimeException)
{
    Statics &statics = getStatics();

    // continue !
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();

    if( isLog( m_pSettings, LogLevel::INFO ) )
    {
        rtl::OUStringBuffer buf( 128 );
        buf.appendAscii( "DatabaseMetaData::getColumns got called with " );
        buf.append( schemaPattern );
        buf.appendAscii( "." );
        buf.append( tableNamePattern );
        buf.appendAscii( "." );
        buf.append( columnNamePattern );
        log( m_pSettings, LogLevel::INFO, buf.makeStringAndClear() );
    }

    // ignore catalog, as a single pq connection
    // does not support multiple catalogs eitherway

    //  1. TABLE_CAT string => table catalog (may be NULL)
    //               => not supported
    //  2. TABLE_SCHEM string => table schema (may be NULL)
    //               => pg_namespace.nspname
    //  3. TABLE_NAME string => table name
    //               => pg_class.relname
    //  4. COLUMN_NAME string => column name
    //               => pg_attribure.attname
    //  5. DATA_TYPE short => SQL type from java.sql.Types
    //               => pg_type.typname => sdbc.DataType
    //  6. TYPE_NAME string => Data source dependent type name, for a UDT the
    //                         type name is fully qualified
    //               => pg_type.typname
    //  7. COLUMN_SIZE long => column size. For char or date types this is
    //                         the maximum number of characters, for numeric
    //                         or decimal types this is precision.
    //               => pg_attribute.atttypmod
    //  8. BUFFER_LENGTH is not used.
    //               => not used
    //  9. DECIMAL_DIGITS long => the number of fractional digits
    //               => don't know ! TODO !
    //  10. NUM_PREC_RADIX long => Radix (typically either 10 or 2)
    //               => TODO ??
    //  11. NULLABLE long => is NULL allowed?
    //                      NO_NULLS - might not allow NULL values
    //                      NULABLE - definitely allows NULL values
    //                      NULLABLE_UNKNOWN - nullability unknown
    //               => pg_attribute.attnotnull
    //  12. REMARKS string => comment describing column (may be NULL )
    //               => Don't know, there does not seem to exist something like
    //                  that in postgres
    //  13. COLUMN_DEF string => default value (may be NULL)
    //               => pg_type.typdefault
    //  14. SQL_DATA_TYPE long => unused
    //               => empty
    //  15. SQL_DATETIME_SUB long => unused
    //               => empty
    //  16. CHAR_OCTET_LENGTH long => for char types the maximum number of
    //                                bytes in the column
    //               => pg_type.typlen
    //  17. ORDINAL_POSITION int => index of column in table (starting at 1)
    //                              pg_attribute.attnum
    //  18. IS_NULLABLE string => "NO" means column definitely does not allow
    //                            NULL values; "YES" means the column might
    //                            allow NULL values. An empty string means
    //                            nobody knows.
    //               => pg_attribute.attnotnull
// select objoid,description,objsubid,pg_attribute.attname from pg_attribute LEFT JOIN pg_description ON pg_attribute.attrelid=pg_description.objoid and pg_attribute.attnum = pg_description.objsubid

    Reference< XPreparedStatement > statement = m_origin->prepareStatement(
        ASCII_STR(

            "SELECT pg_namespace.nspname, "  // 1
            "pg_class.relname, "             // 2
            "pg_attribute.attname, "         // 3
            "pg_type.typname, "              // 4
            "pg_attribute.atttypmod, "       // 5
            "pg_attribute.attnotnull, "      // 6
            "pg_type.typdefault, "           // 7
            "pg_attribute.attnum, "          // 8
            "pg_type.typtype, "              // 9
            "pg_attrdef.adsrc, "             // 10
            "pg_description.description, "    // 11
            "pg_type.typbasetype "           // 12
            "FROM pg_class, "
                 "pg_attribute LEFT JOIN pg_attrdef ON pg_attribute.attrelid = pg_attrdef.adrelid AND pg_attribute.attnum = pg_attrdef.adnum "
                              "LEFT JOIN pg_description ON pg_attribute.attrelid = pg_description.objoid AND pg_attribute.attnum=pg_description.objsubid,"
                 " pg_type, pg_namespace "
            "WHERE pg_attribute.attrelid = pg_class.oid "
                   "AND pg_attribute.atttypid = pg_type.oid "
                   "AND pg_class.relnamespace = pg_namespace.oid "
                   "AND pg_namespace.nspname LIKE ? "
                   "AND pg_class.relname LIKE ? "
                   "AND pg_attribute.attname LIKE ? "
            "ORDER BY pg_namespace.nspname || pg_class.relname || pg_attribute.attnum"
            ) );

    Reference< XParameters > parameters( statement, UNO_QUERY );
    parameters->setString( 1 , schemaPattern );
    parameters->setString( 2 , tableNamePattern );
    parameters->setString( 3 , columnNamePattern );

    Reference< XResultSet > rs = statement->executeQuery();
    Reference< XRow > xRow( rs, UNO_QUERY );
    SequenceAnyVector vec;

    Oid2DatabaseTypeDescriptionMap domainMap;
    Reference< XStatement > domainTypeStmt = m_origin->createStatement();
    columnMetaData2DatabaseTypeDescription( domainMap, rs, domainTypeStmt );

    while( rs->next() )
    {
        OUString columnName = xRow->getString(3);
        if( m_pSettings->showSystemColumns || ! isSystemColumn( columnName ) )
        {
            sal_Int32 precision, scale, type;
            Sequence< Any > row( 18 );
            row[0] <<= m_pSettings->catalog;
            row[1] <<= xRow->getString(1);  //
            row[2] <<= xRow->getString(2);
            row[3] <<= columnName;
            if( xRow->getString(9).equalsAscii( "d" ) )
            {
                DatabaseTypeDescription desc( domainMap[xRow->getInt(12)] );
                type = typeNameToDataType( desc.typeName, desc.typeType );
            }
            else
            {
                type = typeNameToDataType( xRow->getString(4), xRow->getString(9) );
            }
            extractPrecisionAndScale( type, xRow->getInt(5) , &precision, &scale );
            row[4] <<= type;
            row[5] <<= xRow->getString(4);
            row[6] <<= precision;
            row[8] <<= scale;
            if( xRow->getBoolean( 6 ) && ! isSystemColumn(xRow->getString(3)) )
            {
                row[10] <<= OUString::valueOf(com::sun::star::sdbc::ColumnValue::NO_NULLS);
                row[17] <<= statics.NO;
            }
            else
            {
                row[10] <<= OUString::valueOf(com::sun::star::sdbc::ColumnValue::NULLABLE);
                row[17] <<= statics.YES;
            }

            row[11] <<= xRow->getString( 11 ); // comment
            row[12] <<= xRow->getString(10); // COLUMN_DEF = pg_type.typdefault
            row[15] <<= precision;
            row[16] <<= xRow->getString(8) ;

            // no description in postgresql AFAIK
            vec.push_back( row );
        }
    }
    Reference< XCloseable > closeable( statement, UNO_QUERY );
    if( closeable.is() )
        closeable->close();

    return new SequenceResultSet(
        m_refMutex, *this, statics.columnRowNames,
        Sequence< Sequence< Any > > ( &vec[0],vec.size() ), m_pSettings->tc );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getColumnPrivileges(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schema,
    const OUString& table,
    const OUString& columnNamePattern ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return new SequenceResultSet(
        m_refMutex, *this, Sequence< OUString >(), Sequence< Sequence< Any > > (), m_pSettings->tc );
}

static void addPrivilegesToVector(
    sal_Int32 privilege, const OUString &catalog, const OUString & schema,
    const OUString &tableName, const OUString &grantor, const OUString &grantee,
    bool grantable, SequenceAnyVector &vec )
{
    Statics & statics = getStatics();
    for( int index = 1; index <= PRIVILEGE_MAX ; index = index << 1 )
    {
        OUString privname;
        switch( privilege & index )
        {
        case PRIVILEGE_SELECT:
            privname = statics.SELECT; break;
        case PRIVILEGE_UPDATE:
            privname = statics.UPDATE; break;
        case PRIVILEGE_INSERT:
            privname = statics.INSERT; break;
        case PRIVILEGE_DELETE:
            privname = statics.DELETE; break;
        case PRIVILEGE_RULE:
            privname = statics.RULE; break;
        case PRIVILEGE_REFERENCES:
            privname = statics.REFERENCES; break;
        case PRIVILEGE_TRIGGER:
            privname = statics.TRIGGER; break;
        case PRIVILEGE_EXECUTE:
            privname = statics.EXECUTE; break;
        case PRIVILEGE_USAGE:
            privname = statics.USAGE; break;
        case PRIVILEGE_CREATE:
            privname = statics.CREATE; break;
        case PRIVILEGE_TEMPORARY:
            privname = statics.TEMPORARY; break;
        default:
            break;
        }

        Sequence< Any > seq( 7 );
        seq[0] <<= catalog;
        seq[1] <<= schema;
        seq[2] <<= tableName;
        seq[3] <<= grantor;
        seq[4] <<= grantee;
        seq[5] <<= privname;
        seq[6] <<= (grantable ? statics.YES : statics.NO );
        vec.push_back( seq );
    }
}


::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getTablePrivileges(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schemaPattern,
    const OUString& tableNamePattern ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();

    if( isLog( m_pSettings, LogLevel::INFO ) )
    {
        rtl::OUStringBuffer buf( 128 );
        buf.appendAscii( "DatabaseMetaData::getTablePrivileges got called with " );
        buf.append( schemaPattern );
        buf.appendAscii( "." );
        buf.append( tableNamePattern );
        log( m_pSettings, LogLevel::INFO, buf.makeStringAndClear() );
    }

    // algorithm

    // get the pg_class.relact item for the concrete table
    // get userid for username from pg_shadow (or pg_user view)
    // get the group names mentioned in pg_class.relact from pg_group
    // identify, which groups the current user belongs to
    // calculate the union of all permissions (1 public, 1 user, n groups)

    //     1. TABLE_CAT string =&gt; table catalog (may be NULL )
    //     2. TABLE_SCHEM string =&gt; table schema (may be NULL )
    //     3. TABLE_NAME string =&gt; table name
    //     4. GRANTOR =&gt; grantor of access (may be NULL )
    //     5. GRANTEE string =&gt; grantee of access
    //     6. PRIVILEGE string =&gt; name of access (SELECT, INSERT, UPDATE, REFERENCES, ...)
    //     7. IS_GRANTABLE string =&gt; "YES" if grantee is permitted to grant to
    //        others; "NO" if not; NULL if unknown

    Reference< XPreparedStatement > statement = m_origin->prepareStatement(
        ASCII_STR(
            "SELECT pg_namespace.nspname, "
                    "pg_class.relname, "
                    "pg_class.relacl, "
                    "pg_user.usename "
            "FROM pg_class, pg_user, pg_namespace "
            "WHERE pg_class.relowner = pg_user.usesysid "
                   "AND ( pg_class.relkind = 'r' OR pg_class.relkind = 'v' ) "
                   "AND pg_class.relnamespace = pg_namespace.oid "
                   "AND pg_namespace.nspname LIKE ? "
                   "AND pg_class.relname LIKE ?"
            "ORDER BY pg_namespace.nspname || pg_class.relname "
            ) );

    Reference< XParameters > parameters( statement, UNO_QUERY );
    parameters->setString( 1 , schemaPattern );
    parameters->setString( 2 , tableNamePattern );

    Reference< XResultSet > rs = statement->executeQuery();
    Reference< XRow > xRow( rs, UNO_QUERY );
    SequenceAnyVector vec;
    while( rs->next() )
    {
        // TODO calculate privileges  !
        sal_Int32 privilege = 0;
        privilege =
            PRIVILEGE_SELECT | PRIVILEGE_UPDATE | PRIVILEGE_INSERT |
            PRIVILEGE_DELETE | PRIVILEGE_RULE   | PRIVILEGE_REFERENCES |
            PRIVILEGE_TRIGGER| PRIVILEGE_EXECUTE| PRIVILEGE_USAGE      |
            PRIVILEGE_CREATE |PRIVILEGE_TEMPORARY;

        addPrivilegesToVector( privilege,
                               m_pSettings->catalog,
                               xRow->getString( 1 ),
                               xRow->getString( 2 ),
                               xRow->getString( 4 ),
                               m_pSettings->user,
                               m_pSettings->user == xRow->getString( 4 ),
                               vec );
    }

    return new SequenceResultSet(
        m_refMutex, *this, getStatics().tablePrivilegesNames,
        Sequence< Sequence< Any > > ( &vec[0], vec.size() ), m_pSettings->tc );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getBestRowIdentifier(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schema,
    const OUString& table,
    sal_Int32 scope,
    sal_Bool nullable ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return new SequenceResultSet(
        m_refMutex, *this, Sequence< OUString >(), Sequence< Sequence< Any > > (), m_pSettings->tc );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getVersionColumns(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schema,
    const OUString& table ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return new SequenceResultSet(
        m_refMutex, *this, Sequence< OUString >(), Sequence< Sequence< Any > > (), m_pSettings->tc );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getPrimaryKeys(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schema,
    const OUString& table ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();

//        1.  TABLE_CAT string =&gt; table catalog (may be NULL )
//        2. TABLE_SCHEM string =&gt; table schema (may be NULL )
//        3. TABLE_NAME string =&gt; table name
//        4. COLUMN_NAME string =&gt; column name
//        5. KEY_SEQ short =&gt; sequence number within primary key
//        6. PK_NAME string =&gt; primary key name (may be NULL )

    if( isLog( m_pSettings, LogLevel::INFO ) )
    {
        rtl::OUStringBuffer buf( 128 );
        buf.appendAscii( "DatabaseMetaData::getPrimaryKeys got called with " );
        buf.append( schema );
        buf.appendAscii( "." );
        buf.append( table );
        log( m_pSettings, LogLevel::INFO, buf.makeStringAndClear() );
    }

    Reference< XPreparedStatement > statement = m_origin->prepareStatement(
        ASCII_STR(
            "SELECT nmsp.nspname, "
                    "cl.relname, "
                    "con.conkey, "
                    "con.conname, "
                    "con.conrelid "
            "FROM pg_constraint as con,pg_class as cl, pg_namespace as nmsp "
            "WHERE con.connamespace = nmsp.oid AND con.conrelid = cl.oid AND con.contype = 'p' "
                "AND nmsp.nspname LIKE ? AND cl.relname LIKE ?" ) );

    Reference< XParameters > parameters( statement, UNO_QUERY );
    parameters->setString( 1 , schema );
    parameters->setString( 2 , table );

    Reference< XResultSet > rs = statement->executeQuery();
    Reference< XRow > xRow( rs, UNO_QUERY );
    SequenceAnyVector vec;

    while( rs->next() )
    {
        Sequence< Any > row( 6 );
        row[0] <<= m_pSettings->catalog;
        row[1] <<= xRow->getString(1);  //
        row[2] <<= xRow->getString(2);
        OUString array = xRow->getString(3);
        row[4] <<= xRow->getString(5); // the relid
        row[5] <<= xRow->getString(4);

        int i = 0;
        // now retrieve the columns information
        // unfortunately, postgresql does not allow array of variable size in
        // WHERE clauses (in the default installation), so we have to choose
        // this expensive and somewhat ugly way
        // annotation: postgresql shouldn't have choosen an array here, instead they
        //             should have multiple rows per table
        while( array[i] && '}' != array[i] )
        {
            i++;
            int start = i;
            while( array[i] && array[i] != '}' && array[i] != ',' ) i++;
            row[3] <<= OUString( &array[start], i - start );
            vec.push_back( row );
        }
    }

    {
        Reference< XCloseable > closeable( statement, UNO_QUERY );
        if( closeable.is() )
            closeable->close();
    }


    SequenceAnyVector::iterator ii = vec.begin();
    OUString lastTableOid;
    sal_Int32 index;
    Sequence< Sequence< Any > > ret( vec.size() );
    int elements = 0;
    for( ; ii != vec.end() ; ++ ii )
    {

        Sequence< Any > row = *ii;
        OUString tableOid;
        OUString attnum;

        row[4] >>= tableOid;
        row[3] >>= attnum;
        statement = m_origin->prepareStatement(
            ASCII_STR(
                "SELECT att.attname FROM "
                "pg_attribute AS att, pg_class AS cl WHERE "
                "att.attrelid = ? AND att.attnum = ?" ));

        parameters = Reference< XParameters >( statement, UNO_QUERY );
        parameters->setString( 1 , tableOid );
        parameters->setString( 2 , attnum );

        rs = statement->executeQuery();
        xRow = Reference< XRow >( rs, UNO_QUERY );
        if( rs->next() )
        {
            // column name
            row[3] <<= xRow->getString( 1 );
            if( tableOid != lastTableOid )
                index = 1;
            lastTableOid = tableOid;
            row[4] <<= OUString::valueOf( index );
            index ++;
        }
        {
            Reference< XCloseable > closeable( statement, UNO_QUERY );
            if( closeable.is() )
                closeable->close();
        }
        ret[elements] = row;
        elements ++;
    }
    return new SequenceResultSet(
        m_refMutex, *this, getStatics().primaryKeyNames, ret , m_pSettings->tc );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getImportedKeys(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schema,
    const OUString& table ) throw (SQLException, RuntimeException)
{
//     MutexGuard guard( m_refMutex->mutex );
//     checkClosed();

//     Statics &st = getStatics();
//     Int2StringMap mainMap;
//     fillAttnum2attnameMap( mainMap, m_origin, m_schemaName, m_tableName );

//     Reference< XPreparedStatement > stmt = m_origin->prepareStatement(
//         ASCII_STR(
//             "SELECT  conname, "            // 1
//                     "confupdtype, "        // 2
//                     "confdeltype, "        // 3
//                     "class2.relname, "     // 4
//                     "nmsp2.nspname, "      // 5
//                     "conkey,"              // 6
//                     "confkey "             // 7
//             "FROM pg_constraint INNER JOIN pg_class ON conrelid = pg_class.oid "
//                  "INNER JOIN pg_namespace ON pg_class.relnamespace = pg_namespace.oid "
//                  "LEFT JOIN pg_class AS class2 ON confrelid = class2.oid "
//                  "LEFT JOIN pg_namespace AS nmsp2 ON class2.relnamespace=nmsp2.oid "
//             "WHERE pg_class.relname = ? AND "
//                   "pg_namespace.nspname = ? "
//                   "AND contype = 'f'"));

//     Reference< XResultSet > rs = stmt->executeQuery();
//     Reference< XRow > row( rs, UNO_QUERY );
//     while( rs->next() )
//     {

//         static const PKTABLE_CAT = 0;
//         static const PKTABLE_SCHEM = 1;
//         static const PKTABLE_NAME = 2;
//         static const PKCOLUMN_NAME = 3;
//         static const FKTABLE_CAT = 4;
//         static const FKTABLE_SCHEM = 5;
//         static const FKTABLE_NAME = 6;
//         static const FKCOLUMN_NAME = 7;
//         static const KEY_SEQ = 8;
//         static const UPDATE_RULE = 9;
//         static const DELETE_RULE = 10;
//         static const FK_NAME = 11;
//         static const PK_NAME = 12;
//         static const DEFERRABILITY = 13;

//         OUString pkSchema =  xRow->getString(6);
//         OUString pkTable = xRow->getString(5);

//         Int2StringMap foreignMap;
//         fillAttnum2attnameMap( foreignMap, m_origin,pkSchema, pkTable);

//         Sequence< rtl::OUString > pkColNames =
//             convertMappedIntArray2StringArray(
//                 foreignMap, string2intarray( row->getString( 7 ) ) );
//         Sequence< rtl::OUString > fkColNames =
//             convertMappedIntArray2StringArray(
//                 mainMap, string2intarray( row->getString( 6 ) ) );

//         for( sal_Int32 i = 0 ; i < pkColNames.getLength() ; i ++ )
//         {
//             Sequence< Any > theRow( 14 );

//             theRow[PKTABLE_SCHEM] = makeAny( pkSchema );
//             theRow[PKTABLE_NAME] = makeAny( pkTable );
//             theRow[PKCOLUMN_NAME] = makeAny( pkColNames[i] );
//             theRow[FKTABLE_SCHEM] = makeAny( schema );
//             theRow[FKTABLE_NAME] = makeAny( table );
//             theRow[FKCOLUMN_NAME] = makeAny( fkColNames[i] );
//             theRow[KEY_SEQ] = makeAny( OUString::valueOf( i ) );
//             theRow[


//         pKey->setPropertyValue_NoBroadcast_public(
//             st.PRIVATE_FOREIGN_COLUMNS,
//             makeAny( resolveColumnNames(foreignMap, xRow->getString(8) ) ) );

//     }
    // fake the getImportedKey() function call in
    // dbaccess/source/ui/relationdesigin/RelationController.cxx
    // it seems to be the only place in the office, where this function is needed
    return new SequenceResultSet(
        m_refMutex, *this, Sequence< OUString >(), Sequence< Sequence< Any > > (1), m_pSettings->tc );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getExportedKeys(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schema,
    const OUString& table ) throw (SQLException, RuntimeException)
{
    throw ::com::sun::star::sdbc::SQLException(
        ASCII_STR( "pq_databasemetadata: imported keys from tables not supported " ),
        *this,
        OUString(), 1, Any() );
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getCrossReference(
    const ::com::sun::star::uno::Any& primaryCatalog,
    const OUString& primarySchema,
    const OUString& primaryTable,
    const ::com::sun::star::uno::Any& foreignCatalog,
    const OUString& foreignSchema,
    const OUString& foreignTable ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return new SequenceResultSet(
        m_refMutex, *this, Sequence< OUString >(), Sequence< Sequence< Any > > (), m_pSettings->tc );
}


struct TypeInfoByDataTypeSorter
{
    bool operator () ( const Sequence< Any > & a, const Sequence< Any > & b )
    {
        OUString valueA;
        OUString valueB;
        a[1 /*DATA_TYPE*/] >>= valueA;
        b[1 /*DATA_TYPE*/] >>= valueB;
        if( valueB.toInt32() == valueA.toInt32() )
        {
            OUString nameA;
            OUString nameB;
            a[0 /*TYPE_NAME*/] >>= nameA;
            b[0 /*TYPE_NAME*/] >>= nameB;
            if( nameA.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "int4" ) ) == 0 )
                return 1;
            if( nameB.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "int4" ) ) == 0 )
                return 0;
            return nameA.compareTo( nameB ) < 0;
        }

        return valueA.toInt32() < valueB.toInt32();
//         sal_Int32 valueA;
//         sal_Int32 valueB;
//         a[1 /*DATA_TYPE*/] >>= valueA;
//         b[1 /*DATA_TYPE*/] >>= valueB;
//         if( valueB == valueA )
//         {
//             OUString nameA;
//             OUString nameB;
//             a[0 /*TYPE_NAME*/] >>= nameA;
//             b[0 /*TYPE_NAME*/] >>= nameB;
//             return nameA.compareTo( nameB ) < 0;
//         }

//         return valueA < valueB;
    }
};

static sal_Int32 calcSearchable( sal_Int32 dataType )
{
    sal_Int32 ret = com::sun::star::sdbc::ColumnSearch::FULL;
    if( com::sun::star::sdbc::DataType::BINARY == dataType ||
        com::sun::star::sdbc::DataType::VARBINARY == dataType ||
        com::sun::star::sdbc::DataType::LONGVARBINARY == dataType )
        ret = com::sun::star::sdbc::ColumnSearch::NONE;

    return ret;
}

static sal_Int32 getMaxScale( sal_Int32 dataType )
{
    sal_Int32 ret = 0;
    if( dataType == com::sun::star::sdbc::DataType::NUMERIC )
        ret = 1000; // see pg-docs DataType/numeric
//     else if( dataType == DataType::DOUBLE )
//         ret = 308;
//     else if( dataType == DataType::FLOAT )
//         ret =
    return ret;
}


struct RawType
{
    const char * typeName;
    const char * createParam;
    sal_Int32    sdbcType;
    sal_Int32    precision;
    sal_Int32    nullable;
    bool         caseSensitive;
    sal_Int32    searchable;
};

static void pgTypeInfo2ResultSet(
     SequenceAnyVector &vec,
     const Reference< XResultSet > &rs )
{
    static const sal_Int32 TYPE_NAME = 0;  // string Type name
    static const sal_Int32 DATA_TYPE = 1;  // short SQL data type from java.sql.Types
    static const sal_Int32 PRECISION = 2;  // long maximum precision
    static const sal_Int32 CREATE_PARAMS = 5; // string => parameters used in creating the type (may be NULL )
    static const sal_Int32 NULLABLE  = 6;  // short ==> can you use NULL for this type?
                                           // - NO_NULLS - does not allow NULL values
                                           // - NULLABLE - allows NULL values
                                           // - NULLABLE_UNKNOWN - nullability unknown

    static const sal_Int32 CASE_SENSITIVE = 7; // boolean==> is it case sensitive
    static const sal_Int32 SEARCHABLE = 8;  // short ==>; can you use
                                            // "WHERE" based on this type:
                                            //   - NONE - No support
                                            //   - CHAR - Only supported with WHERE .. LIKE
                                            //   - BASIC - Supported except for WHERE .. LIKE
                                            //   - FULL - Supported for all WHERE ..
    static const sal_Int32 UNSIGNED_ATTRIBUTE = 9; // boolean ==> is it unsigned?
    static const sal_Int32 FIXED_PREC_SCALE = 10; // boolean ==> can it be a money value?
    static const sal_Int32 AUTO_INCREMENT = 11; // boolean ==> can it be used for
                                                // an auto-increment value?
    static const sal_Int32 MINIMUM_SCALE = 13; // short ==> minimum scale supported
    static const sal_Int32 MAXIMUM_SCALE = 14; // short ==> maximum scale supported
    static const sal_Int32 NUM_PREC_RADIX = 17; // long ==> usually 2 or 10

    /*  not filled so far
        3. LITERAL_PREFIX string ==> prefix used to quote a literal
                                     (may be <NULL/>)
        4, LITERAL_SUFFIX string ==> suffix used to quote a literal
                                    (may be <NULL/>)
        5. CREATE_PARAMS string ==> parameters used in creating thw type (may be <NULL/>)
        12. LOCAL_TYPE_NAME  string ==> localized version of type name (may be <NULL/>)
        15, SQL_DATA_TYPE long ==> unused
        16. SQL_DATETIME_SUB long ==> unused
     */
    Reference< XRow > xRow( rs, UNO_QUERY );
    while( rs->next() )
    {
        Sequence< Any > row(18);

        sal_Int32 dataType =typeNameToDataType(xRow->getString(5),xRow->getString(2));
        sal_Int32 precision = xRow->getString(3).toInt32();

        if( dataType == com::sun::star::sdbc::DataType::CHAR  ||
            ( dataType == com::sun::star::sdbc::DataType::VARCHAR &&
              xRow->getString(TYPE_NAME+1).equalsIgnoreAsciiCaseAscii( "varchar") ) )
        {
            // reflect varchar as varchar with upper limit !
            //NOTE: the sql spec requires varchar to have an upper limit, however
            //      in postgresql the upper limit is optional, no limit means unlimited
            //      length (=1GB).
            precision = 0x40000000; // about 1 GB, see character type docs in postgresql
            row[CREATE_PARAMS] <<= ASCII_STR( "length" );
        }
        else if( dataType == com::sun::star::sdbc::DataType::NUMERIC )
        {
            precision = 1000;
            row[CREATE_PARAMS] <<= ASCII_STR( "length, scale" );
        }

        row[TYPE_NAME] <<= xRow->getString(1);
        row[DATA_TYPE] <<= OUString::valueOf(dataType);
        row[PRECISION] <<= OUString::valueOf( precision );
        sal_Int32 nullable = xRow->getBoolean(4) ?
            com::sun::star::sdbc::ColumnValue::NO_NULLS :
            com::sun::star::sdbc::ColumnValue::NULLABLE;
        row[NULLABLE] <<= OUString::valueOf(nullable);
        row[CASE_SENSITIVE] <<= OUString::valueOf((sal_Int32)1);
        row[SEARCHABLE] <<= OUString::valueOf( calcSearchable( dataType ) );
        row[UNSIGNED_ATTRIBUTE] <<= ASCII_STR( "0" ); //
        if( com::sun::star::sdbc::DataType::INTEGER == dataType ||
            com::sun::star::sdbc::DataType::BIGINT == dataType )
            row[AUTO_INCREMENT] <<= ASCII_STR( "1" );     // TODO
        else
            row[AUTO_INCREMENT] <<= ASCII_STR( "0" );     // TODO
        row[MINIMUM_SCALE] <<= ASCII_STR( "0" );      // TODO: what is this ?
        row[MAXIMUM_SCALE] <<= OUString::valueOf( getMaxScale( dataType ) );
        row[NUM_PREC_RADIX] <<= ASCII_STR( "10" );    // TODO: what is this ?
        vec.push_back( row );
    }

}


::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getTypeInfo(  )
    throw (SQLException, RuntimeException)
{
    // Note: Indexes start at 0 (in the API doc, they start at 1)
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();

    if( isLog( m_pSettings, LogLevel::INFO ) )
    {
        log( m_pSettings, LogLevel::INFO, "DatabaseMetaData::getTypeInfo() got called" );
    }

    Reference< XStatement > statement = m_origin->createStatement();
    Reference< XResultSet > rs = statement->executeQuery(
        ASCII_STR(
          "SELECT pg_type.typname AS typname," //1
          "pg_type.typtype AS typtype,"        //2
          "pg_type.typlen AS typlen,"          //3
          "pg_type.typnotnull AS typnotnull,"  //4
          "pg_type.typname AS typname "        //5
          "FROM pg_type "
          "WHERE pg_type.typtype = 'b' "
          "OR pg_type.typtype = 'p'"
            ) );

    SequenceAnyVector vec;
    pgTypeInfo2ResultSet( vec, rs );

    // check for domain types
    rs = statement->executeQuery(
     ASCII_STR(
        "SELECT t1.typname as typname,"
        "t2.typtype AS typtype,"
        "t2.typlen AS typlen,"
        "t2.typnotnull AS typnotnull,"
        "t2.typname as realtypname "
        "FROM pg_type as t1 LEFT JOIN pg_type AS t2 ON t1.typbasetype=t2.oid "
        "WHERE t1.typtype = 'd'" ) );
    pgTypeInfo2ResultSet( vec, rs );

    std::sort( vec.begin(), vec.end(), TypeInfoByDataTypeSorter() );

    return new SequenceResultSet(
        m_refMutex,
        *this,
        getStatics().typeinfoColumnNames,
        Sequence< Sequence< Any > > ( &vec[0] , vec.size() ),
        m_pSettings->tc,
        &( getStatics().typeInfoMetaData ));
}


static sal_Int32 seqContains( const Sequence< sal_Int32 > &seq, sal_Int32 value )
{
    sal_Int32 ret = -1;
    for( int i = 0; i < seq.getLength(); i ++ )
    {
        if( seq[i] == value )
        {
            ret = i;
            break;
        }
    }
    return ret;
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getIndexInfo(
    const ::com::sun::star::uno::Any& catalog,
    const OUString& schema,
    const OUString& table,
    sal_Bool unique,
    sal_Bool approximate ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();

    /*
       1. TABLE_CAT string -> table catalog (may be NULL )
       2. TABLE_SCHEM string -> table schema (may be NULL )
       3. TABLE_NAME string -> table name
       4. NON_UNIQUE boolean -> Can index values be non-unique?
                                false when TYPE is tableIndexStatistic
       5. INDEX_QUALIFIER string -> index catalog (may be NULL );
                                NULL when TYPE is tableIndexStatistic
       6. INDEX_NAME string -> index name; NULL when TYPE is tableIndexStatistic
       7. TYPE short -> index type:
              * 0 - this identifies table statistics that are returned
                    in conjuction with a table's index descriptions
              * CLUSTERED - this is a clustered index
              * HASHED - this is a hashed index
              * OTHER - this is some other style of index
       8. ORDINAL_POSITION short -> column sequence number within index;
                                    zero when TYPE is tableIndexStatistic
       9. COLUMN_NAME string -> column name; NULL when TYPE is tableIndexStatistic
      10. ASC_OR_DESC string -> column sort sequence, "A"= ascending,
                                "D" = descending, may be NULL if sort sequence
                                is not supported; NULL when TYPE is tableIndexStatistic
      11. CARDINALITY long -> When TYPE is tableIndexStatistic, then this is
                              the number of rows in the table; otherwise, it
                              is the number of unique values in the index.
      12. PAGES long -> When TYPE is tableIndexStatisic then this is
                        the number of pages used for the table, otherwise
                        it is the number of pages used for the current index.
      13. FILTER_CONDITION string -> Filter condition, if any. (may be NULL )

    */
    static const sal_Int32 C_SCHEMA = 1;
    static const sal_Int32 C_TABLENAME = 2;
    static const sal_Int32 C_INDEXNAME = 3;
    static const sal_Int32 C_IS_CLUSTERED = 4;
    static const sal_Int32 C_IS_UNIQUE = 5;
    static const sal_Int32 C_IS_PRIMARY = 6;
    static const sal_Int32 C_COLUMNS = 7;

    static const sal_Int32 R_TABLE_SCHEM = 1;
    static const sal_Int32 R_TABLE_NAME = 2;
    static const sal_Int32 R_NON_UNIQUE = 3;
    static const sal_Int32 R_INDEX_NAME = 5;
    static const sal_Int32 R_TYPE = 6;
    static const sal_Int32 R_ORDINAL_POSITION = 7;
    static const sal_Int32 R_COLUMN_NAME = 8;

    Reference< XPreparedStatement > stmt = m_origin->prepareStatement(
        ASCII_STR(
            "SELECT nspname, "          // 1
                   "pg_class.relname, " // 2
                   "class2.relname, "   // 3
                   "indisclustered, "   // 4
                   "indisunique, "      // 5
                   "indisprimary, "     // 6
                   "indkey "            // 7
            "FROM pg_index INNER JOIN pg_class ON indrelid = pg_class.oid "
                          "INNER JOIN pg_namespace ON pg_class.relnamespace = pg_namespace.oid "
                          "INNER JOIN pg_class as class2 ON pg_index.indexrelid = class2.oid "
            "WHERE nspname = ? AND pg_class.relname = ?" ) );

    Reference< XParameters > param ( stmt, UNO_QUERY );
    param->setString( 1, schema );
    param->setString( 2, table );
    Reference< XResultSet > rs = stmt->executeQuery();
    Reference< XRow > xRow ( rs, UNO_QUERY );

    SequenceAnyVector vec;
    while( rs->next() )
    {
        Sequence< sal_Int32 > columns = parseIntArray( xRow->getString(C_COLUMNS) );
        Reference< XPreparedStatement > columnsStmt = m_origin->prepareStatement(
            ASCII_STR(
                "SELECT attnum, attname "
                "FROM pg_attribute "
                "     INNER JOIN pg_class ON attrelid = pg_class.oid "
                "     INNER JOIN pg_namespace ON pg_class.relnamespace=pg_namespace.oid "
                "     WHERE pg_namespace.nspname=?  AND pg_class.relname=?" ) );
        Reference< XParameters > paramColumn ( columnsStmt, UNO_QUERY );
        OUString currentSchema = xRow->getString( C_SCHEMA );
        OUString currentTable = xRow->getString( C_TABLENAME );
        OUString currentIndexName = xRow->getString( C_INDEXNAME );
        sal_Bool isNonUnique = ! xRow->getBoolean( C_IS_UNIQUE );
        sal_Bool isPrimary = xRow->getBoolean( C_IS_PRIMARY );
        sal_Int32 indexType =  xRow->getBoolean( C_IS_CLUSTERED ) ?
            com::sun::star::sdbc::IndexType::CLUSTERED :
            com::sun::star::sdbc::IndexType::HASHED;

        paramColumn->setString( C_SCHEMA, currentSchema );
        paramColumn->setString( C_TABLENAME, currentTable );

        Reference< XResultSet > rsColumn = columnsStmt->executeQuery();
        Reference< XRow > rowColumn( rsColumn, UNO_QUERY );
        while( rsColumn->next() )
        {
            sal_Int32 pos = seqContains( columns, rowColumn->getInt( 1 ) );
            if( pos >= 0 && ( ! isNonUnique || !  unique ) )
            {
                Sequence< Any > result( 13 );
                result[R_TABLE_SCHEM] = makeAny(currentSchema);
                result[R_TABLE_NAME] = makeAny(currentTable);
                result[R_INDEX_NAME] = makeAny(currentIndexName);
                result[R_NON_UNIQUE] =
                    Any( &isNonUnique, getBooleanCppuType() );
                result[R_TYPE] = makeAny( indexType );
                result[R_COLUMN_NAME] = makeAny( rowColumn->getString(2) );
                sal_Int32 nPos = ((sal_Int32)pos+1); // MSVC++ nonsense
                result[R_ORDINAL_POSITION] = makeAny( nPos );
                vec.push_back( result );
            }
        }
    }
    return new SequenceResultSet(
        m_refMutex, *this, getStatics().indexinfoColumnNames,
        Sequence< Sequence< Any > > ( &vec[0] , vec.size() ),
        m_pSettings->tc );
}

sal_Bool DatabaseMetaData::supportsResultSetType( sal_Int32 setType )
    throw (SQLException, RuntimeException)
{
    return
        setType == com::sun::star::sdbc::ResultSetType::SCROLL_INSENSITIVE ||
        setType == com::sun::star::sdbc::ResultSetType::FORWARD_ONLY;
}

sal_Bool DatabaseMetaData::supportsResultSetConcurrency(
    sal_Int32 setType, sal_Int32 concurrency ) throw (SQLException, RuntimeException)
{
    return supportsResultSetType( setType ) &&
        (concurrency == com::sun::star::sdbc::TransactionIsolation::READ_COMMITTED ||
         concurrency == com::sun::star::sdbc::TransactionIsolation::SERIALIZABLE );
}

sal_Bool DatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::updatesAreDetected( sal_Int32 setType ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::deletesAreDetected( sal_Int32 setType ) throw (SQLException, RuntimeException)
{
    return sal_False;
}
sal_Bool DatabaseMetaData::insertsAreDetected( sal_Int32 setType ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool DatabaseMetaData::supportsBatchUpdates(  ) throw (SQLException, RuntimeException)
{
    return sal_False;
}

::com::sun::star::uno::Reference< XResultSet > DatabaseMetaData::getUDTs( const ::com::sun::star::uno::Any& catalog, const OUString& schemaPattern, const OUString& typeNamePattern, const ::com::sun::star::uno::Sequence< sal_Int32 >& types ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return new SequenceResultSet(
        m_refMutex, *this, Sequence< OUString >(), Sequence< Sequence< Any > > (), m_pSettings->tc );
}

::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > DatabaseMetaData::getConnection()
    throw (SQLException, RuntimeException)
{
    return m_origin;
}
}
