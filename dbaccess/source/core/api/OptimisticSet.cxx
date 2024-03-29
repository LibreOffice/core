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


#include <memory>
#include "OptimisticSet.hxx"
#include <core_resource.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <map>
#include <algorithm>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <composertools.hxx>

using namespace dbaccess;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star;

typedef std::map<OUString, OUStringBuffer> TSQLStatements;
namespace
{
    void lcl_fillKeyCondition(const OUString& i_sTableName,std::u16string_view i_sQuotedColumnName,const ORowSetValue& i_aValue,TSQLStatements& io_aKeyConditions)
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


OptimisticSet::OptimisticSet(const Reference<XComponentContext>& _rContext,
                             const Reference< XConnection>& i_xConnection,
                             const Reference< XSingleSelectQueryAnalyzer >& _xComposer,
                             const ORowSetValueVector& _aParameterValueForCache,
                             sal_Int32 i_nMaxRows,
                             sal_Int32& o_nRowCount)
            :OKeySet(nullptr,OUString(),_xComposer,_aParameterValueForCache,i_nMaxRows,o_nRowCount)
            ,m_aSqlParser( _rContext )
            ,m_aSqlIterator( i_xConnection, Reference<XTablesSupplier>(_xComposer,UNO_QUERY_THROW)->getTables(), m_aSqlParser )
            ,m_bResultSetChanged(false)
{
}

OptimisticSet::~OptimisticSet()
{
}

void OptimisticSet::construct(const Reference< XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter)
{
    OCacheSet::construct(_xDriverSet,i_sRowSetFilter);

    initColumns();
    m_sRowSetFilter = i_sRowSetFilter;

    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    bool bCase = xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers();
    Reference<XColumnsSupplier> xQueryColSup(m_xComposer,UNO_QUERY);
    const Reference<XNameAccess> xQueryColumns = xQueryColSup->getColumns();
    const Reference<XTablesSupplier> xTabSup(m_xComposer,UNO_QUERY);
    const Reference<XNameAccess> xTables = xTabSup->getTables();
    const Sequence< OUString> aTableNames = xTables->getElementNames();
    const OUString* pTableNameIter = aTableNames.getConstArray();
    const OUString* pTableNameEnd = pTableNameIter + aTableNames.getLength();
    for( ; pTableNameIter != pTableNameEnd ; ++pTableNameIter)
    {
        std::unique_ptr<SelectColumnsMetaData> pKeyColumNames(new SelectColumnsMetaData(bCase));
        findTableColumnsMatching_throw(xTables->getByName(*pTableNameIter),*pTableNameIter,xMeta,xQueryColumns,pKeyColumNames);
        m_pKeyColumnNames->insert(pKeyColumNames->begin(),pKeyColumNames->end());
    }

    // the first row is empty because it's now easier for us to distinguish when we are beforefirst or first
    // without extra variable to be set
    OKeySetValue keySetValue(nullptr,std::pair<sal_Int32,Reference<XRow> >(0,Reference<XRow>()));
    m_aKeyMap.emplace(0,keySetValue);
    m_aKeyIter = m_aKeyMap.begin();

    Reference< XSingleSelectQueryComposer> xSourceComposer(m_xComposer,UNO_QUERY);
    Reference< XMultiServiceFactory >  xFactory(m_xConnection, UNO_QUERY_THROW);
    Reference<XSingleSelectQueryComposer> xAnalyzer(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
    OUString sQuery = xSourceComposer->getQuery();
    xAnalyzer->setElementaryQuery(xSourceComposer->getElementaryQuery());
    // check for joins
    OUString aErrorMsg;
    std::unique_ptr<OSQLParseNode> pStatementNode( m_aSqlParser.parseTree( aErrorMsg, sQuery ) );
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

void OptimisticSet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOriginalRow,const connectivity::OSQLTable& /*_xTable*/  )
{
    if ( m_aJoinedKeyColumns.empty() )
        throw SQLException();
    // list all columns that should be set
    OUString aQuote  = getIdentifierQuoteString();

    std::map< OUString,bool > aResultSetChanged;
    TSQLStatements aKeyConditions;
    TSQLStatements aSql;

    // here we build the condition part for the update statement
    for (auto const& columnName : *m_pColumnNames)
    {
        aResultSetChanged.try_emplace(columnName.second.sTableName, false);
        const OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,columnName.second.sRealName);
        if ( m_pKeyColumnNames->find(columnName.first) != m_pKeyColumnNames->end() )
        {
            aResultSetChanged[columnName.second.sTableName] = m_aJoinedKeyColumns.find(columnName.second.nPosition) != m_aJoinedKeyColumns.end();
            lcl_fillKeyCondition(columnName.second.sTableName,sQuotedColumnName,(*_rOriginalRow)[columnName.second.nPosition],aKeyConditions);
        }
        if((*_rInsertRow)[columnName.second.nPosition].isModified())
        {
            if ( m_aJoinedKeyColumns.find(columnName.second.nPosition) != m_aJoinedKeyColumns.end() )
                throw SQLException();

            std::map<sal_Int32,sal_Int32>::const_iterator aJoinIter = m_aJoinedColumns.find(columnName.second.nPosition);
            if ( aJoinIter != m_aJoinedColumns.end() )
            {
                (*_rInsertRow)[aJoinIter->second] = (*_rInsertRow)[columnName.second.nPosition];
            }
            OUStringBuffer& rPart = aSql[columnName.second.sTableName];
            if ( !rPart.isEmpty() )
                rPart.append(", ");
            rPart.append(sQuotedColumnName + " = ?");
        }
    }

    if( aSql.empty() )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_VALUE_CHANGED ), StandardSQLState::GENERAL_ERROR, m_xConnection );

    if( aKeyConditions.empty() )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_CONDITION_FOR_PK ), StandardSQLState::GENERAL_ERROR, m_xConnection );

    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();

    for (auto const& elem : aSql)
    {
        if ( !elem.second.isEmpty() )
        {
            m_bResultSetChanged = m_bResultSetChanged || aResultSetChanged[elem.first];
            OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(xMetaData,elem.first,sCatalog,sSchema,sTable,::dbtools::EComposeRule::InDataManipulation);
            OUStringBuffer sSql("UPDATE " + ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable ) +
                                       " SET " + elem.second);
            OUStringBuffer& rCondition = aKeyConditions[elem.first];
            if ( !rCondition.isEmpty() )
                sSql.append(" WHERE " + rCondition );

            executeUpdate(_rInsertRow ,_rOriginalRow,sSql.makeStringAndClear(),elem.first);
        }
    }
}

void OptimisticSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& /*_xTable*/ )
{
    TSQLStatements aSql;
    TSQLStatements aParameter;
    TSQLStatements aKeyConditions;
    std::map< OUString,bool > aResultSetChanged;
    OUString aQuote  = getIdentifierQuoteString();

    // here we build the condition part for the update statement
    for (auto const& columnName : *m_pColumnNames)
    {
        aResultSetChanged.try_emplace(columnName.second.sTableName, false);

        const OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,columnName.second.sRealName);
        if ( (*_rInsertRow)[columnName.second.nPosition].isModified() )
        {
            if ( m_aJoinedKeyColumns.find(columnName.second.nPosition) != m_aJoinedKeyColumns.end() )
            {
                lcl_fillKeyCondition(columnName.second.sTableName,sQuotedColumnName,(*_rInsertRow)[columnName.second.nPosition],aKeyConditions);
                aResultSetChanged[columnName.second.sTableName] = true;
            }
            std::map<sal_Int32,sal_Int32>::const_iterator aJoinIter = m_aJoinedColumns.find(columnName.second.nPosition);
            if ( aJoinIter != m_aJoinedColumns.end() )
            {
                (*_rInsertRow)[aJoinIter->second] = (*_rInsertRow)[columnName.second.nPosition];
            }
            OUStringBuffer& rPart = aSql[columnName.second.sTableName];
            if ( !rPart.isEmpty() )
                rPart.append(", ");
            rPart.append(sQuotedColumnName);
            OUStringBuffer& rParam = aParameter[columnName.second.sTableName];
            if ( !rParam.isEmpty() )
                rParam.append(", ");
            rParam.append("?");
        }
    }
    if ( aParameter.empty() )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_VALUE_CHANGED ), StandardSQLState::GENERAL_ERROR, m_xConnection );

    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    for (auto const& elem : aSql)
    {
        if ( !elem.second.isEmpty() )
        {
            m_bResultSetChanged = m_bResultSetChanged || aResultSetChanged[elem.first];
            OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(xMetaData,elem.first,sCatalog,sSchema,sTable,::dbtools::EComposeRule::InDataManipulation);
            OUString sComposedTableName = ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable );
            OUString sSql("INSERT INTO " + sComposedTableName + " ( " + elem.second +
                                 ") VALUES ( " + aParameter[elem.first] + " )");

            OUStringBuffer& rCondition = aKeyConditions[elem.first];
            if ( !rCondition.isEmpty() )
            {
                OUString sQuery("SELECT " + elem.second + " FROM " + sComposedTableName +
                                       " WHERE " + rCondition);

                try
                {
                    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(sQuery));
                    Reference< XParameters > xParameter(xPrep,UNO_QUERY);
                    // and then the values of the where condition
                    sal_Int32 i = 1;
                    for (auto const& keyColumnName : *m_pKeyColumnNames)
                    {
                        if ( keyColumnName.second.sTableName == elem.first )
                        {
                            setParameter(i++,xParameter,(*_rInsertRow)[keyColumnName.second.nPosition],keyColumnName.second.nType,keyColumnName.second.nScale);
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

            executeInsert(_rInsertRow,sSql,elem.first);
        }
    }
}

void OptimisticSet::deleteRow(const ORowSetRow& _rDeleteRow,const connectivity::OSQLTable& /*_xTable*/   )
{
    OUString aQuote  = getIdentifierQuoteString();
    TSQLStatements aKeyConditions;

    // here we build the condition part for the update statement
    for (auto const& columnName : *m_pColumnNames)
    {
        if ( m_aJoinedKeyColumns.find(columnName.second.nPosition) == m_aJoinedKeyColumns.end() && m_pKeyColumnNames->find(columnName.first) != m_pKeyColumnNames->end() )
        {
            // only delete rows which aren't the key in the join
            const OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,columnName.second.sRealName);
            lcl_fillKeyCondition(columnName.second.sTableName,sQuotedColumnName,(*_rDeleteRow)[columnName.second.nPosition],aKeyConditions);
        }
    }
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    for (auto & keyCondition : aKeyConditions)
    {
        OUStringBuffer& rCondition = keyCondition.second;
        if ( !rCondition.isEmpty() )
        {
            OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(xMetaData,keyCondition.first,sCatalog,sSchema,sTable,::dbtools::EComposeRule::InDataManipulation);
            OUString sSql("DELETE FROM " + ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable ) +
                                 " WHERE " + rCondition );
            executeDelete(_rDeleteRow, sSql, keyCondition.first);
        }
    }
}

void OptimisticSet::executeDelete(const ORowSetRow& _rDeleteRow,const OUString& i_sSQL,std::u16string_view i_sTableName)
{
    // now create and execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(i_sSQL));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    sal_Int32 i = 1;
    for (auto const& keyColumnName : *m_pKeyColumnNames)
    {
        if ( keyColumnName.second.sTableName == i_sTableName )
            setParameter(i++,xParameter,(*_rDeleteRow)[keyColumnName.second.nPosition],keyColumnName.second.nType,keyColumnName.second.nScale);
    }
    m_bDeleted = xPrep->executeUpdate() > 0;

    if(m_bDeleted)
    {
        sal_Int32 nBookmark = ::comphelper::getINT32((*_rDeleteRow)[0].getAny());
        const auto iter = m_aKeyMap.find(nBookmark);
        assert(iter != m_aKeyMap.end());
        if(m_aKeyIter == iter && m_aKeyIter != m_aKeyMap.end())
            ++m_aKeyIter;
        m_aKeyMap.erase(nBookmark);
        m_bDeleted = true;
    }
}

void OptimisticSet::fillJoinedColumns_throw(const std::vector< TNodePair >& i_aJoinColumns)
{
    for (auto const& joinColumn : i_aJoinColumns)
    {
        OUString sColumnName,sTableName;
        m_aSqlIterator.getColumnRange(joinColumn.first,sColumnName,sTableName);
        OUString sLeft(sTableName + "." + sColumnName);
        m_aSqlIterator.getColumnRange(joinColumn.second,sColumnName,sTableName);
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

void OptimisticSet::mergeColumnValues(sal_Int32 i_nColumnIndex,ORowSetValueVector::Vector& io_aInsertRow,ORowSetValueVector::Vector& io_aRow,std::vector<sal_Int32>& o_aChangedColumns)
{
    o_aChangedColumns.push_back(i_nColumnIndex);
    std::map<sal_Int32,sal_Int32>::const_iterator aJoinIter = m_aJoinedColumns.find(i_nColumnIndex);
    if ( aJoinIter != m_aJoinedColumns.end() )
    {
        io_aRow[aJoinIter->second] = io_aRow[i_nColumnIndex];
        io_aInsertRow[aJoinIter->second] = io_aInsertRow[i_nColumnIndex];
        io_aRow[aJoinIter->second].setModified(true);
        o_aChangedColumns.push_back(aJoinIter->second);
    }
}

bool OptimisticSet::updateColumnValues(const ORowSetValueVector::Vector& io_aCachedRow,ORowSetValueVector::Vector& io_aRow,const std::vector<sal_Int32>& i_aChangedColumns)
{
    bool bRet = false;
    for( const auto& aColIdx : i_aChangedColumns )
    {
        SelectColumnsMetaData::const_iterator aFind = std::find_if(
            m_pKeyColumnNames->begin(),m_pKeyColumnNames->end(),
            [&aColIdx]( const SelectColumnsMetaData::value_type& aType )
            { return aType.second.nPosition == aColIdx; } );
        if ( aFind != m_pKeyColumnNames->end() )
        {
            const OUString sTableName = aFind->second.sTableName;
            aFind = std::find_if( m_pKeyColumnNames->begin(),m_pKeyColumnNames->end(),
                                    [&sTableName]
                                    ( const SelectColumnsMetaData::value_type& rCurr )
                                    { return rCurr.second.sTableName == sTableName; } );
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
                for( const auto& aCol : *m_pColumnNames )
                {
                    if ( aCol.second.sTableName == sTableName )
                    {
                        io_aRow[aCol.second.nPosition] = io_aCachedRow[aCol.second.nPosition];
                        io_aRow[aCol.second.nPosition].setModified(true);
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
    for( const auto& aCol : *m_pColumnNames )
    {
        sal_Int32 nPos = aCol.second.nPosition;
        SelectColumnsMetaData::const_iterator aFind = std::find_if(
            m_pKeyColumnNames->begin(),m_pKeyColumnNames->end(),
            [&nPos] ( const SelectColumnsMetaData::value_type& aType )
            { return aType.second.nPosition == nPos; } );
        if ( aFind != m_pKeyColumnNames->end() )
        {
            const OUString sTableName = aFind->second.sTableName;
            aFind = std::find_if( m_pKeyColumnNames->begin(),m_pKeyColumnNames->end(),
                                    [&sTableName]
                                    ( const SelectColumnsMetaData::value_type& rCurr )
                                    { return rCurr.second.sTableName == sTableName; } );
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
                for( const auto& aCol2 : *m_pColumnNames )
                {
                    if ( aCol2.second.sTableName == sTableName )
                    {
                        o_aCachedRow[aCol2.second.nPosition] = i_aRow[aCol2.second.nPosition];
                        o_aCachedRow[aCol2.second.nPosition].setModified(true);
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
    OUString aQuote  = getIdentifierQuoteString();
    // here we build the condition part for the update statement
    for (auto const& columnName : *m_pColumnNames)
    {
        const OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,columnName.second.sRealName);
        if ( m_aJoinedKeyColumns.find(columnName.second.nPosition) != m_aJoinedKeyColumns.end() )
        {
            lcl_fillKeyCondition(columnName.second.sTableName,sQuotedColumnName,io_aRow[columnName.second.nPosition],aKeyConditions);
        }
        OUStringBuffer& rPart = aSql[columnName.second.sTableName];
        if ( !rPart.isEmpty() )
            rPart.append(", ");
        rPart.append(sQuotedColumnName);
    }
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    for (auto const& elem : aSql)
    {
        if ( !elem.second.isEmpty() )
        {
            OUStringBuffer& rCondition = aKeyConditions[elem.first];
            if ( !rCondition.isEmpty() )
            {
                OUString sCatalog,sSchema,sTable;
                ::dbtools::qualifiedNameComponents(xMetaData,elem.first,sCatalog,sSchema,sTable,::dbtools::EComposeRule::InDataManipulation);
                OUString sComposedTableName = ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable );
                OUString sQuery("SELECT " + elem.second + " FROM " + sComposedTableName + " WHERE " + rCondition);
                rCondition.setLength(0);

                try
                {
                    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(sQuery));
                    Reference< XParameters > xParameter(xPrep,UNO_QUERY);
                    // and then the values of the where condition
                    sal_Int32 i = 1;
                    for (auto const& keyColumn : *m_pKeyColumnNames)
                    {
                        if ( keyColumn.second.sTableName == elem.first )
                        {
                            setParameter(i++,xParameter,io_aRow[keyColumn.second.nPosition],keyColumn.second.nType,keyColumn.second.nScale);
                        }
                    }
                    Reference<XResultSet> xRes = xPrep->executeQuery();
                    Reference<XRow> xRow(xRes,UNO_QUERY);
                    if ( xRow.is() && xRes->next() )
                    {
                        i = 1;
                        for (auto const& columnName : *m_pColumnNames)
                        {
                            if ( columnName.second.sTableName == elem.first )
                            {
                                io_aRow[columnName.second.nPosition].fill(i++, columnName.second.nType, xRow);
                                io_aRow[columnName.second.nPosition].setModified(true);
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
