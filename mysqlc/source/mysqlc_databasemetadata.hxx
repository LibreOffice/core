/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

#ifndef CONNECTIVITY_SDATABASEMETADATA_HXX
#define CONNECTIVITY_SDATABASEMETADATA_HXX

#include "mysqlc_connection.hxx"

#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <cppuhelper/implbase1.hxx>

#include <cppconn/metadata.h>

namespace connectivity
{
    namespace mysqlc
    {
        typedef ::com::sun::star::sdbc::SQLException my_SQLException;
        typedef ::com::sun::star::uno::RuntimeException my_RuntimeException;
        typedef ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > my_XResultSetRef;
        using ::com::sun::star::uno::Any;
        using ::rtl::OUString;

        //**************************************************************
        //************ Class: ODatabaseMetaData
        //**************************************************************

        typedef ::cppu::WeakImplHelper1< ::com::sun::star::sdbc::XDatabaseMetaData> ODatabaseMetaData_BASE;

        class ODatabaseMetaData : public ODatabaseMetaData_BASE
        {
            OConnection&    m_rConnection;
            sal_Bool        m_bUseCatalog;
        protected:
            sql::DatabaseMetaData * meta;
            OUString        identifier_quote_string;
            bool            identifier_quote_string_set;

        private:
            OUString impl_getStringMetaData( const sal_Char* _methodName, const std::string& (sql::DatabaseMetaData::*_Method)() );
            OUString impl_getStringMetaData( const sal_Char* _methodName, std::string (sql::DatabaseMetaData::*_Method)() );
            OUString impl_getStringMetaData( const sal_Char* _methodName, const sql::SQLString& (sql::DatabaseMetaData::*_Method)() );
            OUString impl_getStringMetaData( const sal_Char* _methodName, sql::SQLString (sql::DatabaseMetaData::*_Method)() );
            sal_Int32 impl_getInt32MetaData( const sal_Char* _methodName, unsigned int (sql::DatabaseMetaData::*_Method)() );
            sal_Bool impl_getBoolMetaData( const sal_Char* _methodName, bool (sql::DatabaseMetaData::*_Method)() );
            sal_Bool impl_getBoolMetaData( const sal_Char* _methodName, bool (sql::DatabaseMetaData::*_Method)(int), sal_Int32 _arg );
            sal_Bool impl_getRSTypeMetaData( const sal_Char* _methodName, bool (sql::DatabaseMetaData::*_Method)(int), sal_Int32 _resultSetType );

        public:
            inline const OConnection& getOwnConnection() const { return m_rConnection; }

            ODatabaseMetaData(OConnection& _rCon);
            virtual ~ODatabaseMetaData();

            // as I mentioned before this interface is really BIG
            // XDatabaseMetaData
            sal_Bool SAL_CALL allProceduresAreCallable()            throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL allTablesAreSelectable()              throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getURL()                      throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getUserName()                 throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL isReadOnly()                          throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL nullsAreSortedHigh()                  throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL nullsAreSortedLow()                   throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL nullsAreSortedAtStart()               throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL nullsAreSortedAtEnd()                 throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getDatabaseProductName()      throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getDatabaseProductVersion()   throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getDriverName()               throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getDriverVersion()                throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getDriverMajorVersion()              throw(my_RuntimeException);
            sal_Int32 SAL_CALL getDriverMinorVersion()              throw(my_RuntimeException);
            sal_Bool SAL_CALL usesLocalFiles()                      throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL usesLocalFilePerTable()               throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsMixedCaseIdentifiers()        throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL storesUpperCaseIdentifiers()          throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL storesLowerCaseIdentifiers()          throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL storesMixedCaseIdentifiers()          throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsMixedCaseQuotedIdentifiers()  throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL storesUpperCaseQuotedIdentifiers()    throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL storesLowerCaseQuotedIdentifiers()    throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL storesMixedCaseQuotedIdentifiers()    throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getIdentifierQuoteString()    throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getSQLKeywords()              throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getNumericFunctions()         throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getStringFunctions()          throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getSystemFunctions()          throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getTimeDateFunctions()            throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getSearchStringEscape()       throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getExtraNameCharacters()      throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsAlterTableWithAddColumn()     throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsAlterTableWithDropColumn()    throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsColumnAliasing()              throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL nullPlusNonNullIsNull()               throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsTypeConversion()              throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsConvert(sal_Int32 fromType, sal_Int32 toType)
                                                                    throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsTableCorrelationNames()       throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsDifferentTableCorrelationNames() throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsExpressionsInOrderBy()        throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsOrderByUnrelated()            throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsGroupBy()                     throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsGroupByUnrelated()            throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsGroupByBeyondSelect()         throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsLikeEscapeClause()            throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsMultipleResultSets()          throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsMultipleTransactions()        throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsNonNullableColumns()          throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsMinimumSQLGrammar()           throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsCoreSQLGrammar()              throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsExtendedSQLGrammar()          throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsANSI92EntryLevelSQL()         throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsANSI92IntermediateSQL()       throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsANSI92FullSQL()               throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsIntegrityEnhancementFacility()throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsOuterJoins()                  throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsFullOuterJoins()              throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsLimitedOuterJoins()           throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getSchemaTerm()               throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getProcedureTerm()                throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getCatalogTerm()              throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL isCatalogAtStart()                    throw(my_SQLException, my_RuntimeException);
            OUString SAL_CALL getCatalogSeparator()         throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsSchemasInDataManipulation()   throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsSchemasInProcedureCalls()     throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsSchemasInTableDefinitions()   throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsSchemasInIndexDefinitions()   throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsSchemasInPrivilegeDefinitions() throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsCatalogsInDataManipulation()  throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsCatalogsInProcedureCalls()    throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsCatalogsInTableDefinitions()  throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsCatalogsInIndexDefinitions()  throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsCatalogsInPrivilegeDefinitions() throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsPositionedDelete()            throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsPositionedUpdate()            throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsSelectForUpdate()             throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsStoredProcedures()            throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsSubqueriesInComparisons()     throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsSubqueriesInExists()          throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsSubqueriesInIns()             throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsSubqueriesInQuantifieds()     throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsCorrelatedSubqueries()        throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsUnion()                       throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsUnionAll()                    throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsOpenCursorsAcrossCommit()     throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsOpenCursorsAcrossRollback()   throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsOpenStatementsAcrossCommit()  throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsOpenStatementsAcrossRollback() throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxBinaryLiteralLength()          throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxCharLiteralLength()            throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxColumnNameLength()             throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxColumnsInGroupBy()             throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxColumnsInIndex()               throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxColumnsInOrderBy()             throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxColumnsInSelect()              throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxColumnsInTable()               throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxConnections()                  throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxCursorNameLength()             throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxIndexLength()                  throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxSchemaNameLength()             throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxProcedureNameLength()          throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxCatalogNameLength()            throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxRowSize()                      throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL doesMaxRowSizeIncludeBlobs()          throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxStatementLength()              throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxStatements()                   throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxTableNameLength()              throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxTablesInSelect()               throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getMaxUserNameLength()               throw(my_SQLException, my_RuntimeException);
            sal_Int32 SAL_CALL getDefaultTransactionIsolation()     throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsTransactions()                throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsTransactionIsolationLevel(sal_Int32 level)
                                                                    throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsDataDefinitionAndDataManipulationTransactions()
                                                                    throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsDataManipulationTransactionsOnly()throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL dataDefinitionCausesTransactionCommit()   throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL dataDefinitionIgnoredInTransactions()     throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getProcedures(const Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getProcedureColumns(const Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern, const OUString& columnNamePattern) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getTables(const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const ::com::sun::star::uno::Sequence< OUString >& types) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getSchemas() throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getCatalogs() throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getTableTypes() throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getColumns(const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getColumnPrivileges(const Any& catalog, const OUString& schema, const OUString& table, const OUString& columnNamePattern) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getTablePrivileges(const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getBestRowIdentifier(const Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope, sal_Bool nullable) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getVersionColumns(const Any& catalog, const OUString& schema, const OUString& table) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getPrimaryKeys(const Any& catalog, const OUString& schema, const OUString& table) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getImportedKeys(const Any& catalog, const OUString& schema, const OUString& table) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getExportedKeys(const Any& catalog, const OUString& schema, const OUString& table) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getCrossReference(const Any& primaryCatalog, const OUString& primarySchema, const OUString& primaryTable, const Any& foreignCatalog, const OUString& foreignSchema, const OUString& foreignTable) throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getTypeInfo() throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getIndexInfo(const Any& catalog, const OUString& schema, const OUString& table, sal_Bool unique, sal_Bool approximate) throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsResultSetType(sal_Int32 setType) throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsResultSetConcurrency(sal_Int32 setType, sal_Int32 concurrency) throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL ownUpdatesAreVisible(sal_Int32 setType)   throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL ownDeletesAreVisible(sal_Int32 setType)   throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL ownInsertsAreVisible(sal_Int32 setType)   throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL othersUpdatesAreVisible(sal_Int32 setType)throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL othersDeletesAreVisible(sal_Int32 setType)throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL othersInsertsAreVisible(sal_Int32 setType)throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL updatesAreDetected(sal_Int32 setType)     throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL deletesAreDetected(sal_Int32 setType)     throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL insertsAreDetected(sal_Int32 setType)     throw(my_SQLException, my_RuntimeException);
            sal_Bool SAL_CALL supportsBatchUpdates() throw(my_SQLException, my_RuntimeException);
            my_XResultSetRef SAL_CALL getUDTs(const Any& catalog, const OUString& schemaPattern, const OUString& typeNamePattern, const ::com::sun::star::uno::Sequence< sal_Int32 >& types) throw(my_SQLException, my_RuntimeException);
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection() throw(my_SQLException, my_RuntimeException);
        };
    }
}

#endif // CONNECTIVITY_SDATABASEMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
