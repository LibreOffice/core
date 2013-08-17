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


#include "OptimisticSet.hxx"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include "dbastrings.hrc"
#include "apitools.hxx"
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <list>
#include <algorithm>
#include <string.h>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include "querycomposer.hxx"
#include "composertools.hxx"
#include <tools/debug.hxx>

using namespace dbaccess;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star;
using namespace ::cppu;
using namespace ::osl;

DECLARE_STL_USTRINGACCESS_MAP(OUStringBuffer,TSQLStatements);
namespace
{
    void lcl_fillKeyCondition(const OUString& i_sTableName,const OUString& i_sQuotedColumnName,const ORowSetValue& i_aValue,TSQLStatements& io_aKeyConditions)
    {
        OUStringBuffer& rKeyCondition = io_aKeyConditions[i_sTableName];
        if ( !rKeyCondition.isEmpty() )
            rKeyCondition.append(" AND ");
        rKeyCondition.append(i_sQuotedColumnName);
        if ( i_aValue.isNull() )
            rKeyCondition.append(" IS NULL");
        else
            rKeyCondition.append(" = ?");
    }
}

DBG_NAME(OptimisticSet)

OptimisticSet::OptimisticSet(const Reference<XComponentContext>& _rContext,
                             const Reference< XConnection>& i_xConnection,
                             const Reference< XSingleSelectQueryAnalyzer >& _xComposer,
                             const ORowSetValueVector& _aParameterValueForCache,
                             sal_Int32 i_nMaxRows,
                             sal_Int32& o_nRowCount)
            :OKeySet(NULL,NULL,OUString(),_xComposer,_aParameterValueForCache,i_nMaxRows,o_nRowCount)
            ,m_aSqlParser( _rContext )
            ,m_aSqlIterator( i_xConnection, Reference<XTablesSupplier>(_xComposer,UNO_QUERY)->getTables(), m_aSqlParser, NULL )
            ,m_bResultSetChanged(false)
{
    SAL_INFO("dbaccess", "OptimisticSet::OptimisticSet" );
    DBG_CTOR(OptimisticSet,NULL);
}

OptimisticSet::~OptimisticSet()
{
    DBG_DTOR(OptimisticSet,NULL);
}

void OptimisticSet::construct(const Reference< XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter)
{
    SAL_INFO("dbaccess", "OptimisticSet::construct" );

    OCacheSet::construct(_xDriverSet,i_sRowSetFilter);

    initColumns();
    m_sRowSetFilter = i_sRowSetFilter;

    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    bool bCase = (xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers()) ? true : false;
    Reference<XColumnsSupplier> xQueryColSup(m_xComposer,UNO_QUERY);
    const Reference<XNameAccess> xQueryColumns = xQueryColSup->getColumns();
    const Reference<XTablesSupplier> xTabSup(m_xComposer,UNO_QUERY);
    const Reference<XNameAccess> xTables = xTabSup->getTables();
    const Sequence< OUString> aTableNames = xTables->getElementNames();
    const OUString* pTableNameIter = aTableNames.getConstArray();
    const OUString* pTableNameEnd = pTableNameIter + aTableNames.getLength();
    for( ; pTableNameIter != pTableNameEnd ; ++pTableNameIter)
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SelectColumnsMetaData> pKeyColumNames(new SelectColumnsMetaData(bCase));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        findTableColumnsMatching_throw(xTables->getByName(*pTableNameIter),*pTableNameIter,xMeta,xQueryColumns,pKeyColumNames);
        m_pKeyColumnNames->insert(pKeyColumNames->begin(),pKeyColumNames->end());
    }

    // the first row is empty because it's now easier for us to distinguish when we are beforefirst or first
    // without extra variable to be set
    OKeySetValue keySetValue((ORowSetValueVector *)NULL,::std::pair<sal_Int32,Reference<XRow> >(0,(Reference<XRow>)NULL));
    m_aKeyMap.insert(OKeySetMatrix::value_type(0,keySetValue));
    m_aKeyIter = m_aKeyMap.begin();

    Reference< XSingleSelectQueryComposer> xSourceComposer(m_xComposer,UNO_QUERY);
    Reference< XMultiServiceFactory >  xFactory(m_xConnection, UNO_QUERY_THROW);
    Reference<XSingleSelectQueryComposer> xAnalyzer(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
    OUString sQuery = xSourceComposer->getQuery();
    xAnalyzer->setElementaryQuery(xSourceComposer->getElementaryQuery());
    // check for joins
    OUString aErrorMsg;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<OSQLParseNode> pStatementNode( m_aSqlParser.parseTree( aErrorMsg, sQuery ) );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    m_aSqlIterator.setParseTree( pStatementNode.get() );
    m_aSqlIterator.traverseAll();
    fillJoinedColumns_throw(m_aSqlIterator.getJoinConditions());

}

void OptimisticSet::makeNewStatement( )
{
    OUStringBuffer aFilter = createKeyFilter();

    Reference< XSingleSelectQueryComposer> xSourceComposer(m_xComposer,UNO_QUERY);
    Reference< XMultiServiceFactory >  xFactory(m_xConnection, UNO_QUERY_THROW);
    Reference<XSingleSelectQueryComposer> xAnalyzer(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
    xAnalyzer->setElementaryQuery(xSourceComposer->getElementaryQuery());

    const OUString sComposerFilter = m_xComposer->getFilter();
    if ( !m_sRowSetFilter.isEmpty() || !sComposerFilter.isEmpty() )
    {
        FilterCreator aFilterCreator;
        if ( !sComposerFilter.isEmpty() && sComposerFilter != m_sRowSetFilter )
            aFilterCreator.append( sComposerFilter );
        aFilterCreator.append( m_sRowSetFilter );
        aFilterCreator.append( aFilter.makeStringAndClear() );
        aFilter = aFilterCreator.getComposedAndClear();
    }
    xAnalyzer->setFilter(aFilter.makeStringAndClear());
    m_xStatement = m_xConnection->prepareStatement(xAnalyzer->getQueryWithSubstitution());
    ::comphelper::disposeComponent(xAnalyzer);
}

// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL OptimisticSet::deleteRows( const Sequence< Any >& /*rows*/ ,const connectivity::OSQLTable& /*_xTable*/) throw(SQLException, RuntimeException)
{
    Sequence< sal_Int32 > aRet;
    return aRet;
}

void SAL_CALL OptimisticSet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& /*_xTable*/  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "OptimisticSet::updateRow" );
    if ( m_aJoinedKeyColumns.empty() )
        throw SQLException();
    // list all cloumns that should be set
    static OUString s_sPara(" = ?");
    OUString aQuote  = getIdentifierQuoteString();

    ::std::map< OUString,bool > aResultSetChanged;
    TSQLStatements aKeyConditions;
    TSQLStatements aIndexConditions;
    TSQLStatements aSql;

    // here we build the condition part for the update statement
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(;aIter != aEnd;++aIter)
    {
        if ( aResultSetChanged.find( aIter->second.sTableName ) == aResultSetChanged.end() )
            aResultSetChanged[aIter->second.sTableName] = false;
        const OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,aIter->second.sRealName);
        if ( m_pKeyColumnNames->find(aIter->first) != m_pKeyColumnNames->end() )
        {
            aResultSetChanged[aIter->second.sTableName] = m_aJoinedKeyColumns.find(aIter->second.nPosition) != m_aJoinedKeyColumns.end();
            lcl_fillKeyCondition(aIter->second.sTableName,sQuotedColumnName,(_rOrginalRow->get())[aIter->second.nPosition],aKeyConditions);
        }
        if((_rInsertRow->get())[aIter->second.nPosition].isModified())
        {
            if ( m_aJoinedKeyColumns.find(aIter->second.nPosition) != m_aJoinedKeyColumns.end() )
                throw SQLException();

            ::std::map<sal_Int32,sal_Int32>::const_iterator aJoinIter = m_aJoinedColumns.find(aIter->second.nPosition);
            if ( aJoinIter != m_aJoinedColumns.end() )
            {
                (_rInsertRow->get())[aJoinIter->second] = (_rInsertRow->get())[aIter->second.nPosition];
            }
            OUStringBuffer& rPart = aSql[aIter->second.sTableName];
            if ( !rPart.isEmpty() )
                rPart.append(", ");
            rPart.append(sQuotedColumnName + s_sPara);
        }
    }

    if( aSql.empty() )
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_VALUE_CHANGED ), SQL_GENERAL_ERROR, m_xConnection );

    if( aKeyConditions.empty() )
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_CONDITION_FOR_PK ), SQL_GENERAL_ERROR, m_xConnection );

    static const OUString s_sUPDATE("UPDATE ");
    static const OUString s_sSET(" SET ");

    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();

    TSQLStatements::iterator aSqlIter = aSql.begin();
    TSQLStatements::iterator aSqlEnd  = aSql.end();
    for(;aSqlIter != aSqlEnd ; ++aSqlIter)
    {
        if ( !aSqlIter->second.isEmpty() )
        {
            m_bResultSetChanged = m_bResultSetChanged || aResultSetChanged[aSqlIter->first];
            OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(xMetaData,aSqlIter->first,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
            OUStringBuffer sSql(s_sUPDATE + ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable ) +
                                       s_sSET + aSqlIter->second.toString());
            OUStringBuffer& rCondition = aKeyConditions[aSqlIter->first];
            if ( !rCondition.isEmpty() )
                sSql.append(" WHERE " + rCondition.toString() );

            executeUpdate(_rInsertRow ,_rOrginalRow,sSql.makeStringAndClear(),aSqlIter->first);
        }
    }
}

void SAL_CALL OptimisticSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& /*_xTable*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "OptimisticSet::insertRow" );
    TSQLStatements aSql;
    TSQLStatements aParameter;
    TSQLStatements aKeyConditions;
    ::std::map< OUString,bool > aResultSetChanged;
    OUString aQuote  = getIdentifierQuoteString();

    // here we build the condition part for the update statement
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(;aIter != aEnd;++aIter)
    {
        if ( aResultSetChanged.find( aIter->second.sTableName ) == aResultSetChanged.end() )
            aResultSetChanged[aIter->second.sTableName] = false;

        const OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,aIter->second.sRealName);
        if ( (_rInsertRow->get())[aIter->second.nPosition].isModified() )
        {
            if ( m_aJoinedKeyColumns.find(aIter->second.nPosition) != m_aJoinedKeyColumns.end() )
            {
                lcl_fillKeyCondition(aIter->second.sTableName,sQuotedColumnName,(_rInsertRow->get())[aIter->second.nPosition],aKeyConditions);
                aResultSetChanged[aIter->second.sTableName] = true;
            }
            ::std::map<sal_Int32,sal_Int32>::const_iterator aJoinIter = m_aJoinedColumns.find(aIter->second.nPosition);
            if ( aJoinIter != m_aJoinedColumns.end() )
            {
                (_rInsertRow->get())[aJoinIter->second] = (_rInsertRow->get())[aIter->second.nPosition];
            }
            OUStringBuffer& rPart = aSql[aIter->second.sTableName];
            if ( !rPart.isEmpty() )
                rPart.append(", ");
            rPart.append(sQuotedColumnName);
            OUStringBuffer& rParam = aParameter[aIter->second.sTableName];
            if ( !rParam.isEmpty() )
                rParam.append(", ");
            rParam.append("?");
        }
    }
    if ( aParameter.empty() )
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_VALUE_CHANGED ), SQL_GENERAL_ERROR, m_xConnection );

    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    static const OUString s_sINSERT("INSERT INTO ");
    static const OUString s_sVALUES(") VALUES ( ");
    TSQLStatements::iterator aSqlIter = aSql.begin();
    TSQLStatements::iterator aSqlEnd  = aSql.end();
    for(;aSqlIter != aSqlEnd ; ++aSqlIter)
    {
        if ( !aSqlIter->second.isEmpty() )
        {
            m_bResultSetChanged = m_bResultSetChanged || aResultSetChanged[aSqlIter->first];
            OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(xMetaData,aSqlIter->first,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
            OUString sComposedTableName = ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable );
            OUString sSql(s_sINSERT + sComposedTableName + " ( " + aSqlIter->second.toString() +
                                 s_sVALUES + aParameter[aSqlIter->first].toString() + " )");

            OUStringBuffer& rCondition = aKeyConditions[aSqlIter->first];
            if ( !rCondition.isEmpty() )
            {
                OUString sQuery("SELECT " + aSqlIter->second.toString() + " FROM " + sComposedTableName +
                                       " WHERE " + rCondition.toString());

                try
                {
                    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(sQuery));
                    Reference< XParameters > xParameter(xPrep,UNO_QUERY);
                    // and then the values of the where condition
                    SelectColumnsMetaData::iterator aKeyCol = m_pKeyColumnNames->begin();
                    SelectColumnsMetaData::iterator aKeysEnd = m_pKeyColumnNames->end();
                    sal_Int32 i = 1;
                    for(;aKeyCol != aKeysEnd;++aKeyCol)
                    {
                        if ( aKeyCol->second.sTableName == aSqlIter->first )
                        {
                            setParameter(i++,xParameter,(_rInsertRow->get())[aKeyCol->second.nPosition],aKeyCol->second.nType,aKeyCol->second.nScale);
                        }
                    }
                    Reference<XResultSet> xRes = xPrep->executeQuery();
                    Reference<XRow> xRow(xRes,UNO_QUERY);
                    if ( xRow.is() && xRes->next() )
                    {
                        m_bResultSetChanged = true;
                        continue;
                    }
                }
                catch(const SQLException&)
                {
                }
            }

            executeInsert(_rInsertRow,sSql,aSqlIter->first);
        }
    }
}

void SAL_CALL OptimisticSet::deleteRow(const ORowSetRow& _rDeleteRow,const connectivity::OSQLTable& /*_xTable*/   ) throw(SQLException, RuntimeException)
{
    OUString aQuote  = getIdentifierQuoteString();
    TSQLStatements aKeyConditions;
    TSQLStatements aIndexConditions;
    TSQLStatements aSql;

    // here we build the condition part for the update statement
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(;aIter != aEnd;++aIter)
    {
        if ( m_aJoinedKeyColumns.find(aIter->second.nPosition) == m_aJoinedKeyColumns.end() && m_pKeyColumnNames->find(aIter->first) != m_pKeyColumnNames->end() )
        {
            // only delete rows which aren't the key in the join
            const OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,aIter->second.sRealName);
            lcl_fillKeyCondition(aIter->second.sTableName,sQuotedColumnName,(_rDeleteRow->get())[aIter->second.nPosition],aKeyConditions);
        }
    }
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    TSQLStatements::iterator aSqlIter = aKeyConditions.begin();
    TSQLStatements::iterator aSqlEnd  = aKeyConditions.end();
    for(;aSqlIter != aSqlEnd ; ++aSqlIter)
    {
        OUStringBuffer& rCondition = aSqlIter->second;
        if ( !rCondition.isEmpty() )
        {
            OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(xMetaData,aSqlIter->first,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
            OUString sSql("DELETE FROM " + ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable ) +
                                 " WHERE " + rCondition.toString() );
            executeDelete(_rDeleteRow, sSql, aSqlIter->first);
        }
    }
}

void OptimisticSet::executeDelete(const ORowSetRow& _rDeleteRow,const OUString& i_sSQL,const OUString& i_sTableName)
{
    SAL_INFO("dbaccess", "OptimisticSet::executeDelete" );

    // now create and execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(i_sSQL));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    SelectColumnsMetaData::const_iterator aIter = m_pKeyColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pKeyColumnNames->end();
    sal_Int32 i = 1;
    for(;aIter != aEnd;++aIter)
    {
        if ( aIter->second.sTableName == i_sTableName )
            setParameter(i++,xParameter,(_rDeleteRow->get())[aIter->second.nPosition],aIter->second.nType,aIter->second.nScale);
    }
    m_bDeleted = xPrep->executeUpdate() > 0;

    if(m_bDeleted)
    {
        sal_Int32 nBookmark = ::comphelper::getINT32((_rDeleteRow->get())[0].getAny());
        if(m_aKeyIter == m_aKeyMap.find(nBookmark) && m_aKeyIter != m_aKeyMap.end())
            ++m_aKeyIter;
        m_aKeyMap.erase(nBookmark);
        m_bDeleted = sal_True;
    }
}

void OptimisticSet::fillJoinedColumns_throw(const ::std::vector< TNodePair >& i_aJoinColumns)
{
    ::std::vector< TNodePair >::const_iterator aIter = i_aJoinColumns.begin();
    for(;aIter != i_aJoinColumns.end();++aIter)
    {
        OUString sColumnName,sTableName;
        m_aSqlIterator.getColumnRange(aIter->first,sColumnName,sTableName);
        OUString sLeft(sTableName + "." + sColumnName);
        m_aSqlIterator.getColumnRange(aIter->second,sColumnName,sTableName);
        OUString sRight(sTableName + "." + sColumnName);
        fillJoinedColumns_throw(sLeft, sRight);
    }
}

void OptimisticSet::fillJoinedColumns_throw(const OUString& i_sLeftColumn,const OUString& i_sRightColumn)
{
    sal_Int32 nLeft = 0,nRight = 0;
    SelectColumnsMetaData::const_iterator aLeftIter  = m_pKeyColumnNames->find(i_sLeftColumn);
    SelectColumnsMetaData::const_iterator aRightIter = m_pKeyColumnNames->find(i_sRightColumn);

    bool bLeftKey = aLeftIter != m_pKeyColumnNames->end();
    if ( bLeftKey )
    {
        nLeft = aLeftIter->second.nPosition;
    }
    else
    {
        aLeftIter = m_pColumnNames->find(i_sLeftColumn);
        if ( aLeftIter != m_pColumnNames->end() )
            nLeft = aLeftIter->second.nPosition;
    }

    bool bRightKey = aRightIter != m_pKeyColumnNames->end();
    if ( bRightKey )
    {
        nRight = aRightIter->second.nPosition;
    }
    else
    {
        aRightIter = m_pColumnNames->find(i_sRightColumn);
        if ( aRightIter != m_pColumnNames->end() )
            nRight = aRightIter->second.nPosition;
    }

    if (bLeftKey)
        m_aJoinedKeyColumns[nLeft] = nRight;
    else
        m_aJoinedColumns[nLeft] = nRight;
    if (bRightKey)
        m_aJoinedKeyColumns[nRight] = nLeft;
    else
        m_aJoinedColumns[nRight] = nLeft;
}

bool OptimisticSet::isResultSetChanged() const
{
    bool bOld = m_bResultSetChanged;
    m_bResultSetChanged = false;
    return bOld;
}

void OptimisticSet::reset(const Reference< XResultSet>& _xDriverSet)
{
    OCacheSet::construct(_xDriverSet,OUString());
    m_bRowCountFinal = sal_False;
    m_aKeyMap.clear();
    OKeySetValue keySetValue((ORowSetValueVector *)NULL,::std::pair<sal_Int32,Reference<XRow> >(0,(Reference<XRow>)NULL));
    m_aKeyMap.insert(OKeySetMatrix::value_type(0,keySetValue));
    m_aKeyIter = m_aKeyMap.begin();
}

void OptimisticSet::mergeColumnValues(sal_Int32 i_nColumnIndex,ORowSetValueVector::Vector& io_aInsertRow,ORowSetValueVector::Vector& io_aRow,::std::vector<sal_Int32>& o_aChangedColumns)
{
    o_aChangedColumns.push_back(i_nColumnIndex);
    ::std::map<sal_Int32,sal_Int32>::const_iterator aJoinIter = m_aJoinedColumns.find(i_nColumnIndex);
    if ( aJoinIter != m_aJoinedColumns.end() )
    {
        io_aRow[aJoinIter->second] = io_aRow[i_nColumnIndex];
        io_aInsertRow[aJoinIter->second] = io_aInsertRow[i_nColumnIndex];
        io_aRow[aJoinIter->second].setModified();
        o_aChangedColumns.push_back(aJoinIter->second);
    }
}

namespace
{
    struct PositionFunctor : ::std::unary_function<SelectColumnsMetaData::value_type,bool>
    {
        sal_Int32 m_nPos;
        PositionFunctor(sal_Int32 i_nPos)
            : m_nPos(i_nPos)
        {
        }

        inline bool operator()(const SelectColumnsMetaData::value_type& _aType)
        {
            return m_nPos == _aType.second.nPosition;
        }
    };
    struct TableNameFunctor : ::std::unary_function<SelectColumnsMetaData::value_type,bool>
    {
        OUString m_sTableName;
        TableNameFunctor(const OUString& i_sTableName)
            : m_sTableName(i_sTableName)
        {
        }

        inline bool operator()(const SelectColumnsMetaData::value_type& _aType)
        {
            return m_sTableName == _aType.second.sTableName;
        }
    };
}

bool OptimisticSet::updateColumnValues(const ORowSetValueVector::Vector& io_aCachedRow,ORowSetValueVector::Vector& io_aRow,const ::std::vector<sal_Int32>& i_aChangedColumns)
{
    bool bRet = false;
    ::std::vector<sal_Int32>::const_iterator aColIdxIter = i_aChangedColumns.begin();
    for(;aColIdxIter != i_aChangedColumns.end();++aColIdxIter)
    {
        SelectColumnsMetaData::const_iterator aFind = ::std::find_if(m_pKeyColumnNames->begin(),m_pKeyColumnNames->end(),PositionFunctor(*aColIdxIter));
        if ( aFind != m_pKeyColumnNames->end() )
        {
            const OUString sTableName = aFind->second.sTableName;
            aFind = ::std::find_if(m_pKeyColumnNames->begin(),m_pKeyColumnNames->end(),TableNameFunctor(sTableName));
            while( aFind != m_pKeyColumnNames->end() )
            {
                io_aRow[aFind->second.nPosition].setSigned(io_aCachedRow[aFind->second.nPosition].isSigned());
                if ( io_aCachedRow[aFind->second.nPosition] != io_aRow[aFind->second.nPosition] )
                    break;
                ++aFind;
            }
            if ( aFind == m_pKeyColumnNames->end() )
            {
                bRet = true;
                SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
                SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
                for ( ;aIter != aEnd;++aIter )
                {
                    if ( aIter->second.sTableName == sTableName )
                    {
                        io_aRow[aIter->second.nPosition] = io_aCachedRow[aIter->second.nPosition];
                        io_aRow[aIter->second.nPosition].setModified();
                    }
                }
            }
        }
    }
    return bRet;
}

bool OptimisticSet::columnValuesUpdated(ORowSetValueVector::Vector& o_aCachedRow,const ORowSetValueVector::Vector& i_aRow)
{
    bool bRet = false;
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(;aIter != aEnd;++aIter)
    {
        SelectColumnsMetaData::const_iterator aFind = ::std::find_if(m_pKeyColumnNames->begin(),m_pKeyColumnNames->end(),PositionFunctor(aIter->second.nPosition));
        if ( aFind != m_pKeyColumnNames->end() )
        {
            const OUString sTableName = aFind->second.sTableName;
            aFind = ::std::find_if(m_pKeyColumnNames->begin(),m_pKeyColumnNames->end(),TableNameFunctor(sTableName));
            while( aFind != m_pKeyColumnNames->end() )
            {
                o_aCachedRow[aFind->second.nPosition].setSigned(i_aRow[aFind->second.nPosition].isSigned());
                if ( o_aCachedRow[aFind->second.nPosition] != i_aRow[aFind->second.nPosition] )
                    break;
                ++aFind;
            }
            if ( aFind == m_pKeyColumnNames->end() )
            {
                bRet = true;
                SelectColumnsMetaData::const_iterator aIter2 = m_pColumnNames->begin();
                SelectColumnsMetaData::const_iterator aEnd2 = m_pColumnNames->end();
                for ( ;aIter2 != aEnd2;++aIter2 )
                {
                    if ( aIter2->second.sTableName == sTableName )
                    {
                        o_aCachedRow[aIter2->second.nPosition] = i_aRow[aIter2->second.nPosition];
                        o_aCachedRow[aIter2->second.nPosition].setModified();
                    }
                }
                fillMissingValues(o_aCachedRow);
            }
        }
    }
    return bRet;
}

void OptimisticSet::fillMissingValues(ORowSetValueVector::Vector& io_aRow) const
{
    TSQLStatements aSql;
    TSQLStatements aKeyConditions;
    ::std::map< OUString,bool > aResultSetChanged;
    OUString aQuote  = getIdentifierQuoteString();
    // here we build the condition part for the update statement
    SelectColumnsMetaData::const_iterator aColIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aColEnd = m_pColumnNames->end();
    for(;aColIter != aColEnd;++aColIter)
    {
        const OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,aColIter->second.sRealName);
        if ( m_aJoinedKeyColumns.find(aColIter->second.nPosition) != m_aJoinedKeyColumns.end() )
        {
            lcl_fillKeyCondition(aColIter->second.sTableName,sQuotedColumnName,io_aRow[aColIter->second.nPosition],aKeyConditions);
        }
        OUStringBuffer& rPart = aSql[aColIter->second.sTableName];
        if ( !rPart.isEmpty() )
            rPart.append(", ");
        rPart.append(sQuotedColumnName);
    }
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    TSQLStatements::iterator aSqlIter = aSql.begin();
    TSQLStatements::iterator aSqlEnd  = aSql.end();
    for(;aSqlIter != aSqlEnd ; ++aSqlIter)
    {
        if ( !aSqlIter->second.isEmpty() )
        {
            OUStringBuffer& rCondition = aKeyConditions[aSqlIter->first];
            if ( !rCondition.isEmpty() )
            {
                OUString sCatalog,sSchema,sTable;
                ::dbtools::qualifiedNameComponents(xMetaData,aSqlIter->first,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
                OUString sComposedTableName = ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable );
                OUString sQuery("SELECT " + aSqlIter->second.toString() + " FROM " + sComposedTableName + " WHERE " +
                                       rCondition.makeStringAndClear());

                try
                {
                    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(sQuery));
                    Reference< XParameters > xParameter(xPrep,UNO_QUERY);
                    // and then the values of the where condition
                    SelectColumnsMetaData::iterator aKeyIter = m_pKeyColumnNames->begin();
                    SelectColumnsMetaData::iterator aKeyEnd = m_pKeyColumnNames->end();
                    sal_Int32 i = 1;
                    for(;aKeyIter != aKeyEnd;++aKeyIter)
                    {
                        if ( aKeyIter->second.sTableName == aSqlIter->first )
                        {
                            setParameter(i++,xParameter,io_aRow[aKeyIter->second.nPosition],aKeyIter->second.nType,aKeyIter->second.nScale);
                        }
                    }
                    Reference<XResultSet> xRes = xPrep->executeQuery();
                    Reference<XRow> xRow(xRes,UNO_QUERY);
                    if ( xRow.is() && xRes->next() )
                    {
                        i = 1;
                        aColIter = m_pColumnNames->begin();
                        for(;aColIter != aColEnd;++aColIter)
                        {
                            if ( aColIter->second.sTableName == aSqlIter->first )
                            {
                                io_aRow[aColIter->second.nPosition].fill(i++, aColIter->second.nType, xRow);
                                io_aRow[aColIter->second.nPosition].setModified();
                            }
                        }
                    }
                }
                catch(const SQLException&)
                {
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
