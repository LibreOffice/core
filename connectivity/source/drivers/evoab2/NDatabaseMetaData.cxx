/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "NDatabaseMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <connectivity/dbexception.hxx>
#include <connectivity/FValue.hxx>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>

#include <vector>
#include <string.h>
#include "EApi.h"

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */

using namespace connectivity::evoab;
using namespace connectivity;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;


namespace connectivity
{
    namespace evoab
    {
        static sal_Int32    const s_nCOLUMN_SIZE = 256;
        static sal_Int32    const s_nDECIMAL_DIGITS = 0;
        static sal_Int32    const s_nNULLABLE = 1;
        static sal_Int32 const s_nCHAR_OCTET_LENGTH = 65535;

        static ColumnProperty **pFields=NULL;
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
            pToBeFields[nFields]->pField = g_param_spec_ref(g_param_spec_string (evo_addr[i].pColumnName,evo_addr[i].pColumnName,"",NULL,G_PARAM_WRITABLE));
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
                    ( (GObjectClass *) g_type_class_ref( E_TYPE_CONTACT ),
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


    guint
    getFieldCount()
    {
        initFields();
        return nFields;
    }

    const ColumnProperty *
    getField(guint n)
    {
        initFields();
        if( n < nFields )
            return pFields[n];
        else
            return NULL;
    }

    GType
    getGFieldType( guint nCol )
    {
        initFields();

        sal_Int32 nType = G_TYPE_STRING;
        if ( nCol < nFields )
            return ((GParamSpec *)pFields[nCol]->pField)->value_type;
        return nType;
    }

    sal_Int32
    getFieldType( guint nCol )
    {
        sal_Int32 nType = getGFieldType( nCol );
        return nType == G_TYPE_STRING ? DataType::VARCHAR : DataType::BIT;
    }

    guint findEvoabField(const rtl::OUString& aColName)
    {
        guint nRet = (guint)-1;
        sal_Bool bFound = sal_False;
        initFields();
        for (guint i=0;(i < nFields) && !bFound;i++)
        {
            rtl::OUString aName = getFieldName(i);
            if (aName == aColName)
            {
                nRet = i;
                bFound = sal_True;
            }
        }
        return nRet;
    }

    rtl::OUString
    getFieldTypeName( guint nCol )
    {
        switch( getFieldType( nCol ) )
        {
            case DataType::BIT:
                return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BIT"));
            case DataType::VARCHAR:
                return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VARCHAR"));
            default:
                break;
        }
        return ::rtl::OUString();
    }

    rtl::OUString
    getFieldName( guint nCol )
    {
        const GParamSpec *pSpec = getField( nCol )->pField;
        rtl::OUString aName;
        initFields();

        if( pSpec )
            aName = rtl::OStringToOUString( g_param_spec_get_name( ( GParamSpec * )pSpec ),
                                            RTL_TEXTENCODING_UTF8 );
            aName = aName.replace( '-', '_' );
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
            pFields=NULL;
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

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::ORows& OEvoabDatabaseMetaData::getColumnRows( const ::rtl::OUString& columnNamePattern )
{
    static ODatabaseMetaDataResultSet::ORows aRows;
    ODatabaseMetaDataResultSet::ORow  aRow(19);
    aRows.clear();

    // ****************************************************
    // Some entries in a row never change, so set them now
    // ****************************************************

    // Catalog
    aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")));
    // Schema
    aRow[2] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")));
    // COLUMN_SIZE
    aRow[7] = new ORowSetValueDecorator(s_nCOLUMN_SIZE);
    // BUFFER_LENGTH, not used
    aRow[8] = ODatabaseMetaDataResultSet::getEmptyValue();
    // DECIMAL_DIGITS.
    aRow[9] = new ORowSetValueDecorator(s_nDECIMAL_DIGITS);
    // NUM_PREC_RADIX
    aRow[10] = new ORowSetValueDecorator((sal_Int32)10);
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
    aRow[18] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("YES")));


    aRow[3] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE")));
    ::osl::MutexGuard aGuard( m_aMutex );

    initFields();
    for (sal_Int32 i = 0; i < (sal_Int32) nFields; i++)
    {
        if( match( columnNamePattern, getFieldName( i ), '\0' ) )
        {
            aRow[5] = new ORowSetValueDecorator( static_cast<sal_Int16>( getFieldType( i ) ) );
            aRow[6] = new ORowSetValueDecorator( getFieldTypeName( i ) );

            OSL_TRACE( "ColumnName = '%s'", g_param_spec_get_name( pFields[i]->pField ) );
            // COLUMN_NAME
            aRow[4] = new ORowSetValueDecorator( getFieldName( i ) );
            // ORDINAL_POSITION
            aRow[17] = new ORowSetValueDecorator( i );
            aRows.push_back( aRow );
        }
    }

    return aRows ;
}
// -------------------------------------------------------------------------
::rtl::OUString OEvoabDatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 OEvoabDatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 OEvoabDatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    // We only support a single table
    return 1;
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool OEvoabDatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool OEvoabDatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool OEvoabDatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    return aVal;
}
// -------------------------------------------------------------------------
::rtl::OUString OEvoabDatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    // normally this is "
    ::rtl::OUString aVal(RTL_CONSTASCII_USTRINGPARAM("\""));
    return aVal;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    return aVal;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool OEvoabDatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    sal_Bool bValue = sal_False;
    return bValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 /*level*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool OEvoabDatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_True; // should be supported at least
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool OEvoabDatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool OEvoabDatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool OEvoabDatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException)
{
    // We allow you to select from any table.
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    // For now definately read-only, no support for update/delete
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException)
{
    // todo add Support for this.
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool OEvoabDatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    // Any case may be used
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pConnection->getURL();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getDriverVersion() throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue = ::rtl::OUString::valueOf((sal_Int32)1);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue = ::rtl::OUString::valueOf((sal_Int32)0);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    return TransactionIsolation::NONE;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsResultSetType( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsResultSetConcurrency( sal_Int32 /*setType*/, sal_Int32 /*concurrency*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::ownUpdatesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::ownDeletesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::ownInsertsAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::othersUpdatesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::othersDeletesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::othersInsertsAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::updatesAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::deletesAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::insertsAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
// here follow all methods which return a resultset
// the first methods is an example implementation how to use this resultset
// of course you could implement it on your and you should do this because
// the general way is more memory expensive
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OEvoabDatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    /* Dont need to change as evoab driver supports only table */

    // there exists no possibility to get table types so we have to check
    static ::rtl::OUString sTableTypes[] =
    {
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE")),
        // Currently we only support a 'TABLE' nothing more complex
    };
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTableTypes);
    Reference< XResultSet > xRef = pResult;

    // here we fill the rows which should be visible when ask for data from the resultset returned here
    sal_Int32  nSize = sizeof(sTableTypes) / sizeof(::rtl::OUString);
    ODatabaseMetaDataResultSet::ORows aRows;
    for(sal_Int32 i=0;i < nSize;++i)
    {
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(sTableTypes[i]));

        // bound row
        aRows.push_back(aRow);
    }
    // here we set the rows at the resultset
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > OEvoabDatabaseMetaData::impl_getTypeInfo_throw(  )
{
    /*
     * Return the proper type information required by evo driver
     */

    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTypeInfo);

    Reference< XResultSet > xResultSet = pResultSet;
    static ODatabaseMetaDataResultSet::ORows aRows;

    if(aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.reserve(19);
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VARCHAR"))));
        aRow.push_back(new ORowSetValueDecorator(DataType::VARCHAR));
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)s_nCHAR_OCTET_LENGTH));
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        // aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnValue::NULLABLE));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnSearch::FULL));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)10));

        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VARCHAR")));
        aRow[2] = new ORowSetValueDecorator(DataType::VARCHAR);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)65535);
        aRows.push_back(aRow);
    }
    pResultSet->setRows(aRows);
    return xResultSet;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OEvoabDatabaseMetaData::getColumns(
    const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/, const ::rtl::OUString& /*tableNamePattern*/,
    const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    // this returns an empty resultset where the column-names are already set
    // in special the metadata of the resultset already returns the right columns
    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eColumns );
    Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( getColumnRows( columnNamePattern ) );
    return xResultSet;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OEvoabDatabaseMetaData::getTables(
    const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/,
    const ::rtl::OUString& /*tableNamePattern*/, const Sequence< ::rtl::OUString >& types ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTableTypes);
    Reference< XResultSet > xRef = pResult;

    // check if any type is given
    // when no types are given then we have to return all tables e.g. TABLE

    const ::rtl::OUString aTable(RTL_CONSTASCII_USTRINGPARAM("TABLE"));

    sal_Bool bTableFound = sal_True;
    sal_Int32 nLength = types.getLength();
    if(nLength)
        {
            bTableFound = sal_False;

            const ::rtl::OUString* pBegin = types.getConstArray();
            const ::rtl::OUString* pEnd = pBegin + nLength;
            for(;pBegin != pEnd;++pBegin)
                {
                    if(*pBegin == aTable)
                        {
                            bTableFound = sal_True;
                            break;
                        }
                }
        }
    if(!bTableFound)
        return xRef;

    ODatabaseMetaDataResultSet::ORows aRows;

    ESourceList *pSourceList;
    if( !e_book_get_addressbooks (&pSourceList, NULL) )
            pSourceList = NULL;

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

            rtl::OUString aName = rtl::OStringToOUString( e_source_peek_name( pSource ),
                                                          RTL_TEXTENCODING_UTF8 );

            ODatabaseMetaDataResultSet::ORow aRow(3);
            aRow.reserve(6);
            aRow.push_back(new ORowSetValueDecorator(aName));
            aRow.push_back(new ORowSetValueDecorator(aTable));
            aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
            aRows.push_back(aRow);
        }
    }

    pResult->setRows(aRows);

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OEvoabDatabaseMetaData::getUDTs( const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/, const ::rtl::OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XDatabaseMetaDaza::getUDTs", *this );
    return NULL;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
