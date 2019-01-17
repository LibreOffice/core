/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *   Some portions were adapted from JDBC PostgreSQL driver:
 *
 *    Copyright (c) 2004-2008, PostgreSQL Global Development Group
 *
 *   Licence of original JDBC driver code:
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *    3. Neither the name of the PostgreSQL Global Development Group nor the names
 *       of its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *    POSSIBILITY OF SUCH DAMAGE.
 *
 ************************************************************************/

#include <algorithm>
#include "pq_databasemetadata.hxx"
#include "pq_driver.hxx"
#include "pq_sequenceresultset.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/Deferrability.hpp>

using ::osl::MutexGuard;


using namespace com::sun::star::sdbc;

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;

namespace pq_sdbc_driver
{
#define QUOTEME(X)  #X
#define STRINGIFY(X) QUOTEME(X)

// These are pre-processor versions of KeyRule.idl declarations
// These are inherited from JDBC, and thus won't change anytime soon.
// Having them as pre-processor definitions allows to include them
// into compile-time strings (through STRINGIFY), which can be passed to ASCII_STR.
// That is without resorting to horrendous hacks in template meta-programming.
#define KEYRULE_CASCADE      0
#define KEYRULE_RESTRICT     1
#define KEYRULE_SET_NULL     2
#define KEYRULE_NO_ACTION    4
#define KEYRULE_SET_DEFAULT  4
// Ditto for Deferrability.idl
#define DEFERRABILITY_INITIALLY_DEFERRED  5
#define DEFERRABILITY_INITIALLY_IMMEDIATE 6
#define DEFERRABILITY_NONE                7

DatabaseMetaData::DatabaseMetaData(
    const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
    const css::uno::Reference< css::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings )
  : m_xMutex( refMutex ),
    m_pSettings( pSettings ),
    m_origin( origin ),
    m_getIntSetting_stmt ( m_origin->prepareStatement("SELECT setting FROM pg_catalog.pg_settings WHERE name=?") )
{
    init_getReferences_stmt();
    init_getPrivs_stmt();
}

sal_Bool DatabaseMetaData::allProceduresAreCallable(  )
{
    // TODO
    return false;
}

sal_Bool DatabaseMetaData::allTablesAreSelectable(  )
{
    return true;
}

OUString DatabaseMetaData::getURL(  )
{
    // TODO
    // LEM TODO: implement
    return OUString();
}

OUString DatabaseMetaData::getUserName(  )
{
    return m_pSettings->user;
}

sal_Bool DatabaseMetaData::isReadOnly(  )
{
    return false;
}


sal_Bool DatabaseMetaData::nullsAreSortedHigh(  )
{
    // Whether NULL values are considered, for sorting purposes, LARGER than any other value.
    // Specification: http://download.oracle.com/javase/6/docs/api/java/sql/DatabaseMetaData.html#nullsAreSortedHigh()
    // PostgreSQL behaviour: http://www.postgresql.org/docs/9.1/static/queries-order.html
    return true;
}

sal_Bool DatabaseMetaData::nullsAreSortedLow(  )
{
    return ! nullsAreSortedHigh();
}

sal_Bool DatabaseMetaData::nullsAreSortedAtStart(  )
{
    return false;
}

sal_Bool DatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return false;
}

OUString DatabaseMetaData::getDatabaseProductName(  )
{
    return OUString("PostgreSQL");
}

OUString DatabaseMetaData::getDatabaseProductVersion(  )
{
    return OUString::createFromAscii( PQparameterStatus( m_pSettings->pConnection, "server_version" ) );
}
OUString DatabaseMetaData::getDriverName(  )
{
    return OUString("postgresql-sdbc");
}

OUString DatabaseMetaData::getDriverVersion(  )
{
    return OUString(PQ_SDBC_DRIVER_VERSION);
}

sal_Int32 DatabaseMetaData::getDriverMajorVersion(  )
{
    return PQ_SDBC_MAJOR;
}

sal_Int32 DatabaseMetaData::getDriverMinorVersion(  )
{
    return PQ_SDBC_MINOR;
}

sal_Bool DatabaseMetaData::usesLocalFiles(  )
{
    // LEM TODO:
    //           http://wiki.openoffice.org/wiki/Documentation/DevGuide/Database/XDatabaseMetaData_Interface
    //           says "Returns true when the catalog name of the
    //           database should not appear in the DatasourceBrowser
    //           of OpenOffice.org API, otherwise false is returned."
    //           So, hmmm, think about it.
    return false;
}

sal_Bool DatabaseMetaData::usesLocalFilePerTable(  )
{
    return false;
}

sal_Bool DatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    return false;
}

sal_Bool DatabaseMetaData::storesUpperCaseIdentifiers(  )
{
    return false;
}

sal_Bool DatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    return true;
}


sal_Bool DatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    return false;
}


sal_Bool DatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  )
{
    return true;
}

sal_Bool DatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    return false;
}


sal_Bool DatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    return false;
}


sal_Bool DatabaseMetaData::storesMixedCaseQuotedIdentifiers(  )
{
    return false;
}


OUString DatabaseMetaData::getIdentifierQuoteString(  )
{
    return OUString("\"");
}

OUString DatabaseMetaData::getSQLKeywords(  )
{
    // In Java 6, this is all keywords that are not SQL:2003
    // In Java 2 v1.4 and as per LibreOffice SDK doc, this is all keywords that are not SQL92
    // I understand this to mean "reserved keywords" only.
    // See http://www.postgresql.org/docs/current/static/sql-keywords-appendix.html
    // LEM TODO: consider using pg_get_keywords(), filter on catcode
    return OUString(
        "ANALYSE,"
        "ANALYZE,"
        "ARRAY," //SQL:1999
        "ASYMMETRIC," //SQL:2003
        "BINARY," //SQL:1999
        "CONCURRENTLY,"
        "CURRENT_CATALOG," //SQL:2008
        "CURRENT_ROLE," //SQL:1999
        "CURRENT_SCHEMA," //SQL:2008
        "DO,"
        "FREEZE,"
        "ILIKE,"
        "ISNULL,"
        "LIMIT," //SQL:1999; non-reserved in SQL:2003
        "LOCALTIME," //SQL:1999
        "LOCALTIMESTAMP," //SQL:1999
        "NOTNULL,"
        "OFFSET," //SQL:2008
        "OVER," //SQL:2003
        "PLACING," //non-reserved in SQL:2003
        "RETURNING," //non-reserved in SQL:2008
        "SIMILAR," //SQL:2003
        "VARIADIC,"
        "VERBOSE,"
        "WINDOW" //SQL:2003
 );
}
OUString DatabaseMetaData::getNumericFunctions(  )
{
    // See http://www.postgresql.org/docs/9.1/static/functions-math.html
    // LEM TODO: Err... http://wiki.openoffice.org/wiki/Documentation/DevGuide/Database/Support_Scalar_Functions
    //           says this should be "Open Group CLI" names, not PostgreSQL names.
    //           Currently this is just a list of supported functions in PostgreSQL, with PostgreSQL names.
    //           And it is my job to map from Open Group CLI names/syntax to PostgreSQL names/syntax. Where? By parsing the SQL???
    //           Should look at what the JDBC driver is doing.
    return OUString(
        "abs,"
        "cbrt,"
        "ceil,"
        "ceiling,"
        "degrees,"
        "div,"
        "exp,"
        "floor,"
        "ln,"
        "log,"
        "mod,"
        "pi,"
        "power,"
        "radians,"
        "random,"
        "round,"
        "setseed,"
        "sign,"
        "sqrt,"
        "trunc,"
        "width_bucket,"
        "acos,"
        "asin,"
        "atan,"
        "atan2,"
        "cos,"
        "cot,"
        "sin,"
        "tan"
 );
}

OUString DatabaseMetaData::getStringFunctions(  )
{
    // See http://www.postgresql.org/docs/9.1/static/functions-string.html
    return OUString(
        "bit_length,"
        "char_length,"
        "character_length,"
        "lower,"
        "octet_length,"
        "overlay,"
        "position,"
        "substring,"
        "trim,"
        "upper,"
        "ascii,"
        "btrim,"
        "chr,"
        "concat,"
        "concat_ws,"
        "convert,"
        "convert_from,"
        "convert_to,"
        "decode,"
        "encode,"
        "format,"
        "initcap,"
        "left,"
        "length,"
        "lpad,"
        "ltrim,"
        "md5,"
        "pg_client_encoding,"
        "quote_ident,"
        "quote_literal,"
        "quote_nullable,"
        "regexp_matches,"
        "regexp_replace,"
        "regexp_split_to_array,"
        "regexp_split_to_table,"
        "repeat,"
        "replace,"
        "reverse,"
        "right,"
        "rpad,"
        "rtrim,"
        "split_part,"
        "strpos,"
        "substr,"
        "to_ascii,"
        "to_hex,"
        "translate"
 );
}

OUString DatabaseMetaData::getSystemFunctions(  )
{
    // See http://www.postgresql.org/docs/9.1/static/functions-info.html
    // and http://www.postgresql.org/docs/9.1/static/functions-admin.html
    return OUString(
        "current_catalog,"
        "current_database,"
        "current_query,"
        "current_schema,"
        "current_schemas,"
        "current_user,"
        "inet_client_addr,"
        "inet_client_port,"
        "inet_server_addr,"
        "inet_server_port,"
        "pg_backend_pid,"
        "pg_conf_load_time,"
        "pg_is_other_temp_schema,"
        "pg_listening_channels,"
        "pg_my_temp_schema,"
        "pg_postmaster_start_time,"
        "session_user,"
        "user,"
        "version,"
        "has_any_column_privilege,"
        "has_any_column_privilege,"
        "has_any_column_privilege,"
        "has_column_privilege,"
        "has_database_privilege,"
        "has_foreign_data_wrapper_privilege,"
        "has_function_privilege,"
        "has_language_privilege,"
        "has_schema_privilege,"
        "has_sequence_privilege,"
        "has_server_privilege,"
        "has_table_privilege,"
        "has_tablespace_privilege,"
        "pg_has_role,"
        "pg_collation_is_visible,"
        "pg_conversion_is_visible,"
        "pg_function_is_visible,"
        "pg_opclass_is_visible,"
        "pg_operator_is_visible,"
        "pg_table_is_visible,"
        "pg_ts_config_is_visible,"
        "pg_ts_dict_is_visible,"
        "pg_ts_parser_is_visible,"
        "pg_ts_template_is_visible,"
        "pg_type_is_visible,"
        "format_type,"
        "pg_describe_object,"
        "pg_get_constraintdef,"
        "pg_get_expr,"
        "pg_get_functiondef,"
        "pg_get_function_arguments,"
        "pg_get_function_identity_arguments,"
        "pg_get_function_result,"
        "pg_get_indexdef,"
        "pg_get_keywords,"
        "pg_get_ruledef,"
        "pg_get_serial_sequence,"
        "pg_get_triggerdef,"
        "pg_get_userbyid,"
        "pg_get_viewdef,"
        "pg_options_to_table,"
        "pg_tablespace_databases,"
        "pg_typeof,"
        "col_description,"
        "obj_description,"
        "shobj_description,"
        "txid_current,"
        "txid_current_snapshot,"
        "txid_snapshot_xip,"
        "txid_snapshot_xmax,"
        "txid_snapshot_xmin,"
        "txid_visible_in_snapshot,"
        "xmin,"
        "xmax,"
        "xip_list,"
        "current_setting,"
        "set_config,"
        "pg_cancel_backend,"
        "pg_reload_conf,"
        "pg_rotate_logfile,"
        "pg_terminate_backend,"
        "pg_create_restore_point,"
        "pg_current_xlog_insert_location,"
        "pg_current_xlog_location,"
        "pg_start_backup,"
        "pg_stop_backup,"
        "pg_switch_xlog,"
        "pg_xlogfile_name,"
        "pg_xlogfile_name_offset,"
        "pg_is_in_recovery,"
        "pg_last_xlog_receive_location,"
        "pg_last_xlog_replay_location,"
        "pg_last_xact_replay_timestamp,"
        "pg_is_xlog_replay_paused,"
        "pg_xlog_replay_pause,"
        "pg_xlog_replay_resume,"
        "pg_column_size,"
        "pg_database_size,"
        "pg_indexes_size,"
        "pg_relation_size,"
        "pg_size_pretty,"
        "pg_table_size,"
        "pg_tablespace_size,"
        "pg_tablespace_size,"
        "pg_total_relation_size,"
        "pg_relation_filenode,"
        "pg_relation_filepath,"
        "pg_ls_dir,"
        "pg_read_file,"
        "pg_read_binary_file,"
        "pg_stat_file,"
        "pg_advisory_lock,"
        "pg_advisory_lock_shared,"
        "pg_advisory_unlock,"
        "pg_advisory_unlock_all,"
        "pg_advisory_unlock_shared,"
        "pg_advisory_xact_lock,"
        "pg_advisory_xact_lock_shared,"
        "pg_try_advisory_lock,"
        "pg_try_advisory_lock_shared,"
        "pg_try_advisory_xact_lock,"
        "pg_try_advisory_xact_lock_shared,"
        "pg_sleep"
 );
}
OUString DatabaseMetaData::getTimeDateFunctions(  )
{
    // TODO
    return OUString(
        "age,"
        "age,"
        "clock_timestamp,"
        "current_date,"
        "current_time,"
        "current_timestamp,"
        "date_part,"
        "date_part,"
        "date_trunc,"
        "extract,"
        "extract,"
        "isfinite,"
        "isfinite,"
        "isfinite,"
        "justify_days,"
        "justify_hours,"
        "justify_interval,"
        "localtime,"
        "localtimestamp,"
        "now,"
        "statement_timestamp,"
        "timeofday,"
        "transaction_timestamp,"
 );
}
OUString DatabaseMetaData::getSearchStringEscape(  )
{
    return OUString("\\");
}
OUString DatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString("$");
}

sal_Bool DatabaseMetaData::supportsAlterTableWithAddColumn(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsAlterTableWithDropColumn(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsColumnAliasing(  )
{
    return true;
}

sal_Bool DatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsTypeConversion(  )
{
    // LEM: this is specifically whether the "CONVERT" function is supported
    //      It seems that in PostgreSQL, that function is only for string encoding, so no.
    return false;
}

sal_Bool DatabaseMetaData::supportsConvert( sal_Int32, sal_Int32 )
{
    return false;
}

sal_Bool DatabaseMetaData::supportsTableCorrelationNames(  )
{
    // LEM: A correlation name is "bar" in "SELECT foo FROM qux [AS] bar WHERE ..."
    return true;
}


sal_Bool DatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return false;
}
sal_Bool DatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsOrderByUnrelated(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsGroupBy(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsGroupByUnrelated(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsLikeEscapeClause(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsMultipleResultSets(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsMultipleTransactions(  )
{
    // Allows multiple transactions open at once (on different connections!)
    return true;
}

sal_Bool DatabaseMetaData::supportsNonNullableColumns(  )
{
    return true;
}


sal_Bool DatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsCoreSQLGrammar(  )
{
    // LEM: jdbc driver says not, although the comments in it seem old
    //      fdo#45249 Base query design won't use any aggregate function
    //      (except COUNT(*) unless we say yes, so say yes.
    //      Actually, Base assumes *also* support for aggregate functions "collect, fusion, intersection"
    //      as soon as supportsCoreSQLGrammar() returns true.
    //      Those are *not* Core SQL, though. They are in optional feature S271 "Basic multiset support"
    return true;
}

sal_Bool DatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    return false;
}

sal_Bool DatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    // LEM: jdbc driver says not, although the comments in it seem old
    return false;
}

sal_Bool DatabaseMetaData::supportsANSI92FullSQL(  )
{
    // LEM: jdbc driver says not, although the comments in it seem old
    return false;
}

sal_Bool DatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    // LEM: jdbc driver says yes, although comment says they are not sure what this means...
    return true;
}

sal_Bool DatabaseMetaData::supportsOuterJoins(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsFullOuterJoins(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return true;
}


OUString DatabaseMetaData::getSchemaTerm(  )
{
    return OUString("SCHEMA");
}

OUString DatabaseMetaData::getProcedureTerm(  )
{
    return OUString("function");
}

OUString DatabaseMetaData::getCatalogTerm(  )
{
    return OUString("DATABASE");
}

sal_Bool DatabaseMetaData::isCatalogAtStart(  )
{
    return true;
}

OUString DatabaseMetaData::getCatalogSeparator(  )
{
    return OUString(".");
}

sal_Bool DatabaseMetaData::supportsSchemasInDataManipulation(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsSchemasInTableDefinitions(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsSchemasInIndexDefinitions(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsCatalogsInDataManipulation(  )
{
    return false;
}

sal_Bool DatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    return false;
}

sal_Bool DatabaseMetaData::supportsCatalogsInTableDefinitions(  )
{
    return false;
}


sal_Bool DatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    return false;
}


sal_Bool DatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    return false;
}


// LEM TODO: positioned (through cursor) updates and deletes seem
// to be supported; see {UPDATE,DELETE} /table/ (...) WHERE CURRENT OF /cursor_name/" syntax
// and http://www.postgresql.org/docs/9.1/static/view-pg-cursors.html
// http://www.postgresql.org/docs/9.1/static/libpq-example.html actually uses a cursor :)
sal_Bool DatabaseMetaData::supportsPositionedDelete(  )
{
    // LEM: jdbc driver says not, although the comments in it seem old
    return false;
}

sal_Bool DatabaseMetaData::supportsPositionedUpdate(  )
{
    // LEM: jdbc driver says not, although the comments in it seem old
    return false;
}


sal_Bool DatabaseMetaData::supportsSelectForUpdate(  )
{
    return true;
}


sal_Bool DatabaseMetaData::supportsStoredProcedures(  )
{
    return true;
}


sal_Bool DatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsSubqueriesInExists(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsSubqueriesInIns(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    // LEM: jdbc driver says yes, although comment says they don't know what this means...
    return true;
}

sal_Bool DatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    return true;
}
sal_Bool DatabaseMetaData::supportsUnion(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsUnionAll(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    return false;
}

sal_Bool DatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    return false;
}

sal_Bool DatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    return true;
}
sal_Bool DatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    return true;
}

sal_Int32 DatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return 0;
}

sal_Int32 DatabaseMetaData::getMaxCharLiteralLength(  )
{
    return 0;
}

// Copied / adapted / simplified from JDBC driver
sal_Int32 DatabaseMetaData::getIntSetting(const OUString& settingName)
{
    MutexGuard guard( m_xMutex->GetMutex() );

    Reference< XParameters > params(m_getIntSetting_stmt, UNO_QUERY_THROW );
    params->setString(1, settingName );
    Reference< XResultSet > rs = m_getIntSetting_stmt->executeQuery();
    Reference< XRow > xRow( rs , UNO_QUERY_THROW );
    OSL_VERIFY(rs->next());
    OSL_ENSURE(rs->isFirst(), "postgresql-sdbc DatabaseMetaData getIntSetting not on first row");
    OSL_ENSURE(rs->isLast(),  "postgresql-sdbc DatabaseMetaData getIntSetting not on last row");
    return xRow->getInt(1);
}

sal_Int32 DatabaseMetaData::getMaxNameLength()
{
    if ( m_pSettings->maxNameLen == 0)
        m_pSettings->maxNameLen = getIntSetting( "max_identifier_length" );
    OSL_ENSURE(m_pSettings->maxNameLen, "postgresql-sdbc: maxNameLen is zero");
    return m_pSettings->maxNameLen;
}

sal_Int32 DatabaseMetaData::getMaxIndexKeys()
{
    if ( m_pSettings->maxIndexKeys == 0)
        m_pSettings->maxIndexKeys = getIntSetting("max_index_keys");
    OSL_ENSURE(m_pSettings->maxIndexKeys, "postgresql-sdbc: maxIndexKeys is zero");
    return m_pSettings->maxIndexKeys;
}

sal_Int32 DatabaseMetaData::getMaxColumnNameLength(  )
{
    return getMaxNameLength();
}

sal_Int32 DatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    return 0;
}

sal_Int32 DatabaseMetaData::getMaxColumnsInIndex(  )
{
    return getMaxIndexKeys();
}

sal_Int32 DatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    return 0;
}

sal_Int32 DatabaseMetaData::getMaxColumnsInSelect(  )
{
    return 0;
}

sal_Int32 DatabaseMetaData::getMaxColumnsInTable(  )
{
    return 1600;
}

sal_Int32 DatabaseMetaData::getMaxConnections(  )
{
    // LEM: The JDBC driver returns an arbitrary 8192; truth is as much as OS / hardware supports
    return 0;
}

sal_Int32 DatabaseMetaData::getMaxCursorNameLength(  ) //TODO, don't know
{
    return getMaxNameLength();
}

sal_Int32 DatabaseMetaData::getMaxIndexLength(  ) //TODO, don't know
{
    // LEM: that's the index itself, not its name
    return 0;
}

sal_Int32 DatabaseMetaData::getMaxSchemaNameLength(  )
{
    return getMaxNameLength();
}

sal_Int32 DatabaseMetaData::getMaxProcedureNameLength(  )
{
    return getMaxNameLength();
}

sal_Int32 DatabaseMetaData::getMaxCatalogNameLength(  )
{
    return getMaxNameLength();
}

sal_Int32 DatabaseMetaData::getMaxRowSize(  )
{
    // jdbc driver says 1GB, but http://www.postgresql.org/about/ says 1.6TB
    // and that 1GB is the maximum _field_ size
    // The row limit does not fit into a sal_Int32
    return 0;
}

sal_Bool DatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    // LEM: Err... PostgreSQL basically does not do BLOBs well
    //      In any case, BLOBs do not change the maximal row length AFAIK
    return true;
}

sal_Int32 DatabaseMetaData::getMaxStatementLength(  )
{
    // LEM: actually, that would be 2^sizeof(size_t)-1
    //      on the server? on the client (because of libpq)? minimum of the two? not sure
    //      Anyway, big, so say unlimited.
    return 0;
}

sal_Int32 DatabaseMetaData::getMaxStatements(  ) //TODO, don't know
{
    return 0;
}

sal_Int32 DatabaseMetaData::getMaxTableNameLength(  )
{
    return getMaxNameLength();
}

sal_Int32 DatabaseMetaData::getMaxTablesInSelect(  )
{
    return 0;
}

sal_Int32 DatabaseMetaData::getMaxUserNameLength(  )
{
    return getMaxNameLength();
}

sal_Int32 DatabaseMetaData::getDefaultTransactionIsolation(  )
{
    return css::sdbc::TransactionIsolation::READ_COMMITTED;
}

sal_Bool DatabaseMetaData::supportsTransactions(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level )
{
    if ( level == css::sdbc::TransactionIsolation::READ_COMMITTED
         || level == css::sdbc::TransactionIsolation::SERIALIZABLE
         || level == css::sdbc::TransactionIsolation::READ_UNCOMMITTED
         || level == css::sdbc::TransactionIsolation::REPEATABLE_READ)
        return true;
    else
        return false;
}

sal_Bool DatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    return true;
}

sal_Bool DatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    return false;
}

sal_Bool DatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    return false;
}

sal_Bool DatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    return false;
}

css::uno::Reference< XResultSet > DatabaseMetaData::getProcedures(
    const css::uno::Any&,
    const OUString&,
    const OUString& )
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

// LEM TODO: implement
// LEM TODO: at least fake the columns, even if no row.
    MutexGuard guard( m_xMutex->GetMutex() );
    return new SequenceResultSet(
        m_xMutex, *this, std::vector< OUString >(), std::vector< std::vector< Any > > (), m_pSettings->tc );
}

css::uno::Reference< XResultSet > DatabaseMetaData::getProcedureColumns(
    const css::uno::Any&,
    const OUString&,
    const OUString&,
    const OUString& )
{
    MutexGuard guard( m_xMutex->GetMutex() );
// LEM TODO: implement
// LEM TODO: at least fake the columns, even if no row.
    return new SequenceResultSet(
        m_xMutex, *this, std::vector< OUString >(), std::vector< std::vector< Any > >(), m_pSettings->tc );
}

css::uno::Reference< XResultSet > DatabaseMetaData::getTables(
    const css::uno::Any&,
    const OUString& schemaPattern,
    const OUString& tableNamePattern,
    const css::uno::Sequence< OUString >& )
{
    Statics &statics = getStatics();

    MutexGuard guard( m_xMutex->GetMutex() );

    if (isLog(m_pSettings, LogLevel::Info))
    {
        OUStringBuffer buf( 128 );
        buf.append( "DatabaseMetaData::getTables got called with " );
        buf.append( schemaPattern );
        buf.append( "." );
        buf.append( tableNamePattern );
        log(m_pSettings, LogLevel::Info, buf.makeStringAndClear());
    }
    // ignore catalog, as a single pq connection does not support multiple catalogs

    // LEM TODO: this does not give the right column names, not the right number of columns, etc.
    // Take "inspiration" from JDBC driver
    // Ah, this is used to create a XResultSet manually... Try to do it directly in SQL
    Reference< XPreparedStatement > statement = m_origin->prepareStatement(
            "SELECT "
            "DISTINCT ON (pg_namespace.nspname, relname ) " // avoid duplicates (pg_settings !)
            "pg_namespace.nspname, relname, relkind, pg_description.description "
            "FROM pg_namespace, pg_class LEFT JOIN pg_description ON pg_class.oid = pg_description.objoid "
            "WHERE relnamespace = pg_namespace.oid "
            "AND ( relkind = 'r' OR relkind = 'v') "
            "AND pg_namespace.nspname LIKE ? "
            "AND relname LIKE ? "
//            "ORDER BY pg_namespace.nspname || relname"
            );

    Reference< XParameters > parameters( statement, UNO_QUERY_THROW );
    parameters->setString( 1 , schemaPattern );
    parameters->setString( 2 , tableNamePattern );

    Reference< XResultSet > rs = statement->executeQuery();
    Reference< XRow > xRow( rs, UNO_QUERY_THROW );
    std::vector< std::vector<Any> > vec;

    while( rs->next() )
    {
        std::vector< Any > row( 5 );

        row[0] <<= m_pSettings->catalog;
        row[1] <<= xRow->getString( 1 );
        row[2] <<= xRow->getString( 2 );
        OUString type = xRow->getString(3);
        if( type == "r" )
        {
            if( xRow->getString(1) == "pg_catalog" )
            {
                row[3] <<= statics.SYSTEM_TABLE;
            }
            else
            {
                row[3] <<= statics.TABLE;
            }
        }
        else if( type == "v" )
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
        m_xMutex, *this, statics.tablesRowNames, vec, m_pSettings->tc );
}

namespace
{
    // sort no schema first, then "public", then normal schemas, then internal schemas
    int compare_schema(const OUString &nsA, const OUString &nsB)
    {
        if (nsA.isEmpty())
        {
            return nsB.isEmpty() ? 0 : -1;
        }
        else if (nsB.isEmpty())
        {
            assert(!nsA.isEmpty());
            return 1;
        }
        else if(nsA == "public")
        {
            return (nsB == "public") ? 0 : -1;
        }
        else if(nsB == "public")
        {
            assert(nsA != "public");
            return 1;
        }
        else if(nsA.startsWith("pg_"))
        {
            if(nsB.startsWith("pg_"))
                return nsA.compareTo(nsB);
            else
                return 1;
        }
        else if(nsB.startsWith("pg_"))
        {
            return -1;
        }
        else
        {
            return nsA.compareTo(nsB);
        }
    }

    struct SortInternalSchemasLastAndPublicFirst
    {
        bool operator () ( const std::vector< Any >  & a, const std::vector< Any >  & b )
        {
            OUString valueA;
            OUString valueB;
            a[0] >>= valueA;
            b[0] >>= valueB;
            return compare_schema(valueA, valueB);
        }
    };
}

css::uno::Reference< XResultSet > DatabaseMetaData::getSchemas(  )
{
    MutexGuard guard( m_xMutex->GetMutex() );

    if (isLog(m_pSettings, LogLevel::Info))
    {
        log(m_pSettings, LogLevel::Info, "DatabaseMetaData::getSchemas() got called");
    }
    // <b>TABLE_SCHEM</b> string =&amp;gt; schema name
    Reference< XStatement > statement = m_origin->createStatement();
    Reference< XResultSet > rs = statement->executeQuery(
        "SELECT nspname from pg_namespace" );
    // LEM TODO: look at JDBC driver and consider doing the same
    //           in particular, excluding temporary schemas, but maybe better through pg_is_other_temp_schema(oid) OR  == pg_my_temp_schema()

    Reference< XRow > xRow( rs, UNO_QUERY_THROW );
    std::vector< std::vector<Any> > vec;
    while( rs->next() )
    {
        std::vector<Any> row(1);
        row[0] <<= xRow->getString(1);
        vec.push_back( row );
    }

    // sort public first, sort internal schemas last, sort rest in alphabetic order
    std::sort( vec.begin(), vec.end(), SortInternalSchemasLastAndPublicFirst() );

    Reference< XCloseable > closeable( statement, UNO_QUERY );
    if( closeable.is() )
        closeable->close();
    return new SequenceResultSet(
        m_xMutex, *this, getStatics().schemaNames, vec, m_pSettings->tc );
}

css::uno::Reference< XResultSet > DatabaseMetaData::getCatalogs(  )
{
    // LEM TODO: return the current catalog like JDBC driver?
    //           at least fake the columns, even if no content
    MutexGuard guard( m_xMutex->GetMutex() );
    return new SequenceResultSet(
        m_xMutex, *this, std::vector< OUString >(), std::vector< std::vector< Any > >(), m_pSettings->tc );
}

css::uno::Reference< XResultSet > DatabaseMetaData::getTableTypes(  )
{
    // LEM TODO: this can be made dynamic, see JDBC driver
    MutexGuard guard( m_xMutex->GetMutex() );
    return new SequenceResultSet(
        m_xMutex, *this, getStatics().tableTypeNames, getStatics().tableTypeData,
        m_pSettings->tc );
}


/** returns the constant from sdbc.DataType
 */
sal_Int32 typeNameToDataType( const OUString &typeName, const OUString &typtype )
{
//     sal_Int32 ret = css::sdbc::DataType::DISTINCT;
    // map all unknown types to memo (longvarchar). This allows to show them in
    // string representation. Additionally, the edit-table-type-selection-box
    // is not so unusable anymore.
    sal_Int32 ret = css::sdbc::DataType::LONGVARCHAR;
    if( typtype == "b" )
    {
        // as long as the OOo framework does not support arrays,
        // the user is better of with interpreting arrays as strings !
//         if( typeName.getLength() && '_' == typeName[0] )
//         {
//             it's just a naming convention, but as long as we don't have anything better,
//             we take it as granted
//             ret = css::sdbc::DataType::ARRAY;
//         }
        // base type
        Statics &statics = getStatics();
        BaseTypeMap::const_iterator ii = statics.baseTypeMap.find( typeName );
        if( ii != statics.baseTypeMap.end() )
        {
            ret = ii->second;
        }
    }
    else if( typtype == "c" )
    {
        ret = css::sdbc::DataType::STRUCT;
    }
    else if( typtype == "d" )
    {
        ret = css::sdbc::DataType::LONGVARCHAR;
    }
    return ret;
}

namespace {
    bool isSystemColumn( sal_Int16 attnum )
    {
        return attnum <= 0;
    }

    // is not exported by the postgres header
    const int PQ_VARHDRSZ = sizeof( sal_Int32 );

    // Oh, quelle horreur
    // LEM TODO: Need to severely rewrite that!
    // should probably just "do the same" as ODBC or JDBC drivers...
    void extractPrecisionAndScale(
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
            case css::sdbc::DataType::NUMERIC:
            case css::sdbc::DataType::DECIMAL:
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
}

typedef std::unordered_map
<
    sal_Int32,
    DatabaseTypeDescription
> Oid2DatabaseTypeDescriptionMap;

static void columnMetaData2DatabaseTypeDescription(
    Oid2DatabaseTypeDescriptionMap &oidMap,
    const Reference< XResultSet > &rs,
    const Reference< XStatement > &stmt )
{
    Reference< XRow > row( rs, UNO_QUERY_THROW );
    int domains = 0;
    OUStringBuffer queryBuf(128);
    queryBuf.append( "SELECT oid,typtype,typname FROM pg_TYPE WHERE " );
    while( rs->next() )
    {
        if( row->getString( 9 ) == "d" && oidMap.find( row->getInt( 12 ) ) == oidMap.end() )
        {
            oidMap[row->getInt(12)] = DatabaseTypeDescription();
            if( domains )
                queryBuf.append( " OR " );
            queryBuf.append( "oid = " );
            queryBuf.append( row->getInt(12 ) );
            domains ++;
        }
    }
    rs->beforeFirst();

    if( domains )
    {
        Reference< XResultSet > rsDomain = stmt->executeQuery( queryBuf.makeStringAndClear() );
        row.set( rsDomain, UNO_QUERY_THROW );
        while( rsDomain->next() )
        {
            oidMap[row->getInt(1)] = DatabaseTypeDescription(row->getString(3), row->getString(2) );
        }
        disposeNoThrow( stmt );
    }

}


css::uno::Reference< XResultSet > DatabaseMetaData::getColumns(
    const css::uno::Any&,
    const OUString& schemaPattern,
    const OUString& tableNamePattern,
    const OUString& columnNamePattern )
{
    // LEM TODO: review in comparison with JDBC driver
    Statics &statics = getStatics();

    // continue !
    MutexGuard guard( m_xMutex->GetMutex() );

    if (isLog(m_pSettings, LogLevel::Info))
    {
        OUStringBuffer buf( 128 );
        buf.append( "DatabaseMetaData::getColumns got called with " );
        buf.append( schemaPattern );
        buf.append( "." );
        buf.append( tableNamePattern );
        buf.append( "." );
        buf.append( columnNamePattern );
        log(m_pSettings, LogLevel::Info, buf.makeStringAndClear());
    }

    // ignore catalog, as a single pq connection
    // does not support multiple catalogs anyway
    // We don't use information_schema.columns because it contains
    // only the columns the current user has any privilege over.

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
    //               => pg_description.description
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

    Reference< XPreparedStatement > statement = m_origin->prepareStatement(
            "SELECT pg_namespace.nspname, "  // 1
            "pg_class.relname, "             // 2
            "pg_attribute.attname, "         // 3
            "pg_type.typname, "              // 4
            "pg_attribute.atttypmod, "       // 5
            "pg_attribute.attnotnull, "      // 6
            "pg_type.typdefault, "           // 7
            "pg_type.typtype, "              // 8
            "pg_attrdef.adsrc, "             // 9
            "pg_description.description, "   // 10
            "pg_type.typbasetype, "          // 11
            "pg_attribute.attnum "           // 12
            "FROM pg_class, "
                 "pg_attribute LEFT JOIN pg_attrdef ON pg_attribute.attrelid = pg_attrdef.adrelid AND pg_attribute.attnum = pg_attrdef.adnum "
                              "LEFT JOIN pg_description ON pg_attribute.attrelid = pg_description.objoid AND pg_attribute.attnum=pg_description.objsubid,"
                 " pg_type, pg_namespace "
            "WHERE pg_attribute.attrelid = pg_class.oid "
                   "AND pg_attribute.atttypid = pg_type.oid "
                   "AND pg_class.relnamespace = pg_namespace.oid "
                   "AND NOT pg_attribute.attisdropped "
                   "AND pg_namespace.nspname LIKE ? "
                   "AND pg_class.relname LIKE ? "
                   "AND pg_attribute.attname LIKE ? "
            "ORDER BY pg_namespace.nspname, pg_class.relname, pg_attribute.attnum"
            );

    Reference< XParameters > parameters( statement, UNO_QUERY_THROW );
    parameters->setString( 1 , schemaPattern );
    parameters->setString( 2 , tableNamePattern );
    parameters->setString( 3 , columnNamePattern );

    Reference< XResultSet > rs = statement->executeQuery();
    Reference< XRow > xRow( rs, UNO_QUERY_THROW );
    std::vector< std::vector<Any> > vec;

    Oid2DatabaseTypeDescriptionMap domainMap;
    Reference< XStatement > domainTypeStmt = m_origin->createStatement();
    columnMetaData2DatabaseTypeDescription( domainMap, rs, domainTypeStmt );

    sal_uInt32 colNum(0);
    OUString sSchema( "#invalid#" );
    OUString sTable(  "#invalid#" );

    while( rs->next() )
    {
        if( ! isSystemColumn( xRow->getShort( 12 ) ) )
        {
            OUString sNewSchema( xRow->getString(1) );
            OUString sNewTable(  xRow->getString(2) );
            if ( sNewSchema != sSchema || sNewTable != sTable )
            {
                colNum = 1;
                sSchema = sNewSchema;
                sTable = sNewTable;
            }
            else
                ++colNum;
            sal_Int32 precision, scale, type;
            std::vector< Any > row( 18 );
            row[0] <<= m_pSettings->catalog;
            row[1] <<= sNewSchema;
            row[2] <<= sNewTable;
            row[3] <<= xRow->getString(3);
            if( xRow->getString(8) == "d" )
            {
                DatabaseTypeDescription desc( domainMap[xRow->getInt(11)] );
                type = typeNameToDataType( desc.typeName, desc.typeType );
            }
            else
            {
                type = typeNameToDataType( xRow->getString(4), xRow->getString(8) );
            }
            extractPrecisionAndScale( type, xRow->getInt(5) , &precision, &scale );
            row[4] <<= type;
            row[5] <<= xRow->getString(4);
            row[6] <<= precision;
            // row[7] BUFFER_LENGTH not used
            row[8] <<= scale;
            // row[9] RADIX TODO
            if( xRow->getBoolean( 6 ) && ! isSystemColumn(xRow->getInt( 12 )) )
            {
                row[10] <<= OUString::number(css::sdbc::ColumnValue::NO_NULLS);
                row[17] <<= statics.NO;
            }
            else
            {
                row[10] <<= OUString::number(css::sdbc::ColumnValue::NULLABLE);
                row[17] <<= statics.YES;
            }

            row[11] <<= xRow->getString( 10 ); // comment
            row[12] <<= xRow->getString( 9 ); // COLUMN_DEF = pg_type.typdefault
            // row[13] SQL_DATA_TYPE    not used
            // row[14] SQL_DATETIME_SUB not used
            row[15] <<= precision;
            row[16] <<= colNum ;

            vec.push_back( row );
        }
    }
    Reference< XCloseable > closeable( statement, UNO_QUERY );
    if( closeable.is() )
        closeable->close();

    return new SequenceResultSet(
        m_xMutex, *this, statics.columnRowNames, vec, m_pSettings->tc );
}

css::uno::Reference< XResultSet > DatabaseMetaData::getColumnPrivileges(
    const css::uno::Any&,
    const OUString& schema,
    const OUString& table,
    const OUString& columnNamePattern )
{
    MutexGuard guard( m_xMutex->GetMutex() );

    if (isLog(m_pSettings, LogLevel::Info))
    {
        OUStringBuffer buf( 128 );
        buf.append( "DatabaseMetaData::getColumnPrivileges got called with " );
        buf.append( schema );
        buf.append( "." );
        buf.append( table );
        buf.append( "." );
        buf.append( columnNamePattern );
        log(m_pSettings, LogLevel::Info, buf.makeStringAndClear());
    }

    Reference< XParameters > parameters( m_getColumnPrivs_stmt, UNO_QUERY_THROW );
    parameters->setString( 1 , schema );
    parameters->setString( 2 , table );
    parameters->setString( 3 , columnNamePattern );

    Reference< XResultSet > rs = m_getColumnPrivs_stmt->executeQuery();

    return rs;
}

css::uno::Reference< XResultSet > DatabaseMetaData::getTablePrivileges(
    const css::uno::Any&,
    const OUString& schemaPattern,
    const OUString& tableNamePattern )
{
    MutexGuard guard( m_xMutex->GetMutex() );

    if (isLog(m_pSettings, LogLevel::Info))
    {
        OUStringBuffer buf( 128 );
        buf.append( "DatabaseMetaData::getTablePrivileges got called with " );
        buf.append( schemaPattern );
        buf.append( "." );
        buf.append( tableNamePattern );
        log(m_pSettings, LogLevel::Info, buf.makeStringAndClear());
    }

    Reference< XParameters > parameters( m_getTablePrivs_stmt, UNO_QUERY_THROW );
    parameters->setString( 1 , schemaPattern );
    parameters->setString( 2 , tableNamePattern );

    Reference< XResultSet > rs = m_getTablePrivs_stmt->executeQuery();

    return rs;
}

css::uno::Reference< XResultSet > DatabaseMetaData::getBestRowIdentifier(
    const css::uno::Any&,
    const OUString&,
    const OUString&,
    sal_Int32,
    sal_Bool )
{
    //LEM TODO: implement! See JDBC driver
    MutexGuard guard( m_xMutex->GetMutex() );
    return new SequenceResultSet(
        m_xMutex, *this, std::vector< OUString >(), std::vector< std::vector< Any > >(), m_pSettings->tc );
}

css::uno::Reference< XResultSet > DatabaseMetaData::getVersionColumns(
    const css::uno::Any&,
    const OUString&,
    const OUString& )
{
    //LEM TODO: implement! See JDBC driver
    MutexGuard guard( m_xMutex->GetMutex() );
    return new SequenceResultSet(
        m_xMutex, *this, std::vector< OUString >(), std::vector< std::vector< Any > >(), m_pSettings->tc );
}

css::uno::Reference< XResultSet > DatabaseMetaData::getPrimaryKeys(
    const css::uno::Any&,
    const OUString& schema,
    const OUString& table )
{
    //LEM TODO: review
    MutexGuard guard( m_xMutex->GetMutex() );

//        1.  TABLE_CAT string =&gt; table catalog (may be NULL )
//        2. TABLE_SCHEM string =&gt; table schema (may be NULL )
//        3. TABLE_NAME string =&gt; table name
//        4. COLUMN_NAME string =&gt; column name
//        5. KEY_SEQ short =&gt; sequence number within primary key
//        6. PK_NAME string =&gt; primary key name (may be NULL )

    if (isLog(m_pSettings, LogLevel::Info))
    {
        OUStringBuffer buf( 128 );
        buf.append( "DatabaseMetaData::getPrimaryKeys got called with " );
        buf.append( schema );
        buf.append( "." );
        buf.append( table );
        log(m_pSettings, LogLevel::Info, buf.makeStringAndClear());
    }

    Reference< XPreparedStatement > statement = m_origin->prepareStatement(
            "SELECT nmsp.nspname, "
                    "cl.relname, "
                    "con.conkey, "
                    "con.conname, "
                    "con.conrelid "
            "FROM pg_constraint as con,pg_class as cl, pg_namespace as nmsp "
            "WHERE con.connamespace = nmsp.oid AND con.conrelid = cl.oid AND con.contype = 'p' "
                "AND nmsp.nspname LIKE ? AND cl.relname LIKE ?" );

    Reference< XParameters > parameters( statement, UNO_QUERY_THROW );
    parameters->setString( 1 , schema );
    parameters->setString( 2 , table );

    Reference< XResultSet > rs = statement->executeQuery();
    Reference< XRow > xRow( rs, UNO_QUERY_THROW );
    std::vector< std::vector<Any> > vec;

    while( rs->next() )
    {
        std::vector< Any > row( 6 );
        row[0] <<= m_pSettings->catalog;
        row[1] <<= xRow->getString(1);
        row[2] <<= xRow->getString(2);
        OUString array = xRow->getString(3);
        row[4] <<= xRow->getString(5); // the relid
        row[5] <<= xRow->getString(4);

        int i = 0;
        // now retrieve the columns information
        // unfortunately, postgresql does not allow array of variable size in
        // WHERE clauses (in the default installation), so we have to choose
        // this expensive and somewhat ugly way
        // annotation: postgresql shouldn't have chosen an array here, instead they
        //             should have multiple rows per table
        // LEM: to transform an array into several rows, see unnest;
        //      it is as simple as "SELECT foo, bar, unnest(qux) FROM ..."
        //      where qux is the column that contains an array.
        while( array[i] && '}' != array[i] )
        {
            i++;
            int start = i;
            while( array[i] && array[i] != '}' && array[i] != ',' ) i++;
            row[3] <<= array.copy(start, i - start );
            vec.push_back( row );
        }
    }

    {
        Reference< XCloseable > closeable( statement, UNO_QUERY );
        if( closeable.is() )
            closeable->close();
    }


    OUString lastTableOid;
    sal_Int32 index = 0;
    std::vector< std::vector< Any > > ret( vec.size() );
    int elements = 0;
    for (auto const& elem : vec)
    {

        std::vector< Any > row = elem;
        OUString tableOid;
        OUString attnum;

        row[4] >>= tableOid;
        row[3] >>= attnum;
        statement = m_origin->prepareStatement(
                "SELECT att.attname FROM "
                "pg_attribute AS att, pg_class AS cl WHERE "
                "att.attrelid = ? AND att.attnum = ?" );

        parameters.set( statement, UNO_QUERY_THROW );
        parameters->setString( 1 , tableOid );
        parameters->setString( 2 , attnum );

        rs = statement->executeQuery();
        xRow.set( rs, UNO_QUERY_THROW );
        if( rs->next() )
        {
            // column name
            row[3] <<= xRow->getString( 1 );
            if( tableOid != lastTableOid )
                index = 1;
            lastTableOid = tableOid;
            row[4] <<= OUString::number( index );
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
        m_xMutex, *this, getStatics().primaryKeyNames, ret, m_pSettings->tc );
}

// Copied / adapted / simplified from JDBC driver
#define SQL_CASE_KEYRULE "  WHEN 'c' THEN " STRINGIFY(KEYRULE_CASCADE) \
                         "  WHEN 'n' THEN " STRINGIFY(KEYRULE_SET_NULL) \
                         "  WHEN 'd' THEN " STRINGIFY(KEYRULE_SET_DEFAULT) \
                         "  WHEN 'r' THEN " STRINGIFY(KEYRULE_RESTRICT) \
                         "  WHEN 'a' THEN " STRINGIFY(KEYRULE_NO_ACTION) \
                         "  ELSE NULL "

#define SQL_GET_REFERENCES \
    "WITH con AS (SELECT oid, conname, contype, condeferrable, condeferred, conrelid, confrelid,  confupdtype, confdeltype, generate_subscripts(conkey,1) AS conkeyseq, unnest(conkey) AS conkey , unnest(confkey) AS confkey FROM pg_catalog.pg_constraint) " \
    "SELECT NULL::text AS PKTABLE_CAT, pkn.nspname AS PKTABLE_SCHEM, pkc.relname AS PKTABLE_NAME, pka.attname AS PKCOLUMN_NAME, " \
    " NULL::text AS FKTABLE_CAT, fkn.nspname AS FKTABLE_SCHEM, fkc.relname AS FKTABLE_NAME, fka.attname AS FKCOLUMN_NAME, " \
    " con.conkeyseq AS KEY_SEQ, " \
    " CASE con.confupdtype " \
    SQL_CASE_KEYRULE \
    " END AS UPDATE_RULE, " \
    " CASE con.confdeltype " \
    SQL_CASE_KEYRULE \
    " END AS DELETE_RULE, " \
    " con.conname AS FK_NAME, pkic.relname AS PK_NAME, " \
    " CASE " \
    "  WHEN con.condeferrable AND con.condeferred THEN " STRINGIFY(DEFERRABILITY_INITIALLY_DEFERRED) \
    "  WHEN con.condeferrable THEN " STRINGIFY(DEFERRABILITY_INITIALLY_IMMEDIATE) \
    "  ELSE " STRINGIFY(DEFERRABILITY_NONE) \
    " END AS DEFERRABILITY " \
    "FROM " \
    " pg_catalog.pg_namespace pkn, pg_catalog.pg_class pkc, pg_catalog.pg_attribute pka, " \
    " pg_catalog.pg_namespace fkn, pg_catalog.pg_class fkc, pg_catalog.pg_attribute fka, " \
    " con, pg_catalog.pg_depend dep, pg_catalog.pg_class pkic " \
    "WHERE pkn.oid = pkc.relnamespace AND pkc.oid = pka.attrelid AND pka.attnum = con.confkey AND con.confrelid = pkc.oid " \
    " AND  fkn.oid = fkc.relnamespace AND fkc.oid = fka.attrelid AND fka.attnum = con.conkey  AND con.conrelid  = fkc.oid " \
    " AND con.contype = 'f' AND con.oid = dep.objid AND pkic.oid = dep.refobjid AND pkic.relkind = 'i' AND dep.classid = 'pg_constraint'::regclass::oid AND dep.refclassid = 'pg_class'::regclass::oid "

#define SQL_GET_REFERENCES_PSCHEMA " AND pkn.nspname = ? "
#define SQL_GET_REFERENCES_PTABLE  " AND pkc.relname = ? "
#define SQL_GET_REFERENCES_FSCHEMA " AND fkn.nspname = ? "
#define SQL_GET_REFERENCES_FTABLE  " AND fkc.relname = ? "
#define SQL_GET_REFERENCES_ORDER_SOME_PTABLE "ORDER BY fkn.nspname, fkc.relname, conkeyseq"
#define SQL_GET_REFERENCES_ORDER_NO_PTABLE   "ORDER BY pkn.nspname, pkc.relname, conkeyseq"

#define SQL_GET_REFERENCES_NONE_NONE_NONE_NONE \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_ORDER_NO_PTABLE

#define SQL_GET_REFERENCES_SOME_NONE_NONE_NONE \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PSCHEMA \
    SQL_GET_REFERENCES_ORDER_NO_PTABLE

#define SQL_GET_REFERENCES_NONE_SOME_NONE_NONE \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PTABLE \
    SQL_GET_REFERENCES_ORDER_SOME_PTABLE

#define SQL_GET_REFERENCES_SOME_SOME_NONE_NONE \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PSCHEMA \
    SQL_GET_REFERENCES_PTABLE \
    SQL_GET_REFERENCES_ORDER_SOME_PTABLE

#define SQL_GET_REFERENCES_NONE_NONE_SOME_NONE \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_FSCHEMA \
    SQL_GET_REFERENCES_ORDER_NO_PTABLE

#define SQL_GET_REFERENCES_NONE_NONE_NONE_SOME \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_FTABLE \
    SQL_GET_REFERENCES_ORDER_NO_PTABLE

#define SQL_GET_REFERENCES_NONE_NONE_SOME_SOME \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_FSCHEMA \
    SQL_GET_REFERENCES_FTABLE \
    SQL_GET_REFERENCES_ORDER_NO_PTABLE

#define SQL_GET_REFERENCES_SOME_NONE_SOME_NONE \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PSCHEMA \
    SQL_GET_REFERENCES_FSCHEMA \
    SQL_GET_REFERENCES_ORDER_NO_PTABLE

#define SQL_GET_REFERENCES_SOME_NONE_NONE_SOME \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PSCHEMA \
    SQL_GET_REFERENCES_FTABLE \
    SQL_GET_REFERENCES_ORDER_NO_PTABLE

#define SQL_GET_REFERENCES_SOME_NONE_SOME_SOME \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PSCHEMA \
    SQL_GET_REFERENCES_FSCHEMA \
    SQL_GET_REFERENCES_FTABLE \
    SQL_GET_REFERENCES_ORDER_NO_PTABLE

#define SQL_GET_REFERENCES_NONE_SOME_SOME_NONE \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PTABLE \
    SQL_GET_REFERENCES_FSCHEMA \
    SQL_GET_REFERENCES_ORDER_SOME_PTABLE

#define SQL_GET_REFERENCES_NONE_SOME_NONE_SOME \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PTABLE \
    SQL_GET_REFERENCES_FTABLE \
    SQL_GET_REFERENCES_ORDER_SOME_PTABLE

#define SQL_GET_REFERENCES_NONE_SOME_SOME_SOME \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PTABLE \
    SQL_GET_REFERENCES_FSCHEMA \
    SQL_GET_REFERENCES_FTABLE \
    SQL_GET_REFERENCES_ORDER_SOME_PTABLE

#define SQL_GET_REFERENCES_SOME_SOME_SOME_NONE \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PSCHEMA \
    SQL_GET_REFERENCES_PTABLE \
    SQL_GET_REFERENCES_FSCHEMA \
    SQL_GET_REFERENCES_ORDER_SOME_PTABLE

#define SQL_GET_REFERENCES_SOME_SOME_NONE_SOME \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PSCHEMA \
    SQL_GET_REFERENCES_PTABLE \
    SQL_GET_REFERENCES_FTABLE \
    SQL_GET_REFERENCES_ORDER_SOME_PTABLE

#define SQL_GET_REFERENCES_SOME_SOME_SOME_SOME \
    SQL_GET_REFERENCES \
    SQL_GET_REFERENCES_PSCHEMA \
    SQL_GET_REFERENCES_PTABLE \
    SQL_GET_REFERENCES_FSCHEMA \
    SQL_GET_REFERENCES_FTABLE \
    SQL_GET_REFERENCES_ORDER_SOME_PTABLE

void DatabaseMetaData::init_getReferences_stmt ()
{
    m_getReferences_stmt[0]  = m_origin->prepareStatement(SQL_GET_REFERENCES_NONE_NONE_NONE_NONE);
    m_getReferences_stmt[1]  = m_origin->prepareStatement(SQL_GET_REFERENCES_SOME_NONE_NONE_NONE);
    m_getReferences_stmt[2]  = m_origin->prepareStatement(SQL_GET_REFERENCES_NONE_SOME_NONE_NONE);
    m_getReferences_stmt[3]  = m_origin->prepareStatement(SQL_GET_REFERENCES_SOME_SOME_NONE_NONE);
    m_getReferences_stmt[4]  = m_origin->prepareStatement(SQL_GET_REFERENCES_NONE_NONE_SOME_NONE);
    m_getReferences_stmt[5]  = m_origin->prepareStatement(SQL_GET_REFERENCES_SOME_NONE_SOME_NONE);
    m_getReferences_stmt[6]  = m_origin->prepareStatement(SQL_GET_REFERENCES_NONE_SOME_SOME_NONE);
    m_getReferences_stmt[7]  = m_origin->prepareStatement(SQL_GET_REFERENCES_SOME_SOME_SOME_NONE);
    m_getReferences_stmt[8]  = m_origin->prepareStatement(SQL_GET_REFERENCES_NONE_NONE_NONE_SOME);
    m_getReferences_stmt[9]  = m_origin->prepareStatement(SQL_GET_REFERENCES_SOME_NONE_NONE_SOME);
    m_getReferences_stmt[10] = m_origin->prepareStatement(SQL_GET_REFERENCES_NONE_SOME_NONE_SOME);
    m_getReferences_stmt[11] = m_origin->prepareStatement(SQL_GET_REFERENCES_SOME_SOME_NONE_SOME);
    m_getReferences_stmt[12] = m_origin->prepareStatement(SQL_GET_REFERENCES_NONE_NONE_SOME_SOME);
    m_getReferences_stmt[13] = m_origin->prepareStatement(SQL_GET_REFERENCES_SOME_NONE_SOME_SOME);
    m_getReferences_stmt[14] = m_origin->prepareStatement(SQL_GET_REFERENCES_NONE_SOME_SOME_SOME);
    m_getReferences_stmt[15] = m_origin->prepareStatement(SQL_GET_REFERENCES_SOME_SOME_SOME_SOME);
}

void DatabaseMetaData::init_getPrivs_stmt ()
{
    OUStringBuffer sSQL(300);
    sSQL.append(
            " SELECT dp.TABLE_CAT, dp.TABLE_SCHEM, dp.TABLE_NAME, dp.GRANTOR, pr.rolname AS GRANTEE, dp.privilege, dp.is_grantable "
            " FROM ("
            "  SELECT table_catalog AS TABLE_CAT, table_schema AS TABLE_SCHEM, table_name,"
            "         grantor, grantee, privilege_type AS PRIVILEGE, is_grantable"
            "  FROM information_schema.table_privileges");
    if ( PQserverVersion( m_pSettings->pConnection ) < 90200 )
        // information_schema.table_privileges does not fill in default ACLs when no ACL
        // assume default ACL is "owner has all privileges" and add it
        sSQL.append(
            " UNION "
            "  SELECT current_database() AS TABLE_CAT, pn.nspname AS TABLE_SCHEM, c.relname AS TABLE_NAME,"
            "         ro.rolname AS GRANTOR, rg.rolname AS GRANTEE, p.privilege, 'YES' AS is_grantable"
            "  FROM pg_catalog.pg_class c,"
            "       (VALUES ('SELECT'), ('INSERT'), ('UPDATE'), ('DELETE'), ('TRUNCATE'), ('REFERENCES'), ('TRIGGER')) p (privilege),"
            "       pg_catalog.pg_roles ro,"
            "       (  SELECT oid, rolname FROM pg_catalog.pg_roles"
            "         UNION ALL"
            "          VALUES (0::oid, 'PUBLIC')"
            "       ) AS rg (oid, rolname),"
            "       pg_catalog.pg_namespace pn"
            "  WHERE c.relkind IN ('r', 'v') AND c.relacl IS NULL AND pg_has_role(rg.oid, c.relowner, 'USAGE')"
            "        AND c.relowner=ro.oid AND c.relnamespace = pn.oid");
    sSQL.append(
            " ) dp,"
            " (SELECT oid, rolname FROM pg_catalog.pg_roles UNION ALL VALUES (0, 'PUBLIC')) pr"
            " WHERE table_schem LIKE ? AND table_name LIKE ? AND (dp.grantee = 'PUBLIC' OR pg_has_role(pr.oid, dp.grantee, 'USAGE'))"
            " ORDER BY table_schem, table_name, privilege" );

    m_getTablePrivs_stmt = m_origin->prepareStatement( sSQL.makeStringAndClear() );

    sSQL.append(
            " SELECT dp.TABLE_CAT, dp.TABLE_SCHEM, dp.TABLE_NAME, dp.COLUMN_NAME, dp.GRANTOR, pr.rolname AS GRANTEE, dp.PRIVILEGE, dp.IS_GRANTABLE FROM ("
            "  SELECT table_catalog AS TABLE_CAT, table_schema AS TABLE_SCHEM, table_name, column_name,"
            "         grantor, grantee, privilege_type AS PRIVILEGE, is_grantable"
            "  FROM information_schema.column_privileges");
    if ( PQserverVersion( m_pSettings->pConnection ) < 90200 )
        // information_schema.table_privileges does not fill in default ACLs when no ACL
        // assume default ACL is "owner has all privileges" and add it
        sSQL.append(
            " UNION "
            "  SELECT current_database() AS TABLE_CAT, pn.nspname AS TABLE_SCHEM, c.relname AS TABLE_NAME, a.attname AS column_name,"
            "         ro.rolname AS GRANTOR, rg.rolname AS GRANTEE, p.privilege, 'YES' AS is_grantable"
            "  FROM pg_catalog.pg_class c, pg_catalog.pg_attribute a,"
            "       (VALUES ('SELECT'), ('INSERT'), ('UPDATE'), ('REFERENCES')) p (privilege),"
            "       pg_catalog.pg_roles ro,"
            "       (  SELECT oid, rolname FROM pg_catalog.pg_roles"
            "         UNION ALL"
            "          VALUES (0::oid, 'PUBLIC')"
            "       ) AS rg (oid, rolname),"
            "       pg_catalog.pg_namespace pn"
            "  WHERE c.relkind IN ('r', 'v') AND c.relacl IS NULL AND pg_has_role(rg.oid, c.relowner, 'USAGE')"
            "        AND c.relowner=ro.oid AND c.relnamespace = pn.oid AND a.attrelid = c.oid AND a.attnum > 0");
    sSQL.append(
            " ) dp,"
            " (SELECT oid, rolname FROM pg_catalog.pg_roles UNION ALL VALUES (0, 'PUBLIC')) pr"
            " WHERE table_schem = ? AND table_name = ? AND column_name LIKE ? AND (dp.grantee = 'PUBLIC' OR pg_has_role(pr.oid, dp.grantee, 'USAGE'))"
            " ORDER BY column_name, privilege" );

    m_getColumnPrivs_stmt = m_origin->prepareStatement( sSQL.makeStringAndClear() );
}

css::uno::Reference< XResultSet > DatabaseMetaData::getImportedExportedKeys(
    const Any& /* primaryCatalog */,
    const OUString& primarySchema,
    const OUString& primaryTable,
    const Any& /* foreignCatalog */,
    const OUString& foreignSchema,
    const OUString& foreignTable )
{
    unsigned int i = 0;
    if ( ! primarySchema.isEmpty() )
        i |=  0x01;
    if ( ! primaryTable.isEmpty() )
        i |=  0x02;
    if ( ! foreignSchema.isEmpty() )
        i |=  0x04;
    if ( ! foreignTable.isEmpty() )
        i |=  0x08;

    Reference< XPreparedStatement > stmt = m_getReferences_stmt[i];
    Reference< XParameters > param ( stmt, UNO_QUERY_THROW );

    unsigned int j = 1;
    if ( i & 0x01 )
        param->setString( j++, primarySchema );
    if ( i & 0x02 )
        param->setString( j++, primaryTable  );
    if ( i & 0x04 )
        param->setString( j++, foreignSchema );
    if ( i & 0x08 )
        param->setString( j++, foreignTable  );

    Reference< XResultSet > rs = stmt->executeQuery();

    return rs;
}


css::uno::Reference< XResultSet > DatabaseMetaData::getImportedKeys(
    const css::uno::Any& catalog,
    const OUString& schema,
    const OUString& table )
{
    return getImportedExportedKeys(Any(), OUString(), OUString(), catalog, schema, table);
}

css::uno::Reference< XResultSet > DatabaseMetaData::getExportedKeys(
    const css::uno::Any& catalog,
    const OUString& schema,
    const OUString& table )
{
    return getImportedExportedKeys(catalog, schema, table, Any(), OUString(), OUString());
}

css::uno::Reference< XResultSet > DatabaseMetaData::getCrossReference(
    const css::uno::Any& primaryCatalog,
    const OUString& primarySchema,
    const OUString& primaryTable,
    const css::uno::Any& foreignCatalog,
    const OUString& foreignSchema,
    const OUString& foreignTable )
{
    return getImportedExportedKeys( primaryCatalog, primarySchema, primaryTable, foreignCatalog, foreignSchema, foreignTable );
}

namespace
{
    struct TypeInfoByDataTypeSorter
    {
        bool operator () ( const std::vector< Any > & a, const std::vector< Any > & b )
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
                OUString nsA, tnA, nsB, tnB;

                // parse typename into schema and typename
                sal_Int32 nIndex=0;
                nsA = nameA.getToken(0, '.', nIndex);
                if (nIndex<0)
                {
                    tnA = nsA;
                    nsA.clear();
                }
                else
                {
                    tnA = nameA.getToken(0, '.', nIndex);
                    assert(nIndex < 0);
                }

                nIndex=0;
                nsB = nameB.getToken(0, '.', nIndex);
                if (nIndex<0)
                {
                    tnB = nsB;
                    nsB.clear();
                }
                else
                {
                    tnB = nameB.getToken(0, '.', nIndex);
                    assert(nIndex < 0);
                }

                const int ns_comp = compare_schema(nsA, nsB);
                if(ns_comp == 0)
                {
                    if(nsA.isEmpty())
                    {
                        assert(nsB.isEmpty());
                        // within each type category, sort privileged choice first
                        if( tnA == "int4" || tnA == "varchar" || tnA == "char" || tnA == "text")
                            return true;
                        if( tnB == "int4" || tnB == "varchar" || tnB == "char" || tnB == "text")
                            return false;
                    }
                    return nameA.compareTo( nameB ) < 0;
                }
                else
                {
                    return ns_comp < 0;
                }
            }

            return valueA.toInt32() < valueB.toInt32();
        }
    };

    sal_Int32 calcSearchable( sal_Int32 dataType )
    {
        sal_Int32 ret = css::sdbc::ColumnSearch::FULL;
        if( css::sdbc::DataType::BINARY == dataType ||
            css::sdbc::DataType::VARBINARY == dataType ||
            css::sdbc::DataType::LONGVARBINARY == dataType )
            ret = css::sdbc::ColumnSearch::NONE;

        return ret;
    }

    sal_Int32 getMaxScale( sal_Int32 dataType )
    {
        // LEM TODO: review, see where used, see JDBC, ...
        sal_Int32 ret = 0;
        if( dataType == css::sdbc::DataType::NUMERIC )
            ret = 1000; // see pg-docs DataType/numeric
//     else if( dataType == DataType::DOUBLE )
//         ret = 308;
//     else if( dataType == DataType::FLOAT )
//         ret =
        return ret;
    }

    OUString construct_full_typename(const OUString &ns, const OUString &tn)
    {
        if(ns.isEmpty() || ns == "pg_catalog")
            return tn;
        else
            return ns + "." + tn;
    }

    void pgTypeInfo2ResultSet(
         std::vector< std::vector<Any> > &vec,
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
        // FIXED_PREC_SCALE = 10; boolean ==> can it be a money value?
        static const sal_Int32 AUTO_INCREMENT = 11; // boolean ==> can it be used for
                                                    // an auto-increment value?
        static const sal_Int32 MINIMUM_SCALE = 13; // short ==> minimum scale supported
        static const sal_Int32 MAXIMUM_SCALE = 14; // short ==> maximum scale supported
        static const sal_Int32 NUM_PREC_RADIX = 17; // long ==> usually 2 or 10

        /*  not filled so far
            3. LITERAL_PREFIX string ==> prefix used to quote a literal
                                         (may be <NULL/>)
            4. LITERAL_SUFFIX string ==> suffix used to quote a literal
                                         (may be <NULL/>)
            5. CREATE_PARAMS string ==> parameters used in creating the type (may be <NULL/>)
            12. LOCAL_TYPE_NAME  string ==> localized version of type name (may be <NULL/>)
            15, SQL_DATA_TYPE long ==> unused
            16. SQL_DATETIME_SUB long ==> unused
         */
        Reference< XRow > xRow( rs, UNO_QUERY_THROW );
        while( rs->next() )
        {
            std::vector< Any > row(18);

            sal_Int32 dataType =typeNameToDataType(xRow->getString(5),xRow->getString(2));
            sal_Int32 precision = xRow->getString(3).toInt32();

            if( dataType == css::sdbc::DataType::CHAR  ||
                ( dataType == css::sdbc::DataType::VARCHAR &&
                  xRow->getString(TYPE_NAME+1).equalsIgnoreAsciiCase("varchar") ) )
            {
                // reflect varchar as varchar with upper limit !
                //NOTE: the sql spec requires varchar to have an upper limit, however
                //      in postgresql the upper limit is optional, no limit means unlimited
                //      length (=1GB).
                precision = 0x40000000; // about 1 GB, see character type docs in postgresql
                row[CREATE_PARAMS] <<= OUString("length");
            }
            else if( dataType == css::sdbc::DataType::NUMERIC )
            {
                precision = 1000;
                row[CREATE_PARAMS] <<= OUString("length, scale");
            }

            row[TYPE_NAME] <<= construct_full_typename(xRow->getString(6), xRow->getString(1));
            row[DATA_TYPE] <<= OUString::number(dataType);
            row[PRECISION] <<= OUString::number( precision );
            sal_Int32 nullable = xRow->getBoolean(4) ?
                css::sdbc::ColumnValue::NO_NULLS :
                css::sdbc::ColumnValue::NULLABLE;
            row[NULLABLE] <<= OUString::number(nullable);
            row[CASE_SENSITIVE] <<= OUString::number(1);
            row[SEARCHABLE] <<= OUString::number( calcSearchable( dataType ) );
            row[UNSIGNED_ATTRIBUTE] <<= OUString("0");
            if( css::sdbc::DataType::INTEGER == dataType ||
                css::sdbc::DataType::BIGINT == dataType )
                row[AUTO_INCREMENT] <<= OUString("1");     // TODO
            else
                row[AUTO_INCREMENT] <<= OUString("0");     // TODO
            row[MINIMUM_SCALE] <<= OUString("0");      // TODO: what is this ?
            row[MAXIMUM_SCALE] <<= OUString::number( getMaxScale( dataType ) );
            row[NUM_PREC_RADIX] <<= OUString("10");    // TODO: what is this ?
            vec.push_back( row );
        }
    }
}


css::uno::Reference< XResultSet > DatabaseMetaData::getTypeInfo(  )
{
    // Note: Indexes start at 0 (in the API doc, they start at 1)
    MutexGuard guard( m_xMutex->GetMutex() );

    if (isLog(m_pSettings, LogLevel::Info))
    {
        log(m_pSettings, LogLevel::Info, "DatabaseMetaData::getTypeInfo() got called");
    }

    Reference< XStatement > statement = m_origin->createStatement();
    Reference< XResultSet > rs = statement->executeQuery(
          "SELECT pg_type.typname AS typname," //1
          "pg_type.typtype AS typtype,"        //2
          "pg_type.typlen AS typlen,"          //3
          "pg_type.typnotnull AS typnotnull,"  //4
          "pg_type.typname AS typname, "       //5
          "pg_namespace.nspname as typns "     //6
          "FROM pg_type LEFT JOIN pg_namespace ON pg_type.typnamespace=pg_namespace.oid "
          "WHERE pg_type.typtype = 'b' "
          "OR pg_type.typtype = 'p'"
            );

    std::vector< std::vector<Any> > vec;
    pgTypeInfo2ResultSet( vec, rs );

    // check for domain types
    rs = statement->executeQuery(
        "SELECT t1.typname as typname,"
        "t2.typtype AS typtype,"
        "t2.typlen AS typlen,"
        "t2.typnotnull AS typnotnull,"
        "t2.typname as realtypname, "
        "pg_namespace.nspname as typns "
        "FROM pg_type as t1 LEFT JOIN pg_type AS t2 ON t1.typbasetype=t2.oid LEFT JOIN pg_namespace ON t1.typnamespace=pg_namespace.oid "
        "WHERE t1.typtype = 'd'" );
    pgTypeInfo2ResultSet( vec, rs );

    std::sort( vec.begin(), vec.end(), TypeInfoByDataTypeSorter() );

    return new SequenceResultSet(
        m_xMutex,
        *this,
        getStatics().typeinfoColumnNames,
        vec,
        m_pSettings->tc,
        &( getStatics().typeInfoMetaData ));
}


css::uno::Reference< XResultSet > DatabaseMetaData::getIndexInfo(
    const css::uno::Any& ,
    const OUString& schema,
    const OUString& table,
    sal_Bool unique,
    sal_Bool )
{
    //LEM TODO: review
    MutexGuard guard( m_xMutex->GetMutex() );

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
                    in conjunction with a table's index descriptions
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
    // C_IS_PRIMARY = 6
    static const sal_Int32 C_COLUMNS = 7;

    static const sal_Int32 R_TABLE_SCHEM = 1;
    static const sal_Int32 R_TABLE_NAME = 2;
    static const sal_Int32 R_NON_UNIQUE = 3;
    static const sal_Int32 R_INDEX_NAME = 5;
    static const sal_Int32 R_TYPE = 6;
    static const sal_Int32 R_ORDINAL_POSITION = 7;
    static const sal_Int32 R_COLUMN_NAME = 8;

    Reference< XPreparedStatement > stmt = m_origin->prepareStatement(
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
            "WHERE nspname = ? AND pg_class.relname = ?" );

    Reference< XParameters > param ( stmt, UNO_QUERY_THROW );
    param->setString( 1, schema );
    param->setString( 2, table );
    Reference< XResultSet > rs = stmt->executeQuery();
    Reference< XRow > xRow ( rs, UNO_QUERY_THROW );

    std::vector< std::vector<Any> > vec;
    while( rs->next() )
    {
        std::vector< sal_Int32 > columns = parseIntArray( xRow->getString(C_COLUMNS) );
        Reference< XPreparedStatement > columnsStmt = m_origin->prepareStatement(
                "SELECT attnum, attname "
                "FROM pg_attribute "
                "     INNER JOIN pg_class ON attrelid = pg_class.oid "
                "     INNER JOIN pg_namespace ON pg_class.relnamespace=pg_namespace.oid "
                "     WHERE pg_namespace.nspname=?  AND pg_class.relname=?" );
        Reference< XParameters > paramColumn ( columnsStmt, UNO_QUERY_THROW );
        OUString currentSchema = xRow->getString( C_SCHEMA );
        OUString currentTable = xRow->getString( C_TABLENAME );
        OUString currentIndexName = xRow->getString( C_INDEXNAME );
        bool isNonUnique = ! xRow->getBoolean( C_IS_UNIQUE );
        sal_Int32 indexType =  xRow->getBoolean( C_IS_CLUSTERED ) ?
            css::sdbc::IndexType::CLUSTERED :
            css::sdbc::IndexType::HASHED;

        paramColumn->setString( C_SCHEMA, currentSchema );
        paramColumn->setString( C_TABLENAME, currentTable );

        Reference< XResultSet > rsColumn = columnsStmt->executeQuery();
        Reference< XRow > rowColumn( rsColumn, UNO_QUERY_THROW );
        while( rsColumn->next() )
        {
            auto findIt = std::find( columns.begin(), columns.end(), rowColumn->getInt( 1 ) );
            if( findIt != columns.end() && ( ! isNonUnique || !  unique ) )
            {
                std::vector< Any > result( 13 );
                result[R_TABLE_SCHEM] <<= currentSchema;
                result[R_TABLE_NAME] <<= currentTable;
                result[R_INDEX_NAME] <<= currentIndexName;
                result[R_NON_UNIQUE] <<= isNonUnique;
                result[R_TYPE] <<= indexType;
                result[R_COLUMN_NAME] <<= rowColumn->getString(2);
                sal_Int32 nPos = static_cast<sal_Int32>(findIt - columns.begin() +1); // MSVC++ nonsense
                result[R_ORDINAL_POSITION] <<= nPos;
                vec.push_back( result );
            }
        }
    }
    return new SequenceResultSet(
        m_xMutex, *this, getStatics().indexinfoColumnNames,
        vec,
        m_pSettings->tc );
}

sal_Bool DatabaseMetaData::supportsResultSetType( sal_Int32 setType )
{
    if ( setType == css::sdbc::ResultSetType::SCROLL_SENSITIVE )
        return false;
    else
        return true;
}

sal_Bool DatabaseMetaData::supportsResultSetConcurrency(
    sal_Int32 setType, sal_Int32 )
{
    if ( ! supportsResultSetType( setType ) )
        return false;
    else
        return true;
}

sal_Bool DatabaseMetaData::ownUpdatesAreVisible( sal_Int32 /* setType */ )
{
    return true;
}

sal_Bool DatabaseMetaData::ownDeletesAreVisible( sal_Int32 /* setType */ )
{
    return true;
}

sal_Bool DatabaseMetaData::ownInsertsAreVisible( sal_Int32 /* setType */ )
{
    return true;
}

sal_Bool DatabaseMetaData::othersUpdatesAreVisible( sal_Int32 /* setType */ )
{
    return false;
}

sal_Bool DatabaseMetaData::othersDeletesAreVisible( sal_Int32 /* setType */ )
{
    return false;
}

sal_Bool DatabaseMetaData::othersInsertsAreVisible( sal_Int32 /* setType */ )
{
    return false;
}

sal_Bool DatabaseMetaData::updatesAreDetected( sal_Int32 /* setType */ )
{
    return false;
}

sal_Bool DatabaseMetaData::deletesAreDetected( sal_Int32 /* setType */ )
{
    return false;
}
sal_Bool DatabaseMetaData::insertsAreDetected( sal_Int32 /* setType */ )
{
    return false;
}

sal_Bool DatabaseMetaData::supportsBatchUpdates(  )
{
    return true;
}

css::uno::Reference< XResultSet > DatabaseMetaData::getUDTs( const css::uno::Any&, const OUString&, const OUString&, const css::uno::Sequence< sal_Int32 >& )
{
    //LEM TODO: implement! See JDBC driver
    MutexGuard guard( m_xMutex->GetMutex() );
    return new SequenceResultSet(
        m_xMutex, *this, std::vector< OUString >(), std::vector< std::vector< Any > >(), m_pSettings->tc );
}

css::uno::Reference< css::sdbc::XConnection > DatabaseMetaData::getConnection()
{
    return m_origin;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
