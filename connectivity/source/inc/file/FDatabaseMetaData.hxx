/*************************************************************************
 *
 *  $RCSfile: FDatabaseMetaData.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:26 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_FILE_ODATABASEMETADATA_HXX_
#define _CONNECTIVITY_FILE_ODATABASEMETADATA_HXX_

#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CONNECTIVITY_FILE_OCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif
namespace connectivity
{
    namespace file
    {
        //**************************************************************
        //************ Class: java.sql.DatabaseMetaDataDate
        //**************************************************************

        class ODatabaseMetaData :   public  OBaseMutex,
                                                                        public ::cppu::WeakImplHelper1< ::com::sun::star::sdbc::XDatabaseMetaData >
        {
        protected:
            OConnection*    m_pConnection;
        public:

            ODatabaseMetaData(OConnection* _pCon);
            ~ODatabaseMetaData();
            // XDatabaseMetaData
                        virtual sal_Bool SAL_CALL allProceduresAreCallable(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL allTablesAreSelectable(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getURL(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getUserName(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL nullsAreSortedHigh(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL nullsAreSortedLow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL nullsAreSortedAtStart(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL nullsAreSortedAtEnd(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getDatabaseProductName(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getDatabaseProductVersion(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getDriverName(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getDriverVersion(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getDriverMajorVersion(  ) throw(::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getDriverMinorVersion(  ) throw(::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL usesLocalFiles(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL usesLocalFilePerTable(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsMixedCaseIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL storesUpperCaseIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL storesLowerCaseIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL storesMixedCaseIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsMixedCaseQuotedIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL storesUpperCaseQuotedIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL storesLowerCaseQuotedIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL storesMixedCaseQuotedIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getIdentifierQuoteString(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getSQLKeywords(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getNumericFunctions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getStringFunctions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getSystemFunctions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getTimeDateFunctions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getSearchStringEscape(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getExtraNameCharacters(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsAlterTableWithAddColumn(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsAlterTableWithDropColumn(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsColumnAliasing(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL nullPlusNonNullIsNull(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsTypeConversion(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsConvert( sal_Int32 fromType, sal_Int32 toType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsTableCorrelationNames(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsDifferentTableCorrelationNames(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsExpressionsInOrderBy(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsOrderByUnrelated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsGroupBy(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsGroupByUnrelated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsGroupByBeyondSelect(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsLikeEscapeClause(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsMultipleResultSets(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsMultipleTransactions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsNonNullableColumns(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsMinimumSQLGrammar(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsCoreSQLGrammar(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsExtendedSQLGrammar(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsANSI92EntryLevelSQL(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsANSI92IntermediateSQL(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsANSI92FullSQL(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsIntegrityEnhancementFacility(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsOuterJoins(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsFullOuterJoins(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsLimitedOuterJoins(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getSchemaTerm(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getProcedureTerm(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getCatalogTerm(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL isCatalogAtStart(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getCatalogSeparator(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsSchemasInDataManipulation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsSchemasInProcedureCalls(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsSchemasInTableDefinitions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsSchemasInIndexDefinitions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsSchemasInPrivilegeDefinitions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsCatalogsInDataManipulation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsCatalogsInProcedureCalls(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsCatalogsInTableDefinitions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsCatalogsInIndexDefinitions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsCatalogsInPrivilegeDefinitions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsPositionedDelete(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsPositionedUpdate(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsSelectForUpdate(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsStoredProcedures(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsSubqueriesInComparisons(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsSubqueriesInExists(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsSubqueriesInIns(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsSubqueriesInQuantifieds(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsCorrelatedSubqueries(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsUnion(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsUnionAll(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsOpenCursorsAcrossCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsOpenCursorsAcrossRollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsOpenStatementsAcrossCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsOpenStatementsAcrossRollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxBinaryLiteralLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxCharLiteralLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxColumnNameLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxColumnsInGroupBy(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxColumnsInIndex(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxColumnsInOrderBy(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxColumnsInSelect(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxColumnsInTable(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxConnections(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxCursorNameLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxIndexLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxSchemaNameLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxProcedureNameLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxCatalogNameLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxRowSize(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL doesMaxRowSizeIncludeBlobs(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxStatementLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxStatements(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxTableNameLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxTablesInSelect(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getMaxUserNameLength(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getDefaultTransactionIsolation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsTransactions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsTransactionIsolationLevel( sal_Int32 level ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsDataDefinitionAndDataManipulationTransactions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsDataManipulationTransactionsOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL dataDefinitionCausesTransactionCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL dataDefinitionIgnoredInTransactions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getProcedures( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& procedureNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getProcedureColumns( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& procedureNamePattern, const ::rtl::OUString& columnNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getTables( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& types ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getSchemas(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getCatalogs(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getTableTypes(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getColumns( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern, const ::rtl::OUString& columnNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getColumnPrivileges( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, const ::rtl::OUString& columnNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getTablePrivileges( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getBestRowIdentifier( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Int32 scope, sal_Bool nullable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getVersionColumns( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getPrimaryKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getImportedKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getExportedKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getCrossReference( const ::com::sun::star::uno::Any& primaryCatalog, const ::rtl::OUString& primarySchema, const ::rtl::OUString& primaryTable, const ::com::sun::star::uno::Any& foreignCatalog, const ::rtl::OUString& foreignSchema, const ::rtl::OUString& foreignTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getTypeInfo(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getIndexInfo( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Bool unique, sal_Bool approximate ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsResultSetType( sal_Int32 setType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL ownUpdatesAreVisible( sal_Int32 setType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL ownDeletesAreVisible( sal_Int32 setType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL ownInsertsAreVisible( sal_Int32 setType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL othersUpdatesAreVisible( sal_Int32 setType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL othersDeletesAreVisible( sal_Int32 setType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL othersInsertsAreVisible( sal_Int32 setType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL updatesAreDetected( sal_Int32 setType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL deletesAreDetected( sal_Int32 setType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL insertsAreDetected( sal_Int32 setType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL supportsBatchUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getUDTs( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& typeNamePattern, const ::com::sun::star::uno::Sequence< sal_Int32 >& types ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // _CONNECTIVITY_FILE_ODATABASEMETADATA_HXX_

