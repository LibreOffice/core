/*************************************************************************
 *
 *  $RCSfile: ADatabaseMetaData.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 15:58:09 $
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
#ifndef _CONNECTIVITY_ADO_ADATABASEMETADATA_HXX_
#include "ado/ADatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ADATABASEMETADATARESULTSET_HXX_
#include "ado/ADatabaseMetaDataResultSet.hxx"
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
#ifndef _COM_SUN_STAR_SDBC_TRANSACTIONISOLATION_HPP_
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#endif
#ifndef _CONNECTIVITY_ADO_ACONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ADOIMP_HXX_
#include "ado/adoimp.hxx"
#endif
#ifndef _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_
#include "FDatabaseMetaDataResultSet.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;


//  using namespace connectivity;

ODatabaseMetaData::ODatabaseMetaData(OConnection* _pCon)
    : ::connectivity::ODatabaseMetaDataBase(_pCon)
    ,m_pADOConnection(_pCon->getConnection())
    ,m_pConnection(_pCon)
{
}
// -------------------------------------------------------------------------
sal_Int32 ODatabaseMetaData::getInt32Property(const ::rtl::OUString& _aProperty)  throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    connectivity::ado::WpADOProperties aProps(m_pADOConnection->get_Properties());
    //  ADOS::ThrowException(*m_pADOConnection,*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");
    ADO_PROP(_aProperty);
    sal_Int32 nValue(0);
    if(!aVar.isNull() && !aVar.isEmpty())
        nValue = aVar;
    return nValue;
}

// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::getBoolProperty(const ::rtl::OUString& _aProperty)  throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    connectivity::ado::WpADOProperties aProps(m_pADOConnection->get_Properties());
    ADOS::ThrowException(*m_pADOConnection,*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");
    ADO_PROP(_aProperty);
    return (!aVar.isNull() && !aVar.isEmpty() ? aVar.getBool() : sal_False);
}
// -------------------------------------------------------------------------
::rtl::OUString ODatabaseMetaData::getStringProperty(const ::rtl::OUString& _aProperty)  throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    connectivity::ado::WpADOProperties aProps(m_pADOConnection->get_Properties());
    ADOS::ThrowException(*m_pADOConnection,*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");

    ADO_PROP(_aProperty);
    ::rtl::OUString aValue;
    if(!aVar.isNull() && !aVar.isEmpty() && aVar.getType() == VT_BSTR)
        aValue = aVar;

    return aValue;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTypeInfo(  ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getTypeInfo();
    //  ADOS::ThrowException(*m_pADOConnection,*this);

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setTypeInfoMap(ADOS::isJetEngine(m_pConnection->getEngineType()));
    Reference< XResultSet > xRef = pResult;
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCatalogs(  ) throw(SQLException, RuntimeException)
{
    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    ADORecordset *pRecordset = NULL;
    m_pADOConnection->OpenSchema(adSchemaCatalogs,vtEmpty,vtEmpty,&pRecordset);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCatalogsMap();
    xRef = pResult;

    return xRef;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogSeparator(  ) throw(SQLException, RuntimeException)
{
    return getLiteral(DBLITERAL_CATALOG_SEPARATOR);
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas(  ) throw(SQLException, RuntimeException)
{
    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    ADORecordset *pRecordset = NULL;
    m_pADOConnection->OpenSchema(adSchemaSchemata,vtEmpty,vtEmpty,&pRecordset);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setSchemasMap();
    xRef = pResult;
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
    const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getColumnPrivileges(catalog,schema,table,columnNamePattern);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setColumnPrivilegesMap();
    xRef = pResult;
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
    const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern,
    const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getColumns(catalog,schemaPattern,tableNamePattern,columnNamePattern);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setColumnsMap();
    xRef = pResult;

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& tableNamePattern, const Sequence< ::rtl::OUString >& types ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getTables(catalog,schemaPattern,tableNamePattern,types);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setTablesMap();
    xRef = pResult;

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& procedureNamePattern, const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getProcedureColumns(catalog,schemaPattern,procedureNamePattern,columnNamePattern);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setProcedureColumnsMap();
    xRef = pResult;

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& procedureNamePattern ) throw(SQLException, RuntimeException)
{
    // Create elements used in the array
    ADORecordset *pRecordset = m_pADOConnection->getProcedures(catalog,schemaPattern,procedureNamePattern);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setProceduresMap();
    xRef = pResult;

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getVersionColumns(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet(NULL);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_BINARY_LITERAL);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Maximum Row Size"));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_CATALOG_NAME);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_CHAR_LITERAL);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_COLUMN_NAME);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    //  return getInt32Property(::rtl::OUString::createFromAscii("Max Columns in Index"));
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_CURSOR_NAME);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Active Sessions"));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Max Columns in Table"));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_TEXT_COMMAND);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_TABLE_NAME);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTablesInSelect(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Maximum Tables in SELECT"));
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getExportedKeys(catalog,schema,table);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;
    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();
    xRef = pResult;

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getImportedKeys(catalog,schema,table);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();
    xRef = pResult;

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getPrimaryKeys(catalog,schema,table);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setPrimaryKeysMap();
    xRef = pResult;

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
    sal_Bool unique, sal_Bool approximate ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getIndexInfo(catalog,schema,table,unique,approximate);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setIndexInfoMap();
    xRef = pResult;

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getBestRowIdentifier(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Int32 scope,
    sal_Bool nullable ) throw(SQLException, RuntimeException)
{
    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(NULL);
    xRef = pResult;
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
    const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    Reference< XResultSet > xRef = NULL;
    if(!ADOS::isJetEngine(m_pConnection->getEngineType()))
    {   // the jet provider doesn't support this method
        // Create elements used in the array

        ADORecordset *pRecordset = m_pADOConnection->getTablePrivileges(catalog,schemaPattern,tableNamePattern);
        ADOS::ThrowException(*m_pADOConnection,*this);

        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
        pResult->setTablePrivilegesMap();
        xRef = pResult;
    }
    else
    {
        ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet();
        xRef = pResult;
        pResult->setTablePrivilegesMap();
        ::connectivity::ODatabaseMetaDataResultSet::ORows aRows;
        ::connectivity::ODatabaseMetaDataResultSet::ORow aRow(8);
        aRows.reserve(8);

        aRow[0] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[2] = new ::connectivity::ORowSetValueDecorator(tableNamePattern);
        aRow[3] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[4] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[5] = new ::connectivity::ORowSetValueDecorator(getUserName());
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getSelectValue();
        aRow[7] = new ::connectivity::ORowSetValueDecorator(::rtl::OUString::createFromAscii("NO"));

        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getInsertValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getDeleteValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getUpdateValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getCreateValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getReadValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getAlterValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getDropValue();
        aRows.push_back(aRow);
        pResult->setRows(aRows);
    }

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
    const Any& primaryCatalog, const ::rtl::OUString& primarySchema,
    const ::rtl::OUString& primaryTable, const Any& foreignCatalog,
    const ::rtl::OUString& foreignSchema, const ::rtl::OUString& foreignTable ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getCrossReference(primaryCatalog,primarySchema,primaryTable,foreignCatalog,foreignSchema,foreignTable);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();
    xRef = pResult;

    return xRef;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException)
{
    return getBoolProperty(::rtl::OUString::createFromAscii("Maximum Row Size Includes BLOB"));
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Identifier Case Sensitivity")) & DBPROPVAL_IC_LOWER) == DBPROPVAL_IC_LOWER ;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Identifier Case Sensitivity")) & DBPROPVAL_IC_LOWER) == DBPROPVAL_IC_LOWER ;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Identifier Case Sensitivity")) & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED ;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Identifier Case Sensitivity")) & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED ;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Identifier Case Sensitivity")) & DBPROPVAL_IC_UPPER) == DBPROPVAL_IC_UPPER ;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Identifier Case Sensitivity")) & DBPROPVAL_IC_UPPER) == DBPROPVAL_IC_UPPER ;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithAddColumn(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithDropColumn(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Maximum Index Size"));
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("NULL Concatenation Behavior")) == DBPROPVAL_CB_NON_NULL;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty(::rtl::OUString::createFromAscii("Catalog Term"));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getIdentifierQuoteString(  ) throw(SQLException, RuntimeException)
{
    return getLiteral(DBLITERAL_QUOTE_PREFIX);

}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    //  return getStringProperty(::rtl::OUString::createFromAscii("Special Characters"));
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return isCapable(DBLITERAL_CORRELATION_NAME);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::isCatalogAtStart(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Catalog Location")) == DBPROPVAL_CL_START;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Transaction DDL")) == DBPROPVAL_TC_DDL_IGNORE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Transaction DDL")) == DBPROPVAL_TC_DDL_COMMIT;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Transaction DDL")) == DBPROPVAL_TC_DML;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Transaction DDL")) == DBPROPVAL_TC_ALL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Prepare Abort Behavior")) == DBPROPVAL_CB_PRESERVE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Prepare Commit Behavior")) == DBPROPVAL_CB_PRESERVE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Isolation Retention")) & DBPROPVAL_TR_COMMIT) == DBPROPVAL_TR_COMMIT;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Isolation Retention")) & DBPROPVAL_TR_ABORT) == DBPROPVAL_TR_ABORT;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    sal_Bool bValue(sal_False);

    sal_Int32 nTxn = getInt32Property(::rtl::OUString::createFromAscii("Isolation Levels"));
    if(level == TransactionIsolation::NONE)
        bValue = sal_True;
    else if(level == TransactionIsolation::READ_UNCOMMITTED)
        bValue = (nTxn & DBPROPVAL_TI_READUNCOMMITTED) == DBPROPVAL_TI_READUNCOMMITTED;
    else if(level == TransactionIsolation::READ_COMMITTED)
        bValue = (nTxn & DBPROPVAL_TI_READCOMMITTED) == DBPROPVAL_TI_READCOMMITTED;
    else if(level == TransactionIsolation::REPEATABLE_READ)
        bValue = (nTxn & DBPROPVAL_TI_REPEATABLEREAD) == DBPROPVAL_TI_REPEATABLEREAD;
    else if(level == TransactionIsolation::SERIALIZABLE)
        bValue = (nTxn & DBPROPVAL_TI_SERIALIZABLE) == DBPROPVAL_TI_SERIALIZABLE;

    return bValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInDataManipulation(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Schema Usage")) & DBPROPVAL_SU_DML_STATEMENTS) == DBPROPVAL_SU_DML_STATEMENTS;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property(::rtl::OUString::createFromAscii("SQL Support"));
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_FULL) == DBPROPVAL_SQL_ANSI92_FULL);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property(::rtl::OUString::createFromAscii("SQL Support"));
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_ENTRY) == DBPROPVAL_SQL_ANSI92_ENTRY);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property(::rtl::OUString::createFromAscii("SQL Support"));
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI89_IEF) == DBPROPVAL_SQL_ANSI89_IEF);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Schema Usage")) & DBPROPVAL_SU_INDEX_DEFINITION) == DBPROPVAL_SU_INDEX_DEFINITION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInTableDefinitions(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Schema Usage")) & DBPROPVAL_SU_TABLE_DEFINITION) == DBPROPVAL_SU_TABLE_DEFINITION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInTableDefinitions(  ) throw(SQLException, RuntimeException)
{
    //  return (getInt32Property(::rtl::OUString::createFromAscii("Catalog Usage")) & DBPROPVAL_CU_TABLE_DEFINITION) == DBPROPVAL_CU_TABLE_DEFINITION;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    //  return (getInt32Property(::rtl::OUString::createFromAscii("Catalog Usage")) & DBPROPVAL_CU_INDEX_DEFINITION) == DBPROPVAL_CU_INDEX_DEFINITION;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInDataManipulation(  ) throw(SQLException, RuntimeException)
{
    //  return (getInt32Property(::rtl::OUString::createFromAscii("Catalog Usage")) & DBPROPVAL_CU_DML_STATEMENTS) == DBPROPVAL_CU_DML_STATEMENTS;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException)
{
    if ( ADOS::isJetEngine(m_pConnection->getEngineType()) )
        return sal_True;
    return getBoolProperty(::rtl::OUString::createFromAscii("Outer Join Capabilities"));
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    // Create elements used in the array
    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(NULL);
    xRef = pResult;
    return xRef;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatements(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_PROCEDURE_NAME);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_SCHEMA_NAME);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Transaction DDL")) == DBPROPVAL_TC_NONE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    return getBoolProperty(::rtl::OUString::createFromAscii("Read-Only Data Source"));
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("NULL Concatenation Behavior")) == DBPROPVAL_CB_NULL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException)
{
    return isCapable(DBLITERAL_COLUMN_ALIAS);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return isCapable(DBLITERAL_CORRELATION_NAME);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 fromType, sal_Int32 toType ) throw(SQLException, RuntimeException)
{
    return getBoolProperty(::rtl::OUString::createFromAscii("Rowset Conversions on Command"));
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return getBoolProperty(::rtl::OUString::createFromAscii("ORDER BY Columns in Select List"));
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("GROUP BY Support")) != DBPROPVAL_GB_NOT_SUPPORTED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("GROUP BY Support")) != DBPROPVAL_GB_CONTAINS_SELECT;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("GROUP BY Support")) == DBPROPVAL_GB_NO_RELATION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException)
{
    return isCapable(DBLITERAL_ESCAPE_PERCENT);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return getBoolProperty(::rtl::OUString::createFromAscii("ORDER BY Columns in Select List"));
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Identifier Case Sensitivity")) & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Identifier Case Sensitivity")) & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("NULL Collation Order")) & DBPROPVAL_NC_END) == DBPROPVAL_NC_END;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("NULL Collation Order")) & DBPROPVAL_NC_START) == DBPROPVAL_NC_START;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("NULL Collation Order")) & DBPROPVAL_NC_HIGH) == DBPROPVAL_NC_HIGH;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("NULL Collation Order")) & DBPROPVAL_NC_LOW) == DBPROPVAL_NC_LOW;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Schema Usage")) & DBPROPVAL_SU_PRIVILEGE_DEFINITION) == DBPROPVAL_SU_PRIVILEGE_DEFINITION;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    //  return (getInt32Property(::rtl::OUString::createFromAscii("Catalog Usage")) & DBPROPVAL_CU_PRIVILEGE_DEFINITION) == DBPROPVAL_CU_PRIVILEGE_DEFINITION;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Subquery Support")) & DBPROPVAL_SQ_CORRELATEDSUBQUERIES) == DBPROPVAL_SQ_CORRELATEDSUBQUERIES;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Subquery Support")) & DBPROPVAL_SQ_COMPARISON) == DBPROPVAL_SQ_COMPARISON;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Subquery Support")) & DBPROPVAL_SQ_EXISTS) == DBPROPVAL_SQ_EXISTS;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Subquery Support")) & DBPROPVAL_SQ_IN) == DBPROPVAL_SQ_IN;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property(::rtl::OUString::createFromAscii("Subquery Support")) & DBPROPVAL_SQ_QUANTIFIED) == DBPROPVAL_SQ_QUANTIFIED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property(::rtl::OUString::createFromAscii("SQL Support"));
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_INTERMEDIATE) == DBPROPVAL_SQL_ANSI92_INTERMEDIATE);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString::createFromAscii("sdbc:ado:")+ m_pADOConnection->GetConnectionString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty(::rtl::OUString::createFromAscii("User Name"));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty(::rtl::OUString::createFromAscii("Provider Friendly Name"));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDriverVersion(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty(::rtl::OUString::createFromAscii("Provider Version"));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty(::rtl::OUString::createFromAscii("DBMS Version"));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty(::rtl::OUString::createFromAscii("DBMS Name"));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty(::rtl::OUString::createFromAscii("Procedure Term"));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty(::rtl::OUString::createFromAscii("Schema Term"));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nRet = TransactionIsolation::NONE;
    switch(m_pADOConnection->get_IsolationLevel())
    {
        case adXactReadCommitted:
            nRet = TransactionIsolation::READ_COMMITTED;
            break;
        case adXactRepeatableRead:
            nRet = TransactionIsolation::REPEATABLE_READ;
            break;
        case adXactSerializable:
            nRet = TransactionIsolation::SERIALIZABLE;
            break;
        case adXactReadUncommitted:
            nRet = TransactionIsolation::READ_UNCOMMITTED;
            break;
        default:
            ;
    }
    return nRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = NULL;
    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();
    m_pADOConnection->OpenSchema(adSchemaDBInfoKeywords,vtEmpty,vtEmpty,&pRecordset);
    ADOS::ThrowException(*m_pADOConnection,*this);
    WpADORecordset aRecordset(pRecordset);

    aRecordset.MoveFirst();
    OLEVariant  aValue;
    ::rtl::OUString aRet,aComma = ::rtl::OUString::createFromAscii(",");;
    while(!aRecordset.IsAtEOF())
    {
        WpOLEAppendCollection<ADOFields, ADOField, WpADOField>  aFields(aRecordset.GetFields());
        WpADOField aField(aFields.GetItem(0));
        aField.get_Value(aValue);
        aRet = aRet + aValue + aComma;
        aRecordset.MoveNext();
    }
    aRecordset.Close();
    return aRet.copy(0,aRet.lastIndexOf(','));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    return getLiteral(DBLITERAL_ESCAPE_PERCENT);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue.copy(0,aValue.lastIndexOf(','));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue.copy(0,aValue.lastIndexOf(','));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property(::rtl::OUString::createFromAscii("SQL Support"));
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_EXTENDED) == DBPROPVAL_SQL_ODBC_EXTENDED);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property(::rtl::OUString::createFromAscii("SQL Support"));
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_CORE) == DBPROPVAL_SQL_ODBC_CORE);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property(::rtl::OUString::createFromAscii("SQL Support"));
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_MINIMUM) == DBPROPVAL_SQL_ODBC_MINIMUM);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException)
{
    if ( ADOS::isJetEngine(m_pConnection->getEngineType()) )
        return sal_True;
    return (getInt32Property(::rtl::OUString::createFromAscii("Outer Join Capabilities")) & 0x00000004L) == 0x00000004L;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return supportsFullOuterJoins(  );
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Max Columns in GROUP BY"));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property(::rtl::OUString::createFromAscii("Max Columns in ORDER BY"));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException)
{
    return 0; // getInt32Property(::rtl::OUString::createFromAscii("Max Columns in Select"));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_USER_NAME);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& typeNamePattern, const Sequence< sal_Int32 >& types ) throw(SQLException, RuntimeException)
{
    Reference< XResultSet > xRef = NULL;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(NULL);
    xRef = pResult;
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODatabaseMetaData::getConnection(  ) throw(SQLException, RuntimeException)
{
    return (Reference< XConnection >)m_pConnection;//new OConnection(m_aConnectionHandle);
}
// -------------------------------------------------------------------------


