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

#include <rtl/ustrbuf.hxx>
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

ODatabaseMetaData::ODatabaseMetaData(OConnection& _rCon, MYSQL* pMySql)
    :m_rConnection(_rCon)
    ,m_pMySql(pMySql)
    ,identifier_quote_string_set(false)
{
}

ODatabaseMetaData::~ODatabaseMetaData()
{
}

rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogSeparator()
{
    return rtl::OUString();
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength()
{
        return 16777208L;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize()
{
        return 2147483647L - 8; // Max buffer size - HEADER
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength()
{
        return 32;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength()
{
        return 16777208;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength()
{
        return 64;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex()
{
        return 16;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength()
{
        return 64;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections()
{
        // TODO
        // SELECT @@max_connections
        return 100;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable()
{
        return 512;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength()
{
        // TODO
        // "SHOW VARIABLES LIKE 'max_allowed_packet'"
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength()
{
        return 64;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTablesInSelect()
{
        return 256;
}

sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs()
{
        return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers()
{
        // TODO
        return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers()
{
        //TODO;
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseQuotedIdentifiers()
{
        // TODO
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers()
{
        // TODO
        return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers()
{
        // TODO
        return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers()
{
        // TODO
        return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithAddColumn()
{
        return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithDropColumn()
{
        return true;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength()
{
        return 256;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns()
{
        return true;
}

rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogTerm()
{
    return rtl::OUString("n/a");
}

rtl::OUString SAL_CALL ODatabaseMetaData::getIdentifierQuoteString()
{
    return rtl::OUString("\"");
}

rtl::OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters()
{
    return rtl::OUString("#@");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::isCatalogAtStart()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel(sal_Int32 /*level*/)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInDataManipulation()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInTableDefinitions()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInTableDefinitions()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInDataManipulation()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins()
{
    return true;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatements()
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength()
{
    return 64;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength()
{
    return 64;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures()
{
    return m_rConnection.getMysqlVersion() >= 50000;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate()
{
    return m_rConnection.getMysqlVersion() >= 40000;
}

sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert(sal_Int32 /*fromType*/,  sal_Int32 /*toType*/)
{
    // TODO
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion()
{
    return m_rConnection.getMysqlVersion() >= 40000;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll()
{
    return m_rConnection.getMysqlVersion() >= 40000;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers()
{
    // TODO
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseQuotedIdentifiers()
{
    // TODO
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart()
{
    return m_rConnection.getMysqlVersion() > 40001 && m_rConnection.getMysqlVersion() < 40011;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow()
{
    return !nullsAreSortedHigh();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls()
{
    return m_rConnection.getMysqlVersion() >= 32200;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions()
{
    return m_rConnection.getMysqlVersion() >= 32200;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries()
{
    return m_rConnection.getMysqlVersion() >= 40100;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons()
{
    return m_rConnection.getMysqlVersion() >= 40100;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists()
{
    return m_rConnection.getMysqlVersion() >= 40100;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns()
{
    return m_rConnection.getMysqlVersion() >= 40100;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds()
{
    return m_rConnection.getMysqlVersion() >= 40100;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL()
{
    return false;
}

rtl::OUString SAL_CALL ODatabaseMetaData::getURL()
{
    return m_rConnection.getConnectionSettings().connectionURL;
}

rtl::OUString SAL_CALL ODatabaseMetaData::getUserName()
{
    // TODO execute "SELECT USER()"
    return rtl::OUString();
}

rtl::OUString SAL_CALL ODatabaseMetaData::getDriverName()
{
    return rtl::OUString( "MySQL Connector/OO.org" );
}

rtl::OUString SAL_CALL ODatabaseMetaData::getDriverVersion()
{
    return rtl::OUString( "0.9.2" );
}

rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion()
{
    return rtl::OStringToOUString(mysql_get_server_info(m_pMySql),
        m_rConnection.getConnectionEncoding());
}

rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName()
{
    return rtl::OUString("MySQL");
}

rtl::OUString SAL_CALL ODatabaseMetaData::getProcedureTerm()
{
    return rtl::OUString("procedure");
}

rtl::OUString SAL_CALL ODatabaseMetaData::getSchemaTerm()
{
    return rtl::OUString("database");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion()
{
    // TODO
    return MARIADBC_VERSION_MAJOR;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation()
{
    return m_rConnection.getMysqlVersion() >= 32336 ? TransactionIsolation::READ_COMMITTED :
        TransactionIsolation::NONE;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion()
{
    // TODO
    return MARIADBC_VERSION_MINOR;
}

rtl::OUString SAL_CALL ODatabaseMetaData::getSQLKeywords()
{
    return rtl::OUString(
        "ACCESSIBLE, ADD, ALL,"\
        "ALTER, ANALYZE, AND, AS, ASC, ASENSITIVE, BEFORE,"\
        "BETWEEN, BIGINT, BINARY, BLOB, BOTH, BY, CALL,"\
        "CASCADE, CASE, CHANGE, CHAR, CHARACTER, CHECK,"\
        "COLLATE, COLUMN, CONDITION, CONNECTION, CONSTRAINT,"\
        "CONTINUE, CONVERT, CREATE, CROSS, CURRENT_DATE,"\
        "CURRENT_TIME, CURRENT_TIMESTAMP, CURRENT_USER, CURSOR,"\
        "DATABASE, DATABASES, DAY_HOUR, DAY_MICROSECOND,"\
        "DAY_MINUTE, DAY_SECOND, DEC, DECIMAL, DECLARE,"\
        "DEFAULT, DELAYED, DELETE, DESC, DESCRIBE,"\
        "DETERMINISTIC, DISTINCT, DISTINCTROW, DIV, DOUBLE,"\
        "DROP, DUAL, EACH, ELSE, ELSEIF, ENCLOSED,"\
        "ESCAPED, EXISTS, EXIT, EXPLAIN, FALSE, FETCH,"\
        "FLOAT, FLOAT4, FLOAT8, FOR, FORCE, FOREIGN, FROM,"\
        "FULLTEXT, GRANT, GROUP, HAVING, HIGH_PRIORITY,"\
        "HOUR_MICROSECOND, HOUR_MINUTE, HOUR_SECOND, IF,"\
        "IGNORE, IN, INDEX, INFILE, INNER, INOUT,"\
        "INSENSITIVE, INSERT, INT, INT1, INT2, INT3, INT4,"\
        "INT8, INTEGER, INTERVAL, INTO, IS, ITERATE, JOIN,"\
        "KEY, KEYS, KILL, LEADING, LEAVE, LEFT, LIKE,"\
        "LOCALTIMESTAMP, LOCK, LONG, LONGBLOB, LONGTEXT,"\
        "LOOP, LOW_PRIORITY, MATCH, MEDIUMBLOB, MEDIUMINT,"\
        "MEDIUMTEXT, MIDDLEINT, MINUTE_MICROSECOND,"\
        "MINUTE_SECOND, MOD, MODIFIES, NATURAL, NOT,"\
        "NO_WRITE_TO_BINLOG, NULL, NUMERIC, ON, OPTIMIZE,"\
        "OPTION, OPTIONALLY, OR, ORDER, OUT, OUTER,"\
        "OUTFILE, PRECISION, PRIMARY, PROCEDURE, PURGE,"\
        "RANGE, READ, READS, READ_ONLY, READ_WRITE, REAL,"\
        "REFERENCES, REGEXP, RELEASE, RENAME, REPEAT,"\
        "REPLACE, REQUIRE, RESTRICT, RETURN, REVOKE, RIGHT,"\
        "RLIKE, SCHEMA, SCHEMAS, SECOND_MICROSECOND, SELECT,"\
        "SENSITIVE, SEPARATOR, SET, SHOW, SMALLINT, SPATIAL,"\
        "SPECIFIC, SQL, SQLEXCEPTION, SQLSTATE, SQLWARNING,"\
        "SQL_BIG_RESULT, SQL_CALC_FOUND_ROWS, SQL_SMALL_RESULT,"\
        "SSL, STARTING, STRAIGHT_JOIN, TABLE, TERMINATED,"\
        "THEN, TINYBLOB, TINYINT, TINYTEXT, TO, TRAILING,"\
        "TRIGGER, TRUE, UNDO, UNION, UNIQUE, UNLOCK,"\
        "UNSIGNED, UPDATE, USAGE, USE, USING, UTC_DATE,"\
        "UTC_TIME, UTC_TIMESTAMP, VALUES, VARBINARY, VARCHAR,"\
        "VARCHARACTER, VARYING, WHEN, WHERE, WHILE, WITH,"\
        "WRITE, X509, XOR, YEAR_MONTH, ZEROFILL" \
        "GENERAL, IGNORE_SERVER_IDS, MASTER_HEARTBEAT_PERIOD," \
        "MAXVALUE, RESIGNAL, SIGNAL, SLOW");
}

rtl::OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape()
{
    return rtl::OUString("\\");
}

rtl::OUString SAL_CALL ODatabaseMetaData::getStringFunctions()
{
    return rtl::OUString(
        "ASCII,BIN,BIT_LENGTH,CHAR,CHARACTER_LENGTH,CHAR_LENGTH,CONCAT,"
        "CONCAT_WS,CONV,ELT,EXPORT_SET,FIELD,FIND_IN_SET,HEX,INSERT,"
        "INSTR,LCASE,LEFT,LENGTH,LOAD_FILE,LOCATE,LOCATE,LOWER,LPAD,"
        "LTRIM,MAKE_SET,MATCH,MID,OCT,OCTET_LENGTH,ORD,POSITION,"
        "QUOTE,REPEAT,REPLACE,REVERSE,RIGHT,RPAD,RTRIM,SOUNDEX,"
        "SPACE,STRCMP,SUBSTRING,SUBSTRING,SUBSTRING,SUBSTRING,"
        "SUBSTRING_INDEX,TRIM,UCASE,UPPER");
}

rtl::OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions()
{
    return rtl::OUString(
        "DAYOFWEEK,WEEKDAY,DAYOFMONTH,DAYOFYEAR,MONTH,DAYNAME,"
        "MONTHNAME,QUARTER,WEEK,YEAR,HOUR,MINUTE,SECOND,PERIOD_ADD,"
        "PERIOD_DIFF,TO_DAYS,FROM_DAYS,DATE_FORMAT,TIME_FORMAT,"
        "CURDATE,CURRENT_DATE,CURTIME,CURRENT_TIME,NOW,SYSDATE,"
        "CURRENT_TIMESTAMP,UNIX_TIMESTAMP,FROM_UNIXTIME,"
        "SEC_TO_TIME,TIME_TO_SEC");
}

rtl::OUString SAL_CALL ODatabaseMetaData::getSystemFunctions()
{
    return rtl::OUString(
        "DATABASE,USER,SYSTEM_USER,"
        "SESSION_USER,PASSWORD,ENCRYPT,LAST_INSERT_ID,VERSION");
}

rtl::OUString SAL_CALL ODatabaseMetaData::getNumericFunctions()
{
    return rtl::OUString("ABS,ACOS,ASIN,ATAN,ATAN2,BIT_COUNT,CEILING,COS,"
                "COT,DEGREES,EXP,FLOOR,LOG,LOG10,MAX,MIN,MOD,PI,POW,"
                "POWER,RADIANS,RAND,ROUND,SIN,SQRT,TAN,TRUNCATE");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins()
{
    return true;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy()
{
    return 64;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy()
{
    return 64;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect()
{
    return 256;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength()
{
    return 16;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType(sal_Int32 setType)
{
    return setType == ResultSetType::SCROLL_SENSITIVE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency(sal_Int32 /*setType*/, sal_Int32 /*concurrency*/)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible(sal_Int32 /*setType*/)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible(sal_Int32 /*setType*/)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible(sal_Int32 /*setType*/)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible(sal_Int32 /*setType*/)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible(sal_Int32 /*setType*/)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible(sal_Int32 /*setType*/)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected(sal_Int32 /*setType*/)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected(sal_Int32 /*setType*/)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected(sal_Int32 /*setType*/)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates()
{
    return true;
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
    const char * const table_types[] = {"TABLE", "VIEW"};
    sal_Int32 const requiredVersion[] = {0, 50000};

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
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas()
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;

    rtl::OUString sSql = m_rConnection.getMysqlVersion() > 49999?
            rtl::OUString{"SELECT SCHEMA_NAME AS TABLE_SCHEM, CATALOG_NAME AS TABLE_CATALOG FROM INFORMATION_SCHEMA.SCHEMATA ORDER BY SCHEMA_NAME"}:
            rtl::OUString{"SHOW DATABASES"};

    Reference< XStatement > statement = m_rConnection.createStatement();
    Reference< XInterface > executed = statement->executeQuery(sSql);
    Reference< XResultSet > rs( executed, UNO_QUERY_THROW);
    Reference< XResultSetMetaDataSupplier > supp( executed, UNO_QUERY_THROW);
    Reference< XResultSetMetaData > rs_meta = supp->getMetaData();

    Reference< XRow > xRow( rs, UNO_QUERY_THROW );
    sal_uInt32 columns = rs_meta->getColumnCount();
    while( rs->next() )
    {
        std::vector< Any > aRow { Any() };
        bool informationSchema = false;
        for (sal_uInt32 i = 1; i <= columns; i++) {
            rtl::OUString columnStringValue = xRow->getString(i);
            if (i == 1) {   // TABLE_SCHEM
                informationSchema = columnStringValue.equalsIgnoreAsciiCase("information_schema");
            }
            aRow.push_back(makeAny(columnStringValue));
        }
        if (!informationSchema ) {
            rRows.push_back(aRow);
        }
    }

    lcl_setRows_throw(xResultSet, 1, rRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
        const Any& /*catalog*/,
        const rtl::OUString& schema,
        const rtl::OUString& table,
        const rtl::OUString& columnNamePattern)
{
    rtl::OUString query("SELECT TABLE_CATALOG AS TABLE_CAT, TABLE_SCHEMA AS "
            "TABLE_SCHEM, TABLE_NAME, COLUMN_NAME, NULL AS GRANTOR, "
            "GRANTEE, PRIVILEGE_TYPE AS PRIVILEGE, IS_GRANTABLE FROM "
            "INFORMATION_SCHEMA.COLUMN_PRIVILEGES WHERE TABLE_SCHEMA LIKE "
            "'?' AND TABLE_NAME='?' AND COLUMN_NAME LIKE '?' ORDER BY "
            "COLUMN_NAME, PRIVILEGE_TYPE");

    query = query.replaceFirst("?", schema);
    query = query.replaceFirst("?", table);
    query = query.replaceFirst("?", columnNamePattern);

    Reference<XStatement> statement = m_rConnection.createStatement();
    Reference<XResultSet> rs = statement->executeQuery(query);
    return rs;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
        const Any& /*catalog*/,
        const rtl::OUString& /*schemaPattern*/,
        const rtl::OUString& tableNamePattern,
        const rtl::OUString& columnNamePattern)
{
    rtl::OUStringBuffer queryBuf("SELECT TABLE_CATALOG AS TABLE_CAT, " // 1
        "TABLE_SCHEMA AS TABLE_SCHEM, " // 2
        "TABLE_NAME, " // 3
        "COLUMN_NAME, " // 4
        "DATA_TYPE, " // 5
        // TYPE_NAME missing
        "CHARACTER_MAXIMUM_LENGTH, " // 6
        "NUMERIC_PRECISION, " // 7
        // buffer length missing
        "NUMERIC_SCALE AS DECIMAL_DIGITS, " // 8
        // NUM_PREC_RADIX missing
        // NULLABLE missing
        "COLUMN_COMMENT AS REMARKS, " // 9
        "COLUMN_DEFAULT AS COLUMN_DEF," // 10
        "CHARACTER_OCTET_LENGTH, " // 11
        "ORDINAL_POSITION, " // 12
        "IS_NULLABLE, " // 13
        "COLUMN_TYPE " // 14
        "FROM INFORMATION_SCHEMA.COLUMNS "
        "WHERE (1 = 1) ");

    if (!tableNamePattern.isEmpty())
    {
        rtl::OUString sAppend;
        if (tableNamePattern.match("%"))
            sAppend = "AND TABLE_NAME LIKE '%' ";
        else
            sAppend = "AND TABLE_NAME = '%' ";
        queryBuf.append(sAppend.replaceAll("%", tableNamePattern));
    }
    if (!columnNamePattern.isEmpty())
    {
        rtl::OUString sAppend;
        if (columnNamePattern.match("%"))
            sAppend = "AND COLUMN_NAME LIKE '%' ";
        else
            sAppend = "AND COLUMN_NAME = '%' ";
        queryBuf.append(sAppend.replaceAll("%", columnNamePattern));
    }

    rtl::OUString query = queryBuf.makeStringAndClear();
    Reference<XStatement> statement = m_rConnection.createStatement();
    Reference<XResultSet> rs = statement->executeQuery(query.getStr());
    Reference<XRow> xRow( rs, UNO_QUERY_THROW );

    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance(
                "org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > aRows;
    while( rs->next() )
    {
        std::vector< Any > aRow { Any() }; // 0. element is unused

        // catalog name
        aRow.push_back(makeAny(xRow->getString(1)));
        // schema name
        aRow.push_back(makeAny(xRow->getString(2)));
        // table name
        aRow.push_back(makeAny(xRow->getString(3)));
        // column name
        aRow.push_back(makeAny(xRow->getString(4)));
        // data type
        rtl::OUString sDataType = xRow->getString(5);
        aRow.push_back(makeAny(mysqlc_sdbc_driver::mysqlStrToOOOType(sDataType)));
        // type name
        aRow.push_back(makeAny(sDataType)); // TODO
        // column size
        sal_Int32 nColumnSize = 0;
        rtl::OUString sColumnType = xRow->getString(14);
        sal_Int32 nCharMaxLen = xRow->getShort(6);
        bool bIsCharMax = !xRow->wasNull();
        if( sDataType.equalsIgnoreAsciiCase("year") )
            nColumnSize = sColumnType.copy(6, 1).toInt32(); // 'year(' length is 5
        else if(sDataType.equalsIgnoreAsciiCase("date"))
            nColumnSize = 10;
        else if(sDataType.equalsIgnoreAsciiCase("date"))
            nColumnSize = 8;
        else if(sDataType.equalsIgnoreAsciiCase("datetime")
                || sDataType.equalsIgnoreAsciiCase("timestamp"))
            nColumnSize = 19;
        else if(!bIsCharMax)
            nColumnSize = xRow->getShort(7); // numeric precision
        else
            nColumnSize = nCharMaxLen;
        aRow.push_back(makeAny(nColumnSize));
        aRow.push_back( Any() ); // buffer length - unused
        // decimal digits (scale)
        aRow.push_back(makeAny(xRow->getShort(8)));
        // num_prec_radix
        aRow.push_back(makeAny(sal_Int32(10)));
        // nullable
        rtl::OUString sIsNullable = xRow->getString(13);
        if(xRow->wasNull())
            aRow.push_back(makeAny(ColumnValue::NULLABLE_UNKNOWN));
        else if(sIsNullable.equalsIgnoreAsciiCase("YES"))
            aRow.push_back(makeAny(ColumnValue::NULLABLE));
        else
            aRow.push_back(makeAny(ColumnValue::NO_NULLS));
        // remarks
        aRow.push_back(makeAny(xRow->getString(9)));
        // default
        aRow.push_back(makeAny(xRow->getString(10)));

        aRow.push_back( Any{} ); // sql_data_type - unused
        aRow.push_back( Any{} ); // sql_datetime_sub - unused

        // character octet length
        aRow.push_back(makeAny(xRow->getString(11)));
        // ordinal position
        aRow.push_back(makeAny(xRow->getString(12)));
        // is nullable
        aRow.push_back(makeAny(sIsNullable));
        aRows.push_back(aRow);
    }
    lcl_setRows_throw(xResultSet, 1, aRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
        const Any& /*catalog*/,
        const rtl::OUString& schemaPattern,
        const rtl::OUString& tableNamePattern,
        const Sequence< rtl::OUString >& /*types */)
{
    rtl::OUString query("SELECT TABLE_CATALOG AS TABLE_CAT, TABLE_SCHEMA AS TABLE_SCHEM, TABLE_NAME,"
        "IF(STRCMP(TABLE_TYPE,'BASE TABLE'), TABLE_TYPE, 'TABLE') AS TABLE_TYPE, TABLE_COMMENT AS REMARKS "
        "FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA  LIKE '?' AND TABLE_NAME LIKE '?' "
        "ORDER BY TABLE_TYPE, TABLE_SCHEMA, TABLE_NAME");

    // TODO use prepared stmt instead
    // TODO escape schema, table name ?
    query = query.replaceFirst("?", schemaPattern);
    query = query.replaceFirst("?", tableNamePattern);

    Reference<XStatement> statement = m_rConnection.createStatement();
    Reference<XResultSet> rs = statement->executeQuery(query);
    return rs;
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
        const Any& /*catalog*/,
        const rtl::OUString& /*schemaPattern*/,
        const rtl::OUString& /*procedureNamePattern*/)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    // TODO IMPL
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
        const Any& /*catalog */,
        const rtl::OUString& /*schema */,
        const rtl::OUString& /*table */)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    // TODO implement
    lcl_setRows_throw(xResultSet, 8, rRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
        const Any& /*catalog*/,
        const rtl::OUString& /*schema*/,
        const rtl::OUString& /*table*/)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    // TODO implement
    lcl_setRows_throw(xResultSet,9,rRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
        const Any& /*catalog*/,
        const rtl::OUString& schema,
        const rtl::OUString& table)
{
    rtl::OUString query("SELECT TABLE_CATALOG AS TABLE_CAT, TABLE_SCHEMA "
            "AS TABLE_SCHEM, TABLE_NAME, " "COLUMN_NAME, SEQ_IN_INDEX AS KEY_SEQ,"
            "INDEX_NAME AS PK_NAME FROM INFORMATION_SCHEMA.STATISTICS "
            "WHERE TABLE_SCHEMA LIKE '?' AND TABLE_NAME LIKE '?' AND INDEX_NAME='PRIMARY' "
            "ORDER BY TABLE_SCHEMA, TABLE_NAME, INDEX_NAME, SEQ_IN_INDEX");

    // TODO use prepared stmt instead
    // TODO escape schema, table name ?
    query = query.replaceFirst("?", schema);
    query = query.replaceFirst("?", table);

    Reference<XStatement> statement = m_rConnection.createStatement();
    Reference<XResultSet> rs = statement->executeQuery(query);
    return rs;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
        const Any& /*catalog*/,
        const rtl::OUString& /*schema*/,
        const rtl::OUString& /*table*/,
        sal_Bool /*unique*/,
        sal_Bool /*approximate*/)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    // TODO
    lcl_setRows_throw(xResultSet, 11, rRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getBestRowIdentifier(
        const Any& /*catalog*/,
        const rtl::OUString& /*schema*/,
        const rtl::OUString& /*table*/,
        sal_Int32 /*scope*/,
        sal_Bool /*nullable*/)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    // TODO
    lcl_setRows_throw(xResultSet, 15, rRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
        const Any& /*catalog*/,
        const rtl::OUString& /*schemaPattern*/,
        const rtl::OUString& /*tableNamePattern*/)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    // TODO
    lcl_setRows_throw(xResultSet,12,rRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
        const Any& /*primaryCatalog*/,
        const rtl::OUString& /*primarySchema_*/,
        const rtl::OUString& /*primaryTable_*/,
        const Any& /*foreignCatalog*/,
        const rtl::OUString& /*foreignSchema*/,
        const rtl::OUString& /*foreignTable*/)
{
    Reference< XResultSet > xResultSet(getOwnConnection().getDriver().getFactory()->createInstance("org.openoffice.comp.helper.DatabaseMetaDataResultSet"),UNO_QUERY);
    std::vector< std::vector< Any > > rRows;
    // TODO
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
