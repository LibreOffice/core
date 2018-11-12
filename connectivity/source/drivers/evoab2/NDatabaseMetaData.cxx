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

#include "NDatabaseMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <connectivity/dbexception.hxx>
#include <connectivity/FValue.hxx>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>

#include <cstddef>
#include <vector>
#include <string.h>
#include "EApi.h"

using namespace connectivity::evoab;
using namespace connectivity;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

namespace
{
    bool equal(const char *str1, const char *str2)
    {
        return str1 == nullptr || str2 == nullptr ? str1 == str2 : strcmp(str1, str2) == 0;
    }
}

namespace connectivity
{
    namespace evoab
    {
        static sal_Int32    const s_nCOLUMN_SIZE = 256;
        static sal_Int32    const s_nDECIMAL_DIGITS = 0;
        static sal_Int32    const s_nNULLABLE = 1;
        static sal_Int32 const s_nCHAR_OCTET_LENGTH = 65535;

        static ColumnProperty **pFields=nullptr;
        static guint        nFields = 0;

        static const char *pBlackList[] =
        {
            "id",
            "list-show-addresses",
            "address-label-home",
            "address-label-work",
            "address-label-other"
        };

    const SplitEvoColumns* get_evo_addr()
    {
        static const SplitEvoColumns evo_addr[] = {
            {"addr-line1",DEFAULT_ADDR_LINE1},{"addr-line2",DEFAULT_ADDR_LINE2},{"city",DEFAULT_CITY},{"state",DEFAULT_STATE},{"country",DEFAULT_COUNTRY},{"zip",DEFAULT_ZIP},
            {"work-addr-line1",WORK_ADDR_LINE1},{"work-addr-line2",WORK_ADDR_LINE2},{"work-city",WORK_CITY},{"work-state",WORK_STATE},{"work-country",WORK_COUNTRY},{"work-zip",WORK_ZIP},
            {"home-addr-line1",HOME_ADDR_LINE1},{"home-addr-line2",HOME_ADDR_LINE2},{"home-addr-City",HOME_CITY},{"home-state",HOME_STATE},{"home-country",HOME_COUNTRY},{"home-zip",HOME_ZIP},
            {"other-addr-line1",OTHER_ADDR_LINE1},{"other-addr-line2",OTHER_ADDR_LINE2},{"other-addr-city",OTHER_CITY},{"other-addr-state",OTHER_STATE},{"other-addr-country",OTHER_COUNTRY},{"other-addr-zip",OTHER_ZIP}
        };
        return evo_addr;
    }

    static void
    splitColumn (ColumnProperty **pToBeFields)
    {
        const SplitEvoColumns* evo_addr( get_evo_addr() );
        for (int i = 0; i < OTHER_ZIP; i++)
        {
            pToBeFields[nFields] = g_new0(ColumnProperty,1);
            pToBeFields[nFields]->bIsSplittedValue = true;
            pToBeFields[nFields]->pField = g_param_spec_ref(g_param_spec_string (evo_addr[i].pColumnName,evo_addr[i].pColumnName,"",nullptr,G_PARAM_WRITABLE));
            nFields++;
        }
    }

    static void
    initFields()
    {
        if( !pFields )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pFields )
            {
                guint        nProps;
                ColumnProperty **pToBeFields;
                GParamSpec **pProps;
                nFields = 0;
                pProps = g_object_class_list_properties
                    ( static_cast<GObjectClass *>(g_type_class_ref( E_TYPE_CONTACT )),
                         &nProps );
                pToBeFields = g_new0(ColumnProperty  *, (nProps + OTHER_ZIP)/* new column(s)*/ );
                for ( guint i = 0; i < nProps; i++ )
                {
                    switch (pProps[i]->value_type)
                    {
                        case G_TYPE_STRING:
                        case G_TYPE_BOOLEAN:
                        {
                            bool bAdd = true;
                            const char *pName = g_param_spec_get_name( pProps[i] );
                            for (unsigned int j = 0; j < G_N_ELEMENTS( pBlackList ); j++ )
                            {
                                if( !strcmp( pBlackList[j], pName ) )
                                {
                                    bAdd = false;
                                    break;
                                }
                            }
                            if( bAdd )
                            {
                                pToBeFields[nFields]= g_new0(ColumnProperty,1);
                                pToBeFields[nFields]->bIsSplittedValue=false;
                                pToBeFields[ nFields++ ]->pField = g_param_spec_ref( pProps[i] );
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }

                splitColumn(pToBeFields);
                pFields = pToBeFields;
            }
        }
    }


    const ColumnProperty *
    getField(guint n)
    {
        initFields();
        if( n < nFields )
            return pFields[n];
        else
            return nullptr;
    }

    GType
    getGFieldType( guint nCol )
    {
        initFields();

        if ( nCol < nFields )
            return pFields[nCol]->pField->value_type;
        return G_TYPE_STRING;
    }

    sal_Int32
    getFieldType( guint nCol )
    {
        sal_Int32 nType = getGFieldType( nCol );
        return nType == G_TYPE_STRING ? DataType::VARCHAR : DataType::BIT;
    }

    guint findEvoabField(const OUString& aColName)
    {
        guint nRet = guint(-1);
        bool bFound = false;
        initFields();
        for (guint i=0;(i < nFields) && !bFound;i++)
        {
            OUString aName = getFieldName(i);
            if (aName == aColName)
            {
                nRet = i;
                bFound = true;
            }
        }
        return nRet;
    }

    OUString
    getFieldTypeName( guint nCol )
    {
        switch( getFieldType( nCol ) )
        {
            case DataType::BIT:
                return OUString("BIT");
            case DataType::VARCHAR:
                return OUString("VARCHAR");
            default:
                break;
        }
        return OUString();
    }

    OUString
    getFieldName( guint nCol )
    {
        const GParamSpec *pSpec = getField( nCol )->pField;
        OUString aName;
        initFields();

        if( pSpec )
        {
            aName = OStringToOUString( g_param_spec_get_name( const_cast<GParamSpec *>(pSpec) ),
                                            RTL_TEXTENCODING_UTF8 );
            aName = aName.replace( '-', '_' );
        }
        return aName;
    }

    void
    free_column_resources()
    {
        for (int i=nFields-1;i > 0;i--)
        {
            if (pFields && pFields[i] )
            {
                if (pFields[i]->pField)
                    g_param_spec_unref(pFields[i]->pField);
                g_free(pFields[i]);
            }
        }
        if(pFields)
        {
            g_free(pFields);
            pFields=nullptr;
        }

    }


    }
}


OEvoabDatabaseMetaData::OEvoabDatabaseMetaData(OEvoabConnection* _pCon)
    : ::connectivity::ODatabaseMetaDataBase(_pCon, _pCon->getConnectionInfo())
    ,m_pConnection(_pCon)
{
    OSL_ENSURE(m_pConnection,"OEvoabDatabaseMetaData::OEvoabDatabaseMetaData: No connection set!");
}
OEvoabDatabaseMetaData::~OEvoabDatabaseMetaData()
{
}


ODatabaseMetaDataResultSet::ORows OEvoabDatabaseMetaData::getColumnRows( const OUString& columnNamePattern )
{
    ODatabaseMetaDataResultSet::ORows aRows;
    ODatabaseMetaDataResultSet::ORow  aRow(19);

    // ****************************************************
    // Some entries in a row never change, so set them now
    // ****************************************************

    // Catalog
    aRow[1] = new ORowSetValueDecorator(OUString());
    // Schema
    aRow[2] = new ORowSetValueDecorator(OUString());
    // COLUMN_SIZE
    aRow[7] = new ORowSetValueDecorator(s_nCOLUMN_SIZE);
    // BUFFER_LENGTH, not used
    aRow[8] = ODatabaseMetaDataResultSet::getEmptyValue();
    // DECIMAL_DIGITS.
    aRow[9] = new ORowSetValueDecorator(s_nDECIMAL_DIGITS);
    // NUM_PREC_RADIX
    aRow[10] = new ORowSetValueDecorator(sal_Int32(10));
    // NULLABLE
    aRow[11] = new ORowSetValueDecorator(s_nNULLABLE);
    // REMARKS
    aRow[12] = ODatabaseMetaDataResultSet::getEmptyValue();
    // COULUMN_DEF, not used
    aRow[13] = ODatabaseMetaDataResultSet::getEmptyValue();
    // SQL_DATA_TYPE, not used
    aRow[14] = ODatabaseMetaDataResultSet::getEmptyValue();
    // SQL_DATETIME_SUB, not used
    aRow[15] = ODatabaseMetaDataResultSet::getEmptyValue();
    // CHAR_OCTET_LENGTH, refer to [5]
    aRow[16] = new ORowSetValueDecorator(s_nCHAR_OCTET_LENGTH);
    // IS_NULLABLE
    aRow[18] = new ORowSetValueDecorator(OUString("YES"));


    aRow[3] = new ORowSetValueDecorator(OUString("TABLE"));
    ::osl::MutexGuard aGuard( m_aMutex );

    initFields();
    for (sal_Int32 i = 0; i < static_cast<sal_Int32>(nFields); i++)
    {
        if( match( columnNamePattern, getFieldName( i ), '\0' ) )
        {
            aRow[5] = new ORowSetValueDecorator( static_cast<sal_Int16>( getFieldType( i ) ) );
            aRow[6] = new ORowSetValueDecorator( getFieldTypeName( i ) );

            // COLUMN_NAME
            aRow[4] = new ORowSetValueDecorator( getFieldName( i ) );
            // ORDINAL_POSITION
            aRow[17] = new ORowSetValueDecorator( i );
            aRows.push_back( aRow );
        }
    }

    return aRows ;
}

OUString OEvoabDatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    return OUString();
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxRowSize(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxCatalogNameLength(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxCharLiteralLength(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnNameLength(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnsInIndex(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxCursorNameLength(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxConnections(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnsInTable(  )
{
    return 0;// 0 means no limit
}

sal_Int32 OEvoabDatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxTableNameLength(  )
{
    return 0;// 0 means no limit
}

sal_Int32 OEvoabDatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    // We only support a single table
    return 1;
}


sal_Bool SAL_CALL OEvoabDatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    return false;
}

bool OEvoabDatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::storesUpperCaseIdentifiers(  )
{
    return false;
}

bool OEvoabDatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    return false;
}

bool OEvoabDatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    return false;
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxIndexLength(  )
{
    return 0;// 0 means no limit
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsNonNullableColumns(  )
{
    return false;
}

OUString SAL_CALL OEvoabDatabaseMetaData::getCatalogTerm(  )
{
    return OUString();
}

OUString OEvoabDatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    // normally this is "
    return OUString("\"");
}

OUString SAL_CALL OEvoabDatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString();
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return false;
}

bool OEvoabDatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    return true;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    return true;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    return true;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    return true;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsPositionedDelete(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsPositionedUpdate(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 /*level*/ )
{
    return false;
}

bool OEvoabDatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsANSI92FullSQL(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    return true; // should be supported at least
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSchemasInIndexDefinitions(  )
{
    return false;
}

bool OEvoabDatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    return false;
}

bool OEvoabDatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    return false;
}

bool OEvoabDatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOuterJoins(  )
{
    return false;
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxStatementLength(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxProcedureNameLength(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxSchemaNameLength(  )
{
    return 0;// 0 means no limit
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsTransactions(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::allProceduresAreCallable(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsStoredProcedures(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSelectForUpdate(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::allTablesAreSelectable(  )
{
    // We allow you to select from any table.
    return true;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::isReadOnly(  )
{
    // For now definitely read-only, no support for update/delete
    return true;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::usesLocalFiles(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::usesLocalFilePerTable(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsTypeConversion(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsColumnAliasing(  )
{
    // todo add Support for this.
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsTableCorrelationNames(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsGroupBy(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsGroupByUnrelated(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsMultipleTransactions(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsMultipleResultSets(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsLikeEscapeClause(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOrderByUnrelated(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsUnion(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsUnionAll(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    return false;
}

bool OEvoabDatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    // Any case may be used
    return true;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::nullsAreSortedAtStart(  )
{
    return true;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::nullsAreSortedHigh(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::nullsAreSortedLow(  )
{
    return true;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSubqueriesInExists(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSubqueriesInIns(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    return false;
}

OUString SAL_CALL OEvoabDatabaseMetaData::getURL(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pConnection->getURL();
}

OUString SAL_CALL OEvoabDatabaseMetaData::getUserName(  )
{
    return OUString();
}

OUString SAL_CALL OEvoabDatabaseMetaData::getDriverName(  )
{
    return OUString();
}

OUString SAL_CALL OEvoabDatabaseMetaData::getDriverVersion()
{
    return OUString( "1" );
}

OUString SAL_CALL OEvoabDatabaseMetaData::getDatabaseProductVersion(  )
{
    return OUString( "0" );
}

OUString SAL_CALL OEvoabDatabaseMetaData::getDatabaseProductName(  )
{
    return OUString();
}

OUString SAL_CALL OEvoabDatabaseMetaData::getProcedureTerm(  )
{
    return OUString();
}

OUString SAL_CALL OEvoabDatabaseMetaData::getSchemaTerm(  )
{
    return OUString();
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getDriverMajorVersion(  )
{
    return 1;
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getDefaultTransactionIsolation(  )
{
    return TransactionIsolation::NONE;
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getDriverMinorVersion(  )
{
    return 0;
}

OUString SAL_CALL OEvoabDatabaseMetaData::getSQLKeywords(  )
{
    return OUString();
}

OUString SAL_CALL OEvoabDatabaseMetaData::getSearchStringEscape(  )
{
    return OUString();
}

OUString SAL_CALL OEvoabDatabaseMetaData::getStringFunctions(  )
{
    return OUString();
}

OUString SAL_CALL OEvoabDatabaseMetaData::getTimeDateFunctions(  )
{
    return OUString();
}

OUString SAL_CALL OEvoabDatabaseMetaData::getSystemFunctions(  )
{
    return OUString();
}

OUString SAL_CALL OEvoabDatabaseMetaData::getNumericFunctions(  )
{
    return OUString();
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsCoreSQLGrammar(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    return true;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsFullOuterJoins(  )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return false;
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnsInSelect(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxUserNameLength(  )
{
    return 0;// 0 means no limit
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsResultSetType( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsResultSetConcurrency( sal_Int32 /*setType*/, sal_Int32 /*concurrency*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::ownUpdatesAreVisible( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::ownDeletesAreVisible( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::ownInsertsAreVisible( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::othersUpdatesAreVisible( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::othersDeletesAreVisible( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::othersInsertsAreVisible( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::updatesAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::deletesAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::insertsAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsBatchUpdates(  )
{
    return false;
}

// here follow all methods which return a resultset
// the first methods is an example implementation how to use this resultset
// of course you could implement it on your and you should do this because
// the general way is more memory expensive

Reference< XResultSet > SAL_CALL OEvoabDatabaseMetaData::getTableTypes(  )
{
    /* Don't need to change as evoab driver supports only table */

    // there exists no possibility to get table types so we have to check
    static const OUStringLiteral sTableTypes[] =
    {
        "TABLE" // Currently we only support a 'TABLE' nothing more complex
    };
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTableTypes);
    Reference< XResultSet > xRef = pResult;

    // here we fill the rows which should be visible when ask for data from the resultset returned here
    ODatabaseMetaDataResultSet::ORows aRows;
    for(std::size_t i=0;i < SAL_N_ELEMENTS(sTableTypes);++i)
    {
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(OUString(sTableTypes[i])));

        // bound row
        aRows.push_back(aRow);
    }
    // here we set the rows at the resultset
    pResult->setRows(aRows);
    return xRef;
}

Reference< XResultSet > OEvoabDatabaseMetaData::impl_getTypeInfo_throw(  )
{
    /*
     * Return the proper type information required by evo driver
     */

    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTypeInfo);

    Reference< XResultSet > xResultSet = pResultSet;
    static ODatabaseMetaDataResultSet::ORows aRows = []()
    {
        ODatabaseMetaDataResultSet::ORows tmp;
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.reserve(19);
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(OUString("VARCHAR")));
        aRow.push_back(new ORowSetValueDecorator(DataType::VARCHAR));
        aRow.push_back(new ORowSetValueDecorator(sal_Int32(s_nCHAR_OCTET_LENGTH)));
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        // aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnValue::NULLABLE));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(new ORowSetValueDecorator(sal_Int32(ColumnSearch::FULL)));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(sal_Int32(10)));

        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("VARCHAR"));
        aRow[2] = new ORowSetValueDecorator(DataType::VARCHAR);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(65535));
        tmp.push_back(aRow);
        return tmp;
    }();
    pResultSet->setRows(aRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL OEvoabDatabaseMetaData::getColumns(
    const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*tableNamePattern*/,
    const OUString& columnNamePattern )
{
    // this returns an empty resultset where the column-names are already set
    // in special the metadata of the resultset already returns the right columns
    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eColumns );
    Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( getColumnRows( columnNamePattern ) );
    return xResultSet;
}


bool isSourceBackend(ESource *pSource, const char *backendname)
{
    if (!pSource || !e_source_has_extension (pSource, E_SOURCE_EXTENSION_ADDRESS_BOOK))
        return false;

    gpointer extension = e_source_get_extension (pSource, E_SOURCE_EXTENSION_ADDRESS_BOOK);
    return extension && equal(e_source_backend_get_backend_name (extension), backendname);
}

Reference< XResultSet > SAL_CALL OEvoabDatabaseMetaData::getTables(
    const Any& /*catalog*/, const OUString& /*schemaPattern*/,
    const OUString& /*tableNamePattern*/, const Sequence< OUString >& types )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTableTypes);
    Reference< XResultSet > xRef = pResult;

    // check if any type is given
    // when no types are given then we have to return all tables e.g. TABLE

    const OUString aTable("TABLE");

    bool bTableFound = true;
    sal_Int32 nLength = types.getLength();
    if(nLength)
        {
            bTableFound = false;

            const OUString* pBegin = types.getConstArray();
            const OUString* pEnd = pBegin + nLength;
            for(;pBegin != pEnd;++pBegin)
                {
                    if(*pBegin == aTable)
                        {
                            bTableFound = true;
                            break;
                        }
                }
        }
    if(!bTableFound)
        return xRef;

    ODatabaseMetaDataResultSet::ORows aRows;

    if (eds_check_version(3, 6, 0) == nullptr)
    {
        GList *pSources = e_source_registry_list_sources(get_e_source_registry(), E_SOURCE_EXTENSION_ADDRESS_BOOK);

        for (GList* liter = pSources; liter; liter = liter->next)
        {
            ESource *pSource = E_SOURCE (liter->data);
            bool can = false;
            switch (m_pConnection->getSDBCAddressType())
            {
                case SDBCAddress::EVO_GWISE:
                    can = isSourceBackend( pSource, "groupwise"); // not supported in evo/eds 3.6.x+, somehow
                    break;
                case SDBCAddress::EVO_LOCAL:
                    can = isSourceBackend( pSource, "local");
                    break;
                case SDBCAddress::EVO_LDAP:
                    can = isSourceBackend( pSource, "ldap");
                    break;
                case SDBCAddress::Unknown:
                    can = true;
                    break;
            }
            if (!can)
                continue;

            OUString aHumanName = OStringToOUString( e_source_get_display_name( pSource ),
                                                          RTL_TEXTENCODING_UTF8 );
            OUString aUID = OStringToOUString( e_source_get_uid( pSource ),
                                                          RTL_TEXTENCODING_UTF8 );
            ODatabaseMetaDataResultSet::ORow aRow{
                ORowSetValueDecoratorRef(),
                ORowSetValueDecoratorRef(),
                ORowSetValueDecoratorRef(),
                new ORowSetValueDecorator(aHumanName), //tablename
                new ORowSetValueDecorator(aTable),
                new ORowSetValueDecorator(aUID)}; //comment
            //I'd prefer to swap the comment and the human name and
            //just use e_source_registry_ref_source(get_e_source_registry(), aUID);
            //in open book rather than search for the name again
            aRows.push_back(aRow);
        }

        g_list_foreach (pSources, reinterpret_cast<GFunc>(g_object_unref), nullptr);
        g_list_free (pSources);
    }
    else
    {
        ESourceList *pSourceList;
        if( !e_book_get_addressbooks (&pSourceList, nullptr) )
                pSourceList = nullptr;

        GSList *g;
        for( g = e_source_list_peek_groups( pSourceList ); g; g = g->next)
        {
            GSList *s;
            const char *p = e_source_group_peek_base_uri(E_SOURCE_GROUP(g->data));

            switch (m_pConnection->getSDBCAddressType()) {
            case SDBCAddress::EVO_GWISE:
                        if ( !strncmp( "groupwise://", p, 11 ))
                            break;
                        else
                            continue;
            case SDBCAddress::EVO_LOCAL:
                        if ( !strncmp( "file://", p, 6 ) ||
                             !strncmp( "local://", p, 6 ) )
                            break;
                        else
                            continue;
            case SDBCAddress::EVO_LDAP:
                        if ( !strncmp( "ldap://", p, 6 ))
                            break;
                        else
                            continue;
            case SDBCAddress::Unknown:
                break;
            }
            for (s = e_source_group_peek_sources (E_SOURCE_GROUP (g->data)); s; s = s->next)
            {
                ESource *pSource = E_SOURCE (s->data);

                OUString aName = OStringToOUString( e_source_peek_name( pSource ),
                                                              RTL_TEXTENCODING_UTF8 );

                ODatabaseMetaDataResultSet::ORow aRow{
                    ORowSetValueDecoratorRef(),
                    ORowSetValueDecoratorRef(),
                    ORowSetValueDecoratorRef(),
                    new ORowSetValueDecorator(aName),
                    new ORowSetValueDecorator(aTable),
                    ODatabaseMetaDataResultSet::getEmptyValue()};
                aRows.push_back(aRow);
            }
        }
    }

    pResult->setRows(aRows);

    return xRef;
}

Reference< XResultSet > SAL_CALL OEvoabDatabaseMetaData::getUDTs( const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XDatabaseMetaDaza::getUDTs", *this );
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
