/*************************************************************************
 *
 *  $RCSfile: ODatabaseMetaData.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:23 $
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

#ifndef _CONNECTIVITY_ODBC_ODATABASEMETADATA_HXX_
#include "odbc/ODatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_ORESULTSET_HXX_
#include "odbc/OResultSet.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _CONNECTIVITY_OFUNCTIONDEFS_HXX_
#include "odbc/OFunctiondefs.hxx"
#endif


using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

ODatabaseMetaData::ODatabaseMetaData(const SQLHANDLE _pHandle,OConnection* _pCon)
                        :   m_aConnectionHandle(_pHandle),m_pConnection(_pCon)
{
    m_pConnection->acquire();
}
// -------------------------------------------------------------------------
ODatabaseMetaData::~ODatabaseMetaData()
{
    if (m_pConnection)
        m_pConnection->release();
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTypeInfo(  ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openTypeInfo();
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCatalogs(  ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openCatalogs();
    return xRef;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogSeparator(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    OTools::GetInfo(m_aConnectionHandle,SQL_CATALOG_NAME_SEPARATOR,aVal,*this);

    return aVal;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas(  ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openSchemas();
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
    const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openColumnPrivileges(catalog,schema,table,columnNamePattern);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
    const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern,
    const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openColumns(catalog,schemaPattern,tableNamePattern,columnNamePattern);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& tableNamePattern, const Sequence< ::rtl::OUString >& types ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openTables(catalog,schemaPattern,tableNamePattern,types);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& procedureNamePattern, const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openProcedureColumns(catalog,schemaPattern,procedureNamePattern,columnNamePattern);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& procedureNamePattern ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openProcedures(catalog,schemaPattern,procedureNamePattern);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getVersionColumns(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openVersionColumns(catalog,schema,table);
    return xRef;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_BINARY_LITERAL_LEN,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_ROW_SIZE,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_CATALOG_NAME_LEN,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_CHAR_LITERAL_LEN,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_COLUMN_NAME_LEN,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_COLUMNS_IN_INDEX,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_CURSOR_NAME_LEN,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_ACTIVE_CONNECTIONS,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_COLUMNS_IN_TABLE,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_STATEMENT_LEN,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_TABLE_NAME_LEN,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTablesInSelect(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_TABLES_IN_SELECT,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openExportedKeys(catalog,schema,table);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openImportedKeys(catalog,schema,table);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openPrimaryKeys(catalog,schema,table);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
    sal_Bool unique, sal_Bool approximate ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openIndexInfo(catalog,schema,table,unique,approximate);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getBestRowIdentifier(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Int32 scope,
    sal_Bool nullable ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openBestRowIdentifier(catalog,schema,table,scope,nullable);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
    const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openTablePrivileges(catalog,schemaPattern,tableNamePattern);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
    const Any& primaryCatalog, const ::rtl::OUString& primarySchema,
    const ::rtl::OUString& primaryTable, const Any& foreignCatalog,
    const ::rtl::OUString& foreignSchema, const ::rtl::OUString& foreignTable ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openForeignKeys(primaryCatalog,primarySchema.toChar() == '%' ? &primarySchema : NULL,&primaryTable,
        foreignCatalog, foreignSchema.toChar() == '%' ? &foreignSchema : NULL,&foreignTable);
    return xRef;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_ROW_SIZE_INCLUDES_LONG,aVal,*this);
    return aVal.toChar() == 'Y';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_QUOTED_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_LOWER;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_LOWER;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_QUOTED_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_MIXED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_MIXED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_QUOTED_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_UPPER;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_UPPER;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithAddColumn(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_ALTER_TABLE,nValue,*this);
    return nValue == SQL_AT_ADD_COLUMN;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithDropColumn(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_ALTER_TABLE,nValue,*this);
    return nValue == SQL_AT_DROP_COLUMN;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_INDEX_SIZE,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_NON_NULLABLE_COLUMNS,nValue,*this);
    return nValue == SQL_NNC_NON_NULL;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    OTools::GetInfo(m_aConnectionHandle,SQL_CATALOG_TERM,aVal,*this);
    return aVal;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getIdentifierQuoteString(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    OTools::GetInfo(m_aConnectionHandle,SQL_IDENTIFIER_QUOTE_CHAR,aVal,*this);
    return aVal;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    OTools::GetInfo(m_aConnectionHandle,SQL_SPECIAL_CHARACTERS,aVal,*this);
    return aVal;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
    return nValue != SQL_CN_NONE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::isCatalogAtStart(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CATALOG_LOCATION,nValue,*this);
    return nValue == SQL_CL_START;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_TXN_CAPABLE,nValue,*this);
    return nValue == SQL_TC_DDL_IGNORE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_TXN_CAPABLE,nValue,*this);
    return nValue == SQL_TC_DDL_COMMIT;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_TXN_CAPABLE,nValue,*this);
    return nValue == SQL_TC_DML;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_TXN_CAPABLE,nValue,*this);
    return nValue == SQL_TC_ALL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DYNAMIC_CURSOR_ATTRIBUTES1,nValue,*this);
    return (nValue & SQL_CA1_POS_DELETE) == SQL_CA1_POS_DELETE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DYNAMIC_CURSOR_ATTRIBUTES1,nValue,*this);
    return (nValue & SQL_CA1_POS_UPDATE) == SQL_CA1_POS_UPDATE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CURSOR_ROLLBACK_BEHAVIOR,nValue,*this);
    return nValue == SQL_CB_PRESERVE || nValue == SQL_CB_CLOSE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CURSOR_COMMIT_BEHAVIOR,nValue,*this);
    return nValue == SQL_CB_PRESERVE || nValue == SQL_CB_CLOSE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CURSOR_COMMIT_BEHAVIOR,nValue,*this);
    return nValue == SQL_CB_PRESERVE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CURSOR_ROLLBACK_BEHAVIOR,nValue,*this);
    return nValue == SQL_CB_PRESERVE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_TXN_ISOLATION_OPTION,nValue,*this);
    return (nValue & level) == level;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInDataManipulation(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_DML_STATEMENTS) == SQL_SU_DML_STATEMENTS;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SQL_CONFORMANCE,nValue,*this);
    return nValue == SQL_SC_SQL92_FULL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SQL_CONFORMANCE,nValue,*this);
    return nValue == SQL_SC_SQL92_ENTRY;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aStr;
    OTools::GetInfo(m_aConnectionHandle,SQL_INTEGRITY,aStr,*this);
    return aStr.toChar() == 'Y';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_INDEX_DEFINITION) == SQL_SU_INDEX_DEFINITION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInTableDefinitions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_TABLE_DEFINITION) == SQL_SU_TABLE_DEFINITION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInTableDefinitions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return (nValue & SQL_CU_TABLE_DEFINITION) == SQL_CU_TABLE_DEFINITION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return (nValue & SQL_CU_INDEX_DEFINITION) == SQL_CU_INDEX_DEFINITION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInDataManipulation(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return (nValue & SQL_CU_DML_STATEMENTS) == SQL_CU_DML_STATEMENTS;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_OJ_CAPABILITIES,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    SQLHANDLE hStmt;
    N3SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->getConnection(),&hStmt);

    OResultSet* pResult = new OResultSet(hStmt);
    Reference< XResultSet > xRef = pResult;
    pResult->openTablesTypes();
    return xRef;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatements(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_CONCURRENT_ACTIVITIES,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_PROCEDURE_NAME_LEN,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_SCHEMA_NAME_LEN,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_TXN_CAPABLE,nValue,*this);
    return nValue != SQL_TC_NONE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_ACCESSIBLE_PROCEDURES,aValue,*this);
    return aValue.toChar() == 'Y';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_PROCEDURES,aValue,*this);
    return aValue.toChar() == 'Y';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DYNAMIC_CURSOR_ATTRIBUTES1,nValue,*this);
    return (nValue & SQL_CA1_POSITIONED_UPDATE) == SQL_CA1_POSITIONED_UPDATE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_ACCESSIBLE_TABLES,aValue,*this);
    return aValue.toChar() == 'Y';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    sal_Bool bValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DATA_SOURCE_READ_ONLY,bValue,*this);
    return bValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_FILE_USAGE,nValue,*this);
    return nValue == SQL_FILE_CATALOG;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_FILE_USAGE,nValue,*this);
    return nValue == SQL_FILE_TABLE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_FUNCTIONS,nValue,*this);
    return (nValue & SQL_FN_CVT_CONVERT) == SQL_FN_CVT_CONVERT;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CONCAT_NULL_BEHAVIOR,nValue,*this);
    return nValue == SQL_CB_NULL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_COLUMN_ALIAS,aValue,*this);
    return aValue.toChar() == 'Y';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
    return nValue != SQL_CN_NONE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 fromType, sal_Int32 toType ) throw(SQLException, RuntimeException)
{
    if(fromType == toType)
        return sal_True;

    sal_Int32 nValue=0;
    switch(fromType)
    {
        case DataType::BIT:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_BIT,nValue,*this);
            break;
        case DataType::TINYINT:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_TINYINT,nValue,*this);
            break;
        case DataType::SMALLINT:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_SMALLINT,nValue,*this);
            break;
        case DataType::INTEGER:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_INTEGER,nValue,*this);
            break;
        case DataType::BIGINT:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_BIGINT,nValue,*this);
            break;
        case DataType::FLOAT:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_FLOAT,nValue,*this);
            break;
        case DataType::REAL:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_REAL,nValue,*this);
            break;
        case DataType::DOUBLE:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_DOUBLE,nValue,*this);
            break;
        case DataType::NUMERIC:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_NUMERIC,nValue,*this);
            break;
        case DataType::DECIMAL:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_DECIMAL,nValue,*this);
            break;
        case DataType::CHAR:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_CHAR,nValue,*this);
            break;
        case DataType::VARCHAR:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_VARCHAR,nValue,*this);
            break;
        case DataType::LONGVARCHAR:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_LONGVARCHAR,nValue,*this);
            break;
        case DataType::DATE:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_DATE,nValue,*this);
            break;
        case DataType::TIME:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_TIME,nValue,*this);
            break;
        case DataType::TIMESTAMP:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_TIMESTAMP,nValue,*this);
            break;
        case DataType::BINARY:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_BINARY,nValue,*this);
            break;
        case DataType::VARBINARY:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_VARBINARY,nValue,*this);
            break;
        case DataType::LONGVARBINARY:
            OTools::GetInfo(m_aConnectionHandle,SQL_CONVERT_LONGVARBINARY,nValue,*this);
            break;
        case DataType::SQLNULL:
            //  OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::OTHER:
            //  OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::OBJECT:
            //  OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::DISTINCT:
            //  OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::STRUCT:
            //  OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::ARRAY:
            //  OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::BLOB:
            //  OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::CLOB:
            //  OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::REF:
            //  OTools::GetInfo(m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
    }
    sal_Bool bConvert = sal_False;
    switch(toType)
    {
        case DataType::BIT:
            bConvert = (nValue & SQL_CVT_BIT) == SQL_CVT_BIT;
            break;
        case DataType::TINYINT:
            bConvert = (nValue & SQL_CVT_TINYINT) == SQL_CVT_TINYINT;
            break;
        case DataType::SMALLINT:
            bConvert = (nValue & SQL_CVT_SMALLINT) == SQL_CVT_SMALLINT;
            break;
        case DataType::INTEGER:
            bConvert = (nValue & SQL_CVT_INTEGER) == SQL_CVT_INTEGER;
            break;
        case DataType::BIGINT:
            bConvert = (nValue & SQL_CVT_BIGINT) == SQL_CVT_BIGINT;
            break;
        case DataType::FLOAT:
            bConvert = (nValue & SQL_CVT_FLOAT) == SQL_CVT_FLOAT;
            break;
        case DataType::REAL:
            bConvert = (nValue & SQL_CVT_REAL) == SQL_CVT_REAL;
            break;
        case DataType::DOUBLE:
            bConvert = (nValue & SQL_CVT_DOUBLE) == SQL_CVT_DOUBLE;
            break;
        case DataType::NUMERIC:
            bConvert = (nValue & SQL_CVT_NUMERIC) == SQL_CVT_NUMERIC;
            break;
        case DataType::DECIMAL:
            bConvert = (nValue & SQL_CVT_DECIMAL) == SQL_CVT_DECIMAL;
            break;
        case DataType::CHAR:
            bConvert = (nValue & SQL_CVT_CHAR) == SQL_CVT_CHAR;
            break;
        case DataType::VARCHAR:
            bConvert = (nValue & SQL_CVT_VARCHAR) == SQL_CVT_VARCHAR;
            break;
        case DataType::LONGVARCHAR:
            bConvert = (nValue & SQL_CVT_LONGVARCHAR) == SQL_CVT_LONGVARCHAR;
            break;
        case DataType::DATE:
            bConvert = (nValue & SQL_CVT_DATE) == SQL_CVT_DATE;
            break;
        case DataType::TIME:
            bConvert = (nValue & SQL_CVT_TIME) == SQL_CVT_TIME;
            break;
        case DataType::TIMESTAMP:
            bConvert = (nValue & SQL_CVT_TIMESTAMP) == SQL_CVT_TIMESTAMP;
            break;
        case DataType::BINARY:
            bConvert = (nValue & SQL_CVT_BINARY) == SQL_CVT_BINARY;
            break;
        case DataType::VARBINARY:
            bConvert = (nValue & SQL_CVT_VARBINARY) == SQL_CVT_VARBINARY;
            break;
        case DataType::LONGVARBINARY:
            bConvert = (nValue & SQL_CVT_LONGVARBINARY) == SQL_CVT_LONGVARBINARY;
            break;
    }

    return bConvert;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_EXPRESSIONS_IN_ORDERBY,aValue,*this);
    return aValue.toChar() == 'Y';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_GROUP_BY,nValue,*this);
    return nValue != SQL_GB_NOT_SUPPORTED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_GROUP_BY,nValue,*this);
    return nValue != SQL_GB_GROUP_BY_CONTAINS_SELECT;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_GROUP_BY,nValue,*this);
    return nValue == SQL_GB_NO_RELATION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MULTIPLE_ACTIVE_TXN,aValue,*this);
    return aValue.toChar() == 'Y';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MULT_RESULT_SETS,aValue,*this);
    return aValue.toChar() == 'Y';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_LIKE_ESCAPE_CLAUSE,aValue,*this);
    return aValue.toChar() == 'Y';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_ORDER_BY_COLUMNS_IN_SELECT,aValue,*this);
    return aValue.toChar() == 'N';
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_UNION,nValue,*this);
    return (nValue & SQL_U_UNION) == SQL_U_UNION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_UNION,nValue,*this);
    return (nValue & SQL_U_UNION_ALL) == SQL_U_UNION_ALL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_MIXED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_QUOTED_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_MIXED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_NULL_COLLATION,nValue,*this);
    return nValue == SQL_NC_END;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_NULL_COLLATION,nValue,*this);
    return nValue == SQL_NC_START;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_NULL_COLLATION,nValue,*this);
    return nValue == SQL_NC_HIGH;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_NULL_COLLATION,nValue,*this);
    return nValue == SQL_NC_LOW;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return nValue & SQL_SU_PROCEDURE_INVOCATION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return nValue & SQL_SU_PRIVILEGE_DEFINITION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return nValue & SQL_CU_PROCEDURE_INVOCATION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return nValue & SQL_CU_PRIVILEGE_DEFINITION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return (nValue & SQL_SQ_CORRELATED_SUBQUERIES) == SQL_SQ_CORRELATED_SUBQUERIES;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return (nValue & SQL_SQ_COMPARISON) == SQL_SQ_COMPARISON;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return (nValue & SQL_SQ_EXISTS) == SQL_SQ_EXISTS;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return (nValue & SQL_SQ_IN) == SQL_SQ_IN;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return (nValue & SQL_SQ_QUANTIFIED) == SQL_SQ_QUANTIFIED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SQL_CONFORMANCE,nValue,*this);
    return nValue == SQL_SC_SQL92_INTERMEDIATE;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue = ::rtl::OUString::createFromAscii("sdbc:odbc:");
    OTools::GetInfo(m_aConnectionHandle,SQL_DATA_SOURCE_NAME,aValue,*this);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_USER_NAME,aValue,*this);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DRIVER_NAME,aValue,*this);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDriverVersion(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DRIVER_ODBC_VER,aValue,*this);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DRIVER_VER,aValue,*this);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DBMS_NAME,aValue,*this);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_PROCEDURE_TERM,aValue,*this);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SCHEMA_TERM,aValue,*this);
    return aValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DRIVER_VER,aValue,*this);
    return aValue.copy(0,aValue.indexOf('.')).toInt32();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_DRIVER_VER,aValue,*this);
    return aValue.copy(0,aValue.lastIndexOf('.')).toInt32();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_KEYWORDS,aValue,*this);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SEARCH_PATTERN_ESCAPE,aValue,*this);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_STRING_FUNCTIONS,nValue,*this);
    if(nValue & SQL_FN_STR_ASCII)
        aValue = ::rtl::OUString::createFromAscii("ASCII,");
    if(nValue & SQL_FN_STR_BIT_LENGTH)
        aValue += ::rtl::OUString::createFromAscii("BIT_LENGTH,");
    if(nValue & SQL_FN_STR_CHAR)
        aValue += ::rtl::OUString::createFromAscii("CHAR,");
    if(nValue & SQL_FN_STR_CHAR_LENGTH)
        aValue += ::rtl::OUString::createFromAscii("CHAR_LENGTH,");
    if(nValue & SQL_FN_STR_CHARACTER_LENGTH)
        aValue += ::rtl::OUString::createFromAscii("CHARACTER_LENGTH,");
    if(nValue & SQL_FN_STR_CONCAT)
        aValue += ::rtl::OUString::createFromAscii("CONCAT,");
    if(nValue & SQL_FN_STR_DIFFERENCE)
        aValue += ::rtl::OUString::createFromAscii("DIFFERENCE,");
    if(nValue & SQL_FN_STR_INSERT)
        aValue += ::rtl::OUString::createFromAscii("INSERT,");
    if(nValue & SQL_FN_STR_LCASE)
        aValue += ::rtl::OUString::createFromAscii("LCASE,");
    if(nValue & SQL_FN_STR_LEFT)
        aValue += ::rtl::OUString::createFromAscii("LEFT,");
    if(nValue & SQL_FN_STR_LENGTH)
        aValue += ::rtl::OUString::createFromAscii("LENGTH,");
    if(nValue & SQL_FN_STR_LOCATE)
        aValue += ::rtl::OUString::createFromAscii("LOCATE,");
    if(nValue & SQL_FN_STR_LOCATE_2)
        aValue += ::rtl::OUString::createFromAscii("LOCATE_2,");
    if(nValue & SQL_FN_STR_LTRIM)
        aValue += ::rtl::OUString::createFromAscii("LTRIM,");
    if(nValue & SQL_FN_STR_OCTET_LENGTH)
        aValue += ::rtl::OUString::createFromAscii("OCTET_LENGTH,");
    if(nValue & SQL_FN_STR_POSITION)
        aValue += ::rtl::OUString::createFromAscii("POSITION,");
    if(nValue & SQL_FN_STR_REPEAT)
        aValue += ::rtl::OUString::createFromAscii("REPEAT,");
    if(nValue & SQL_FN_STR_REPLACE)
        aValue += ::rtl::OUString::createFromAscii("REPLACE,");
    if(nValue & SQL_FN_STR_RIGHT)
        aValue += ::rtl::OUString::createFromAscii("RIGHT,");
    if(nValue & SQL_FN_STR_RTRIM)
        aValue += ::rtl::OUString::createFromAscii("RTRIM,");
    if(nValue & SQL_FN_STR_SOUNDEX)
        aValue += ::rtl::OUString::createFromAscii("SOUNDEX,");
    if(nValue & SQL_FN_STR_SPACE)
        aValue += ::rtl::OUString::createFromAscii("SPACE,");
    if(nValue & SQL_FN_STR_SUBSTRING)
        aValue += ::rtl::OUString::createFromAscii("SUBSTRING,");
    if(nValue & SQL_FN_STR_UCASE)
        aValue += ::rtl::OUString::createFromAscii("UCASE,");


    return aValue.copy(0,aValue.lastIndexOf(','));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_TIMEDATE_FUNCTIONS,nValue,*this);

    if(nValue & SQL_FN_TD_CURRENT_DATE)
        aValue = ::rtl::OUString::createFromAscii("CURRENT_DATE,");
    if(nValue & SQL_FN_TD_CURRENT_TIME)
        aValue += ::rtl::OUString::createFromAscii("CURRENT_TIME,");
    if(nValue & SQL_FN_TD_CURRENT_TIMESTAMP)
        aValue += ::rtl::OUString::createFromAscii("CURRENT_TIMESTAMP,");
    if(nValue & SQL_FN_TD_CURDATE)
        aValue += ::rtl::OUString::createFromAscii("CURDATE,");
    if(nValue & SQL_FN_TD_CURTIME)
        aValue += ::rtl::OUString::createFromAscii("CURTIME,");
    if(nValue & SQL_FN_TD_DAYNAME)
        aValue += ::rtl::OUString::createFromAscii("DAYNAME,");
    if(nValue & SQL_FN_TD_DAYOFMONTH)
        aValue += ::rtl::OUString::createFromAscii("DAYOFMONTH,");
    if(nValue & SQL_FN_TD_DAYOFWEEK)
        aValue += ::rtl::OUString::createFromAscii("DAYOFWEEK,");
    if(nValue & SQL_FN_TD_DAYOFYEAR)
        aValue += ::rtl::OUString::createFromAscii("DAYOFYEAR,");
    if(nValue & SQL_FN_TD_EXTRACT)
        aValue += ::rtl::OUString::createFromAscii("EXTRACT,");
    if(nValue & SQL_FN_TD_HOUR)
        aValue += ::rtl::OUString::createFromAscii("HOUR,");
    if(nValue & SQL_FN_TD_MINUTE)
        aValue += ::rtl::OUString::createFromAscii("MINUTE,");
    if(nValue & SQL_FN_TD_MONTH)
        aValue += ::rtl::OUString::createFromAscii("MONTH,");
    if(nValue & SQL_FN_TD_MONTHNAME)
        aValue += ::rtl::OUString::createFromAscii("MONTHNAME,");
    if(nValue & SQL_FN_TD_NOW)
        aValue += ::rtl::OUString::createFromAscii("NOW,");
    if(nValue & SQL_FN_TD_QUARTER)
        aValue += ::rtl::OUString::createFromAscii("QUARTER,");
    if(nValue & SQL_FN_TD_SECOND)
        aValue += ::rtl::OUString::createFromAscii("SECOND,");
    if(nValue & SQL_FN_TD_TIMESTAMPADD)
        aValue += ::rtl::OUString::createFromAscii("TIMESTAMPADD,");
    if(nValue & SQL_FN_TD_TIMESTAMPDIFF)
        aValue += ::rtl::OUString::createFromAscii("TIMESTAMPDIFF,");
    if(nValue & SQL_FN_TD_WEEK)
        aValue += ::rtl::OUString::createFromAscii("WEEK,");
    if(nValue & SQL_FN_TD_YEAR)
        aValue += ::rtl::OUString::createFromAscii("YEAR,");

    return aValue.copy(0,aValue.lastIndexOf(','));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_SYSTEM_FUNCTIONS,nValue,*this);

    if(nValue & SQL_FN_SYS_DBNAME)
        aValue += ::rtl::OUString::createFromAscii("DBNAME,");
    if(nValue & SQL_FN_SYS_IFNULL)
        aValue += ::rtl::OUString::createFromAscii("IFNULL,");
    if(nValue & SQL_FN_SYS_USERNAME)
        aValue += ::rtl::OUString::createFromAscii("USERNAME,");

    return aValue.copy(0,aValue.lastIndexOf(','));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    ::rtl::OUString aValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_NUMERIC_FUNCTIONS,nValue,*this);

    if(nValue & SQL_FN_NUM_ABS)
        aValue += ::rtl::OUString::createFromAscii("ABS,");
    if(nValue & SQL_FN_NUM_ACOS)
        aValue += ::rtl::OUString::createFromAscii("ACOS,");
    if(nValue & SQL_FN_NUM_ASIN)
        aValue += ::rtl::OUString::createFromAscii("ASIN,");
    if(nValue & SQL_FN_NUM_ATAN)
        aValue += ::rtl::OUString::createFromAscii("ATAN,");
    if(nValue & SQL_FN_NUM_ATAN2)
        aValue += ::rtl::OUString::createFromAscii("ATAN2,");
    if(nValue & SQL_FN_NUM_CEILING)
        aValue += ::rtl::OUString::createFromAscii("CEILING,");
    if(nValue & SQL_FN_NUM_COS)
        aValue += ::rtl::OUString::createFromAscii("COS,");
    if(nValue & SQL_FN_NUM_COT)
        aValue += ::rtl::OUString::createFromAscii("COT,");
    if(nValue & SQL_FN_NUM_DEGREES)
        aValue += ::rtl::OUString::createFromAscii("DEGREES,");
    if(nValue & SQL_FN_NUM_EXP)
        aValue += ::rtl::OUString::createFromAscii("EXP,");
    if(nValue & SQL_FN_NUM_FLOOR)
        aValue += ::rtl::OUString::createFromAscii("FLOOR,");
    if(nValue & SQL_FN_NUM_LOG)
        aValue += ::rtl::OUString::createFromAscii("LOGF,");
    if(nValue & SQL_FN_NUM_LOG10)
        aValue += ::rtl::OUString::createFromAscii("LOG10,");
    if(nValue & SQL_FN_NUM_MOD)
        aValue += ::rtl::OUString::createFromAscii("MOD,");
    if(nValue & SQL_FN_NUM_PI)
        aValue += ::rtl::OUString::createFromAscii("PI,");
    if(nValue & SQL_FN_NUM_POWER)
        aValue += ::rtl::OUString::createFromAscii("POWER,");
    if(nValue & SQL_FN_NUM_RADIANS)
        aValue += ::rtl::OUString::createFromAscii("RADIANS,");
    if(nValue & SQL_FN_NUM_RAND)
        aValue += ::rtl::OUString::createFromAscii("RAND,");
    if(nValue & SQL_FN_NUM_ROUND)
        aValue += ::rtl::OUString::createFromAscii("ROUND,");
    if(nValue & SQL_FN_NUM_SIGN)
        aValue += ::rtl::OUString::createFromAscii("SIGN,");
    if(nValue & SQL_FN_NUM_SIN)
        aValue += ::rtl::OUString::createFromAscii("SIN,");
    if(nValue & SQL_FN_NUM_SQRT)
        aValue += ::rtl::OUString::createFromAscii("SQRT,");
    if(nValue & SQL_FN_NUM_TAN)
        aValue += ::rtl::OUString::createFromAscii("TAN,");
    if(nValue & SQL_FN_NUM_TRUNCATE)
        aValue += ::rtl::OUString::createFromAscii("TRUNCATE,");


    return aValue.copy(0,aValue.lastIndexOf(','));
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_ODBC_INTERFACE_CONFORMANCE,nValue,*this);
    return nValue == SQL_OIC_LEVEL2;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_ODBC_INTERFACE_CONFORMANCE,nValue,*this);
    return nValue == SQL_OIC_CORE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_ODBC_INTERFACE_CONFORMANCE,nValue,*this);
    return nValue == SQL_OIC_LEVEL1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_OJ_CAPABILITIES,nValue,*this);
    return (nValue & SQL_OJ_FULL) == SQL_OJ_FULL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return supportsFullOuterJoins(  );
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_COLUMNS_IN_GROUP_BY,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_COLUMNS_IN_ORDER_BY,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_COLUMNS_IN_SELECT,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_MAX_USER_NAME_LEN,nValue,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    OTools::GetInfo(m_aConnectionHandle,SQL_CURSOR_SENSITIVITY,nValue,*this);
    return (nValue & setType) == setType;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    SQLUSMALLINT nAskFor;
    switch(setType)
    {
        case ResultSetType::FORWARD_ONLY:
            nAskFor = SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2;
            break;
        case ResultSetType::SCROLL_INSENSITIVE:
            nAskFor = SQL_STATIC_CURSOR_ATTRIBUTES2;
            break;
        case ResultSetType::SCROLL_SENSITIVE:
            nAskFor = SQL_DYNAMIC_CURSOR_ATTRIBUTES2;
            break;
    }

    OTools::GetInfo(m_aConnectionHandle,nAskFor,nValue,*this);
    sal_Bool bRet = sal_False;
    switch(concurrency)
    {
        case ResultSetConcurrency::READ_ONLY:
            bRet = (nValue & SQL_CA2_READ_ONLY_CONCURRENCY) == SQL_CA2_READ_ONLY_CONCURRENCY;
            break;
        case ResultSetConcurrency::UPDATABLE:
            bRet = (nValue & SQL_CA2_OPT_VALUES_CONCURRENCY) == SQL_CA2_OPT_VALUES_CONCURRENCY;
            break;
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    SQLUSMALLINT nAskFor;
    switch(setType)
    {
        case ResultSetType::FORWARD_ONLY:
            nAskFor = SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2;
            break;
        case ResultSetType::SCROLL_INSENSITIVE:
            nAskFor = SQL_STATIC_CURSOR_ATTRIBUTES2;
            break;
        case ResultSetType::SCROLL_SENSITIVE:
            nAskFor = SQL_DYNAMIC_CURSOR_ATTRIBUTES2;
            break;
    }

    OTools::GetInfo(m_aConnectionHandle,nAskFor,nValue,*this);
    return (nValue & SQL_CA2_SENSITIVITY_UPDATES) == SQL_CA2_SENSITIVITY_UPDATES;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    SQLUSMALLINT nAskFor;
    switch(setType)
    {
        case ResultSetType::FORWARD_ONLY:
            nAskFor = SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2;
            break;
        case ResultSetType::SCROLL_INSENSITIVE:
            nAskFor = SQL_STATIC_CURSOR_ATTRIBUTES2;
            break;
        case ResultSetType::SCROLL_SENSITIVE:
            nAskFor = SQL_DYNAMIC_CURSOR_ATTRIBUTES2;
            break;
    }

    OTools::GetInfo(m_aConnectionHandle,nAskFor,nValue,*this);
    return (nValue & SQL_CA2_SENSITIVITY_DELETIONS) == SQL_CA2_SENSITIVITY_DELETIONS;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue;
    SQLUSMALLINT nAskFor;
    switch(setType)
    {
        case ResultSetType::FORWARD_ONLY:
            nAskFor = SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2;
            break;
        case ResultSetType::SCROLL_INSENSITIVE:
            nAskFor = SQL_STATIC_CURSOR_ATTRIBUTES2;
            break;
        case ResultSetType::SCROLL_SENSITIVE:
            nAskFor = SQL_DYNAMIC_CURSOR_ATTRIBUTES2;
            break;
    }

    OTools::GetInfo(m_aConnectionHandle,nAskFor,nValue,*this);
    return (nValue & SQL_CA2_SENSITIVITY_ADDITIONS) == SQL_CA2_SENSITIVITY_ADDITIONS;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ownUpdatesAreVisible(setType);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ownDeletesAreVisible(setType);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ownInsertsAreVisible(setType);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& typeNamePattern, const Sequence< sal_Int32 >& types ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODatabaseMetaData::getConnection(  ) throw(SQLException, RuntimeException)
{
    return (Reference< XConnection >)m_pConnection;//new OConnection(m_aConnectionHandle);
}
// -------------------------------------------------------------------------


