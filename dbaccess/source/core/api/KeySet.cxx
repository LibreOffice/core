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
#include "KeySet.hxx"
#include <sal/log.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <stringconstants.hxx>
#include <apitools.hxx>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
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
#include <querycomposer.hxx>
#include <composertools.hxx>
#include "PrivateRow.hxx"

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
using std::vector;

namespace
{
    void lcl_fillIndexColumns(const Reference<XIndexAccess>& _xIndexes, std::vector< Reference<XNameAccess> >& _rAllIndexColumns)
    {
        if ( _xIndexes.is() )
        {
            Reference<XPropertySet> xIndexColsSup;
            sal_Int32 nCount = _xIndexes->getCount();
            for(sal_Int32 j = 0 ; j < nCount ; ++j)
            {
                xIndexColsSup.set(_xIndexes->getByIndex(j),UNO_QUERY);
                if( xIndexColsSup.is()
                    && comphelper::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISUNIQUE))
                    && !comphelper::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISPRIMARYKEYINDEX))
                )
                    _rAllIndexColumns.push_back(Reference<XColumnsSupplier>(xIndexColsSup,UNO_QUERY)->getColumns());
            }
        }
    }

    template < typename T > void tryDispose( Reference<T> &r )
    {
        try
        {
            ::comphelper::disposeComponent(r);
        }
        catch(const Exception&)
        {
            r = nullptr;
        }
        catch(...)
        {
            SAL_WARN("dbaccess", "Unknown Exception occurred");
        }
    }
}


OKeySet::OKeySet(const connectivity::OSQLTable& _xTable,
                 const Reference< XIndexAccess>& _xTableKeys,
                 const OUString& _rUpdateTableName,    // this can be the alias or the full qualified name
                 const Reference< XSingleSelectQueryAnalyzer >& _xComposer,
                 const ORowSetValueVector& _aParameterValueForCache,
                 sal_Int32 i_nMaxRows,
                 sal_Int32& o_nRowCount)
            :OCacheSet(i_nMaxRows)
            ,m_aParameterValueForCache(new ORowSetValueVector(_aParameterValueForCache))
            ,m_xTable(_xTable)
            ,m_xTableKeys(_xTableKeys)
            ,m_xComposer(_xComposer)
            ,m_sUpdateTableName(_rUpdateTableName)
            ,m_rRowCount(o_nRowCount)
            ,m_bRowCountFinal(false)
{
}

OKeySet::~OKeySet()
{
    tryDispose(m_xSet);
    // m_xStatement is necessarily one of those
    for (auto & statement : m_vStatements)
    {
        tryDispose(statement.second);
    }

    m_xComposer = nullptr;

}

void OKeySet::initColumns()
{
    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    bool bCase = xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers();
    m_pKeyColumnNames.reset( new SelectColumnsMetaData(bCase) );
    m_pColumnNames.reset( new SelectColumnsMetaData(bCase) );
    m_pParameterNames.reset( new SelectColumnsMetaData(bCase) );
    m_pForeignColumnNames.reset( new SelectColumnsMetaData(bCase) );
}

void OKeySet::findTableColumnsMatching_throw(   const Any& i_aTable,
                                                const OUString& i_rUpdateTableName,
                                                const Reference<XDatabaseMetaData>& i_xMeta,
                                                const Reference<XNameAccess>& i_xQueryColumns,
                                                std::unique_ptr<SelectColumnsMetaData> const & o_pKeyColumnNames)
{
    // first ask the database itself for the best columns which can be used
    Sequence< OUString> aBestColumnNames;
    Reference<XNameAccess> xKeyColumns  = getPrimaryKeyColumns_throw(i_aTable);
    if ( xKeyColumns.is() )
        aBestColumnNames = xKeyColumns->getElementNames();

    const Reference<XColumnsSupplier> xTblColSup(i_aTable,UNO_QUERY_THROW);
    const Reference<XNameAccess> xTblColumns = xTblColSup->getColumns();
    // locate parameter in select columns
    Reference<XParametersSupplier> xParaSup(m_xComposer,UNO_QUERY);
    Reference<XIndexAccess> xQueryParameters = xParaSup->getParameters();
    const sal_Int32 nParaCount = xQueryParameters->getCount();
    Sequence< OUString> aParameterColumns(nParaCount);
    for(sal_Int32 i = 0; i< nParaCount;++i)
    {
        Reference<XPropertySet> xPara(xQueryParameters->getByIndex(i),UNO_QUERY_THROW);
        xPara->getPropertyValue(PROPERTY_REALNAME) >>= aParameterColumns[i];
    }

    OUString sUpdateTableName( i_rUpdateTableName );
    if ( sUpdateTableName.isEmpty() )
    {
        SAL_WARN("dbaccess", "OKeySet::findTableColumnsMatching_throw: This is a fallback only - it won't work when the table has an alias name." );
        // If i_aTable originates from a query composer, and is a table which appears with an alias in the SELECT statement,
        // then the below code will not produce correct results.
        // For instance, imagine a "SELECT alias.col FROM table AS alias". Now i_aTable would be the table named
        // "table", so our sUpdateTableName would be "table" as well - not the information about the "alias" is
        // already lost here.
        // now getColumnPositions would traverse the columns, and check which of them belong to the table denoted
        // by sUpdateTableName. Since the latter is "table", but the columns only know that they belong to a table
        // named "alias", there will be no matching - so getColumnPositions wouldn't find anything.

        OUString sCatalog, sSchema, sTable;
        Reference<XPropertySet> xTableProp( i_aTable, UNO_QUERY_THROW );
        xTableProp->getPropertyValue( PROPERTY_CATALOGNAME )>>= sCatalog;
        xTableProp->getPropertyValue( PROPERTY_SCHEMANAME ) >>= sSchema;
        xTableProp->getPropertyValue( PROPERTY_NAME )       >>= sTable;
        sUpdateTableName = dbtools::composeTableName( i_xMeta, sCatalog, sSchema, sTable, false, ::dbtools::EComposeRule::InDataManipulation );
    }

    ::dbaccess::getColumnPositions(i_xQueryColumns,aBestColumnNames,sUpdateTableName,(*o_pKeyColumnNames),true);
    ::dbaccess::getColumnPositions(i_xQueryColumns,xTblColumns->getElementNames(),sUpdateTableName,(*m_pColumnNames),true);
    ::dbaccess::getColumnPositions(i_xQueryColumns,aParameterColumns,sUpdateTableName,(*m_pParameterNames),true);

    if ( o_pKeyColumnNames->empty() )
    {
        ::dbtools::throwGenericSQLException("Could not find any key column.", *this );
    }

    for (auto const& keyColumn : *o_pKeyColumnNames)
    {
        if ( !xTblColumns->hasByName( keyColumn.second.sRealName ) )
            continue;

        Reference<XPropertySet> xProp( xTblColumns->getByName( keyColumn.second.sRealName ), UNO_QUERY );
        bool bAuto = false;
        if ( ( xProp->getPropertyValue( PROPERTY_ISAUTOINCREMENT ) >>= bAuto ) && bAuto )
            m_aAutoColumns.push_back( keyColumn.first );
    }
}

namespace
{
    void appendOneKeyColumnClause( const OUString &tblName, const OUString &colName, const connectivity::ORowSetValue &_rValue, OUStringBuffer &o_buf )
    {
        OUString fullName;
        if (tblName.isEmpty())
            fullName = colName;
        else
            fullName = tblName + "." + colName;
        if ( _rValue.isNull() )
        {
            o_buf.append(fullName).append(" IS NULL ");
        }
        else
        {
            o_buf.append(fullName).append(" = ? ");
        }
    }
}

void OKeySet::setOneKeyColumnParameter( sal_Int32 &nPos, const Reference< XParameters > &_xParameter, const connectivity::ORowSetValue &_rValue, sal_Int32 _nType, sal_Int32 _nScale )
{
    if ( _rValue.isNull() )
    {
        // Nothing to do, appendOneKeyColumnClause took care of it,
        // the "IS NULL" is hardcoded in the query
    }
    else
    {
        setParameter( nPos++, _xParameter, _rValue, _nType, _nScale );
    }
}

OUStringBuffer OKeySet::createKeyFilter()
{
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aIter = m_aKeyIter->second.first->get().begin();

    static const char aAnd[] = " AND ";
    const OUString aQuote    = getIdentifierQuoteString();
    OUStringBuffer aFilter;
    // create the where clause
    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    for (auto const& keyColumnName : *m_pKeyColumnNames)
    {
        if ( ! aFilter.isEmpty() )
            aFilter.append(aAnd);
        appendOneKeyColumnClause(::dbtools::quoteTableName(xMeta, keyColumnName.second.sTableName, ::dbtools::EComposeRule::InDataManipulation),
                                 ::dbtools::quoteName(aQuote, keyColumnName.second.sRealName),
                                 *aIter++,
                                 aFilter);
    }
    for (auto const& foreignColumnName : * m_pForeignColumnNames)
    {
        if ( ! aFilter.isEmpty() )
            aFilter.append(aAnd);
        appendOneKeyColumnClause(::dbtools::quoteTableName(xMeta, foreignColumnName.second.sTableName, ::dbtools::EComposeRule::InDataManipulation),
                                 ::dbtools::quoteName(aQuote, foreignColumnName.second.sRealName),
                                 *aIter++,
                                 aFilter);
    }
    return aFilter;
}

void OKeySet::construct(const Reference< XResultSet>& _xDriverSet, const OUString& i_sRowSetFilter)
{
    OCacheSet::construct(_xDriverSet,i_sRowSetFilter);

    initColumns();

    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    Reference<XColumnsSupplier> xQueryColSup(m_xComposer, UNO_QUERY);
    const Reference<XNameAccess> xQueryColumns = xQueryColSup->getColumns();
    findTableColumnsMatching_throw( makeAny(m_xTable), m_sUpdateTableName, xMeta, xQueryColumns, m_pKeyColumnNames );

    Reference< XSingleSelectQueryComposer> xSourceComposer(m_xComposer,UNO_QUERY);
    Reference< XMultiServiceFactory >  xFactory(m_xConnection, UNO_QUERY_THROW);
    Reference<XSingleSelectQueryComposer> xAnalyzer(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
    xAnalyzer->setElementaryQuery(xSourceComposer->getElementaryQuery());
    Reference<XTablesSupplier> xTabSup(xAnalyzer,uno::UNO_QUERY);
    Reference<XNameAccess> xSelectTables(xTabSup->getTables(),uno::UNO_QUERY);
    const Sequence< OUString> aSeq = xSelectTables->getElementNames();
    if ( aSeq.getLength() > 1 ) // special handling for join
    {
        const OUString* pIter = aSeq.getConstArray();
        const OUString* const pEnd  = pIter + aSeq.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if ( *pIter != m_sUpdateTableName )
            {
                connectivity::OSQLTable xSelColSup(xSelectTables->getByName(*pIter),uno::UNO_QUERY);
                Reference<XPropertySet> xProp(xSelColSup,uno::UNO_QUERY);
                OUString sSelectTableName = ::dbtools::composeTableName( xMeta, xProp, ::dbtools::EComposeRule::InDataManipulation, false );

                ::dbaccess::getColumnPositions(xQueryColumns, xSelColSup->getColumns()->getElementNames(), sSelectTableName, (*m_pForeignColumnNames), true);

                // LEM: there used to be a break here; however, I see no reason to stop
                //      at first non-updateTable, so I removed it. (think of multiple joins...)
            }
        }
    }

    // the first row is empty because it's now easier for us to distinguish when we are beforefirst or first
    // without extra variable to be set
    OKeySetValue keySetValue(nullptr,std::pair<sal_Int32,Reference<XRow> >(0,Reference<XRow>()));
    m_aKeyMap.emplace(0, keySetValue);
    m_aKeyIter = m_aKeyMap.begin();
}

void OKeySet::reset(const Reference< XResultSet>& _xDriverSet)
{
    OCacheSet::construct(_xDriverSet, m_sRowSetFilter);
    m_bRowCountFinal = false;
    m_aKeyMap.clear();
    OKeySetValue keySetValue(nullptr,std::pair<sal_Int32,Reference<XRow> >(0,Reference<XRow>()));
    m_aKeyMap.emplace(0,keySetValue);
    m_aKeyIter = m_aKeyMap.begin();
}

void OKeySet::ensureStatement( )
{
    // do we already have a statement for the current combination of NULLness
    // of key & foreign columns?
    std::vector<bool> FilterColumnsNULL;
    FilterColumnsNULL.reserve(m_aKeyIter->second.first->get().size());
    for (auto const& elem : m_aKeyIter->second.first->get())
        FilterColumnsNULL.push_back(elem.isNull());
    vStatements_t::const_iterator pNewStatement(m_vStatements.find(FilterColumnsNULL));
    if(pNewStatement == m_vStatements.end())
    {
        // no: make a new one
        makeNewStatement();
        std::pair< vStatements_t::const_iterator, bool > insert_result
            (m_vStatements.emplace( FilterColumnsNULL, m_xStatement));
        (void) insert_result; // WaE: unused variable
        assert(insert_result.second);
    }
    else
        // yes: use it
        m_xStatement = pNewStatement->second;
}

void OKeySet::makeNewStatement()
{
    Reference< XSingleSelectQueryComposer> xSourceComposer(m_xComposer,UNO_QUERY);
    Reference< XMultiServiceFactory >  xFactory(m_xConnection, UNO_QUERY_THROW);
    Reference<XSingleSelectQueryComposer> xAnalyzer(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
    xAnalyzer->setElementaryQuery(xSourceComposer->getElementaryQuery());

    OUStringBuffer aFilter(createKeyFilter());
    executeStatement(aFilter, xAnalyzer);
}

void OKeySet::executeStatement(OUStringBuffer& io_aFilter, Reference<XSingleSelectQueryComposer>& io_xAnalyzer)
{
    bool bFilterSet = !m_sRowSetFilter.isEmpty();
    if ( bFilterSet )
    {
        FilterCreator aFilterCreator;
        aFilterCreator.append( m_sRowSetFilter );
        aFilterCreator.append( io_aFilter.makeStringAndClear() );
        io_aFilter = aFilterCreator.getComposedAndClear();
    }
    io_xAnalyzer->setFilter(io_aFilter.makeStringAndClear());
    if ( bFilterSet )
    {
        Sequence< Sequence< PropertyValue > > aFilter2 = io_xAnalyzer->getStructuredFilter();
        const Sequence< PropertyValue >* pOr = aFilter2.getConstArray();
        const Sequence< PropertyValue >* pOrEnd = pOr + aFilter2.getLength();
        for(;pOr != pOrEnd;++pOr)
        {
            const PropertyValue* pAnd = pOr->getConstArray();
            const PropertyValue* pAndEnd = pAnd + pOr->getLength();
            for(;pAnd != pAndEnd;++pAnd)
            {
                OUString sValue;
                if ( !(pAnd->Value >>= sValue) || !( sValue == "?" || sValue.startsWith( ":" ) ) )
                { // we have a criteria which has to be taken into account for updates
                    m_aFilterColumns.push_back(pAnd->Name);
                }
            }
        }
    }
    m_xStatement = m_xConnection->prepareStatement(io_xAnalyzer->getQueryWithSubstitution());
    ::comphelper::disposeComponent(io_xAnalyzer);
}

void OKeySet::invalidateRow()
{
    m_xRow = nullptr;
    ::comphelper::disposeComponent(m_xSet);
}

Any OKeySet::getBookmark()
{
    OSL_ENSURE(m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin(),
        "getBookmark is only possible when we stand on a valid row!");
    return makeAny(m_aKeyIter->first);
}

bool OKeySet::moveToBookmark( const Any& bookmark )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(bookmark));
    invalidateRow();
    return m_aKeyIter != m_aKeyMap.end();
}

sal_Int32 OKeySet::compareBookmarks( const Any& _first, const Any& _second )
{
    sal_Int32 nFirst = 0, nSecond = 0;
    _first >>= nFirst;
    _second >>= nSecond;

    return (nFirst != nSecond) ? CompareBookmark::NOT_EQUAL : CompareBookmark::EQUAL;
}

bool OKeySet::hasOrderedBookmarks(  )
{
    return true;
}

sal_Int32 OKeySet::hashBookmark( const Any& bookmark )
{
    return ::comphelper::getINT32(bookmark);
}


void OKeySet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOriginalRow,const connectivity::OSQLTable& _xTable  )
{
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    OUStringBuffer aSql = "UPDATE " + m_aComposedTableName + " SET ";
    // list all columns that should be set
    static OUString aPara(" = ?,");
    OUString aQuote  = getIdentifierQuoteString();
    static OUString aAnd(" AND ");
    OUString sIsNull(" IS NULL");
    OUString sParam(" = ?");

    // use keys and indexes for exact positioning
    // first the keys
    Reference<XNameAccess> xKeyColumns = getKeyColumns();

    // second the indexes
    Reference<XIndexesSupplier> xIndexSup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xIndexes;
    if ( xIndexSup.is() )
        xIndexes.set(xIndexSup->getIndexes(),UNO_QUERY);

    std::vector< Reference<XNameAccess> > aAllIndexColumns;
    lcl_fillIndexColumns(xIndexes,aAllIndexColumns);

    OUStringBuffer sKeyCondition,sIndexCondition;
    std::vector<sal_Int32> aIndexColumnPositions;

    const sal_Int32 nOldLength = aSql.getLength();
    sal_Int32 i = 1;
    // here we build the condition part for the update statement
    for (auto const& columnName : *m_pColumnNames)
    {
        if ( m_pKeyColumnNames->find(columnName.first) != m_pKeyColumnNames->end() )
        {
            sKeyCondition.append(::dbtools::quoteName( aQuote,columnName.second.sRealName));
            if((_rOriginalRow->get())[columnName.second.nPosition].isNull())
                sKeyCondition.append(sIsNull);
            else
                sKeyCondition.append(sParam);
            sKeyCondition.append(aAnd);
        }
        else
        {
            for (auto const& indexColumn : aAllIndexColumns)
            {
                if(indexColumn->hasByName(columnName.first))
                {
                    sIndexCondition.append(::dbtools::quoteName( aQuote,columnName.second.sRealName));
                    if((_rOriginalRow->get())[columnName.second.nPosition].isNull())
                        sIndexCondition.append(sIsNull);
                    else
                    {
                        sIndexCondition.append(sParam);
                        aIndexColumnPositions.push_back(columnName.second.nPosition);
                    }
                    sIndexCondition.append(aAnd);
                    break;
                }
            }
        }
        if((_rInsertRow->get())[columnName.second.nPosition].isModified())
        {
            aSql.append(::dbtools::quoteName( aQuote,columnName.second.sRealName)).append(aPara);
        }
        ++i;
    }

    if( aSql.getLength() != nOldLength )
    {
        aSql.setLength(aSql.getLength()-1);
    }
    else
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_VALUE_CHANGED ), StandardSQLState::GENERAL_ERROR, m_xConnection );

    if(!sKeyCondition.isEmpty() || !sIndexCondition.isEmpty())
    {
        aSql.append(" WHERE ");
        if(!sKeyCondition.isEmpty() && !sIndexCondition.isEmpty())
        {
            aSql.append(sKeyCondition.makeStringAndClear()).append(sIndexCondition.makeStringAndClear());
        }
        else if(!sKeyCondition.isEmpty())
        {
            aSql.append(sKeyCondition.makeStringAndClear());
        }
        else if(!sIndexCondition.isEmpty())
        {
            aSql.append(sIndexCondition.makeStringAndClear());
        }
        aSql.setLength(aSql.getLength()-5); // remove the last AND
    }
    else
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_CONDITION_FOR_PK ), StandardSQLState::GENERAL_ERROR, m_xConnection );

    // now create end execute the prepared statement
    executeUpdate(_rInsertRow ,_rOriginalRow,aSql.makeStringAndClear(),"",aIndexColumnPositions);
}

void OKeySet::executeUpdate(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOriginalRow,const OUString& i_sSQL,const OUString& i_sTableName,const std::vector<sal_Int32>& _aIndexColumnPositions)
{
    // now create end execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(i_sSQL));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    bool bRefetch = true;
    Reference<XRow> xRow;
    sal_Int32 i = 1;
    // first the set values
    for (auto const& columnName : *m_pColumnNames)
    {
        if ( i_sTableName.isEmpty() || columnName.second.sTableName == i_sTableName )
        {
            sal_Int32 nPos = columnName.second.nPosition;
            if((_rInsertRow->get())[nPos].isModified())
            {
                if ( bRefetch )
                {
                    bRefetch = std::find(m_aFilterColumns.begin(),m_aFilterColumns.end(),columnName.second.sRealName) == m_aFilterColumns.end();
                }
                impl_convertValue_throw(_rInsertRow,columnName.second);
                (_rInsertRow->get())[nPos].setSigned((_rOriginalRow->get())[nPos].isSigned());
                setParameter(i++,xParameter,(_rInsertRow->get())[nPos],columnName.second.nType,columnName.second.nScale);
            }
        }
    }
    // and then the values of the where condition
    for (auto const& keyColumnName : *m_pKeyColumnNames)
    {
        if ( i_sTableName.isEmpty() || keyColumnName.second.sTableName == i_sTableName )
        {
            setParameter(i++,xParameter,(_rOriginalRow->get())[keyColumnName.second.nPosition],keyColumnName.second.nType,keyColumnName.second.nScale);
        }
    }
    if ( !_aIndexColumnPositions.empty() )
    {
        // now we have to set the index values
        auto aIter = m_pColumnNames->begin();
        for (auto const& indexColumnPosition : _aIndexColumnPositions)
        {
            setParameter(i++,xParameter,(_rOriginalRow->get())[indexColumnPosition],(_rOriginalRow->get())[indexColumnPosition].getTypeKind(),aIter->second.nScale);
            ++aIter;
        }
    }
    const sal_Int32 nRowsUpdated = xPrep->executeUpdate();
    m_bUpdated = nRowsUpdated > 0;
    if(m_bUpdated)
    {
        const sal_Int32 nBookmark = ::comphelper::getINT32((_rInsertRow->get())[0].getAny());
        m_aKeyIter = m_aKeyMap.find(nBookmark);
        m_aKeyIter->second.second.first = 2;
        m_aKeyIter->second.second.second = xRow;
        copyRowValue(_rInsertRow,m_aKeyIter->second.first,nBookmark);
        tryRefetch(_rInsertRow,bRefetch);
    }
}

void OKeySet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable )
{
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    OUStringBuffer aSql( "INSERT INTO " + m_aComposedTableName + " ( ");

    // set values and column names
    OUStringBuffer aValues(" VALUES ( ");
    OUString aQuote = getIdentifierQuoteString();

    bool bRefetch = true;
    bool bModified = false;
    for (auto const& columnName : *m_pColumnNames)
    {
        if((_rInsertRow->get())[columnName.second.nPosition].isModified())
        {
            if ( bRefetch )
            {
                bRefetch = std::find(m_aFilterColumns.begin(),m_aFilterColumns.end(),columnName.second.sRealName) == m_aFilterColumns.end();
            }
            aSql.append(::dbtools::quoteName( aQuote,columnName.second.sRealName)).append(",");
            aValues.append("?,");
            bModified = true;
        }
    }
    if ( !bModified )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_VALUE_CHANGED ), StandardSQLState::GENERAL_ERROR, m_xConnection );

    aSql[aSql.getLength() - 1] = ')';
    aValues[aValues.getLength() - 1] = ')';
    aSql.append(aValues.makeStringAndClear());
    // now create,fill and execute the prepared statement
    executeInsert(_rInsertRow,aSql.makeStringAndClear(),"",bRefetch);
}

void OKeySet::executeInsert( const ORowSetRow& _rInsertRow,const OUString& i_sSQL,const OUString& i_sTableName,bool bRefetch )
{
    // now create,fill and execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(i_sSQL));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    sal_Int32 i = 1;
    for (auto const& columnName : *m_pColumnNames)
    {
        if ( i_sTableName.isEmpty() || columnName.second.sTableName == i_sTableName )
        {
            const sal_Int32 nPos = columnName.second.nPosition;
            if((_rInsertRow->get())[nPos].isModified())
            {
                if((_rInsertRow->get())[nPos].isNull())
                    xParameter->setNull(i++,(_rInsertRow->get())[nPos].getTypeKind());
                else
                {
                    impl_convertValue_throw(_rInsertRow,columnName.second);
                    (_rInsertRow->get())[nPos].setSigned(m_aSignedFlags[nPos-1]);
                    setParameter(i++,xParameter,(_rInsertRow->get())[nPos],columnName.second.nType,columnName.second.nScale);
                }
            }
        }
    }

    m_bInserted = xPrep->executeUpdate() > 0;
    bool bAutoValuesFetched = false;
    if ( m_bInserted )
    {
        // first insert the default values into the insertrow
        for (auto const& columnName : *m_pColumnNames)
        {
            if ( !(_rInsertRow->get())[columnName.second.nPosition].isModified() )
            {
                if(columnName.second.bNullable && columnName.second.sDefaultValue.isEmpty())
                {
                    (_rInsertRow->get())[columnName.second.nPosition].setTypeKind(columnName.second.nType);
                    (_rInsertRow->get())[columnName.second.nPosition].setNull();
                }
                else
                {
                    (_rInsertRow->get())[columnName.second.nPosition] = columnName.second.sDefaultValue;
                    (_rInsertRow->get())[columnName.second.nPosition].setTypeKind(columnName.second.nType);
                }
            }
        }
        try
        {
            Reference< XGeneratedResultSet > xGRes(xPrep, UNO_QUERY);
            if ( xGRes.is() )
            {
                Reference< XResultSet > xRes = xGRes->getGeneratedValues();
                Reference< XRow > xRow(xRes,UNO_QUERY);
                if ( xRow.is() && xRes->next() )
                {
                    Reference< XResultSetMetaDataSupplier > xMdSup(xRes,UNO_QUERY);
                    Reference< XResultSetMetaData > xMd = xMdSup->getMetaData();
                    sal_Int32 nColumnCount = xMd->getColumnCount();
                    std::vector< OUString >::const_iterator aAutoIter = m_aAutoColumns.begin();
                    std::vector< OUString >::const_iterator aAutoEnd = m_aAutoColumns.end();
                    for (sal_Int32 j = 1;aAutoIter !=  aAutoEnd && j <= nColumnCount; ++aAutoIter,++j)
                    {
                        SelectColumnsMetaData::const_iterator aFind = m_pKeyColumnNames->find(*aAutoIter);
                        if ( aFind != m_pKeyColumnNames->end() )
                            (_rInsertRow->get())[aFind->second.nPosition].fill(j, aFind->second.nType, xRow);
                    }
                    bAutoValuesFetched = true;
                }
            }
        }
        catch(const Exception&)
        {
            SAL_WARN("dbaccess", "Could not execute GeneratedKeys() stmt");
        }
    }

    ::comphelper::disposeComponent(xPrep);

    if ( i_sTableName.isEmpty() && !bAutoValuesFetched && m_bInserted )
    {
        // first check if all key column values were set
        const OUString sQuote = getIdentifierQuoteString();
        OUStringBuffer sMaxStmt;
        auto aEnd = m_pKeyColumnNames->end();
        for (auto const& autoColumn : m_aAutoColumns)
        {
            // we will only fetch values which are keycolumns
            SelectColumnsMetaData::const_iterator aFind = m_pKeyColumnNames->find(autoColumn);
            if ( aFind != aEnd )
            {
                sMaxStmt.append(" MAX(").append(::dbtools::quoteName( sQuote,aFind->second.sRealName)).append("),");
            }
        }

        if(!sMaxStmt.isEmpty())
        {
            sMaxStmt[sMaxStmt.getLength()-1] = ' ';
            OUString sStmt = "SELECT " + sMaxStmt.makeStringAndClear() + "FROM ";
            OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(m_xConnection->getMetaData(),m_sUpdateTableName,sCatalog,sSchema,sTable,::dbtools::EComposeRule::InDataManipulation);
            sStmt += ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable );
            try
            {
                // now fetch the autoincrement values
                Reference<XStatement> xStatement = m_xConnection->createStatement();
                Reference<XResultSet> xRes = xStatement->executeQuery(sStmt);
                Reference<XRow> xRow(xRes,UNO_QUERY);
                if(xRow.is() && xRes->next())
                {
                    sal_Int32 j=1;
                    for (auto const& autoColumn : m_aAutoColumns)
                    {
                        // we will only fetch values which are keycolumns
                        SelectColumnsMetaData::const_iterator aFind = m_pKeyColumnNames->find(autoColumn);
                        if ( aFind != aEnd )
                            (_rInsertRow->get())[aFind->second.nPosition].fill(j++, aFind->second.nType, xRow);
                    }
                }
                ::comphelper::disposeComponent(xStatement);
            }
            catch(SQLException&)
            {
                SAL_WARN("dbaccess", "Could not fetch with MAX() ");
            }
        }
    }
    if ( m_bInserted )
    {
        OKeySetMatrix::const_iterator aKeyIter = m_aKeyMap.end();
        --aKeyIter;
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >(m_pKeyColumnNames->size());
        copyRowValue(_rInsertRow,aKeyRow,aKeyIter->first + 1);

        m_aKeyIter = m_aKeyMap.emplace( aKeyIter->first + 1, OKeySetValue(aKeyRow,std::pair<sal_Int32,Reference<XRow> >(1,Reference<XRow>())) ).first;
        // now we set the bookmark for this row
        (_rInsertRow->get())[0] = makeAny(static_cast<sal_Int32>(m_aKeyIter->first));
        tryRefetch(_rInsertRow,bRefetch);
    }
}

void OKeySet::tryRefetch(const ORowSetRow& _rInsertRow,bool bRefetch)
{
    if ( bRefetch )
    {
        try
        {
            bRefetch = doTryRefetch_throw();
        }
        catch(const Exception&)
        {
            bRefetch = false;
        }
    }
    if ( !bRefetch )
    {
        m_aKeyIter->second.second.second = new OPrivateRow(_rInsertRow->get());
    }
}

void OKeySet::copyRowValue(const ORowSetRow& _rInsertRow, ORowSetRow const & _rKeyRow, sal_Int32 i_nBookmark)
{
    connectivity::ORowVector< ORowSetValue >::Vector::iterator aIter = _rKeyRow->get().begin();

    // check the if the parameter values have been changed
    OSL_ENSURE((m_aParameterValueForCache->get().size()-1) == m_pParameterNames->size(),"OKeySet::copyRowValue: Parameter values and names differ!");
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aParaValuesIter = m_aParameterValueForCache->get().begin() +1;

    bool bChanged = false;
    sal_Int32 i = 1;
    for (auto const& parameterName : *m_pParameterNames)
    {
        ORowSetValue aValue(*aParaValuesIter);
        aValue.setSigned(m_aSignedFlags[parameterName.second.nPosition-1]);
        if ( (_rInsertRow->get())[parameterName.second.nPosition] != aValue )
        {
            rtl::Reference aCopy(
                new ORowSetValueVector(*m_aParameterValueForCache));
            (aCopy->get())[i] = (_rInsertRow->get())[parameterName.second.nPosition];
            m_aUpdatedParameter[i_nBookmark] = aCopy;
            bChanged = true;
        }
        ++aParaValuesIter;
        ++i;
    }
    if ( !bChanged )
    {
        m_aUpdatedParameter.erase(i_nBookmark);
    }

    // update the key values
    for (auto const& keyColumnName : *m_pKeyColumnNames)
    {
        impl_convertValue_throw(_rInsertRow,keyColumnName.second);
        *aIter = (_rInsertRow->get())[keyColumnName.second.nPosition];
        aIter->setTypeKind(keyColumnName.second.nType);
        ++aIter;
    }
}

void OKeySet::deleteRow(const ORowSetRow& _rDeleteRow,const connectivity::OSQLTable& _xTable   )
{
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    OUStringBuffer aSql("DELETE FROM " + m_aComposedTableName + " WHERE ");

    // list all columns that should be set
    OUString aQuote  = getIdentifierQuoteString();
    static const char aAnd[] = " AND ";

    // use keys and indexes for exact positioning
    Reference<XNameAccess> xKeyColumns = getKeyColumns();
    // second the indexes
    Reference<XIndexesSupplier> xIndexSup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xIndexes;
    if ( xIndexSup.is() )
        xIndexes.set(xIndexSup->getIndexes(),UNO_QUERY);

    //  Reference<XColumnsSupplier>
    std::vector< Reference<XNameAccess> > aAllIndexColumns;
    lcl_fillIndexColumns(xIndexes,aAllIndexColumns);

    OUStringBuffer sIndexCondition;
    std::vector<sal_Int32> aIndexColumnPositions;

    for (auto const& columnName : *m_pColumnNames)
    {
        if ( m_pKeyColumnNames->find(columnName.first) != m_pKeyColumnNames->end() )
        {
            aSql.append(::dbtools::quoteName( aQuote,columnName.second.sRealName));
            if((_rDeleteRow->get())[columnName.second.nPosition].isNull())
            {
                SAL_WARN("dbaccess", "can a primary key be null");
                aSql.append(" IS NULL");
            }
            else
                aSql.append(" = ?");
            aSql.append(aAnd);
        }
        else
        {
            for (auto const& indexColumn : aAllIndexColumns)
            {
                if(indexColumn->hasByName(columnName.first))
                {
                    sIndexCondition.append(::dbtools::quoteName( aQuote,columnName.second.sRealName));
                    if((_rDeleteRow->get())[columnName.second.nPosition].isNull())
                        sIndexCondition.append(" IS NULL");
                    else
                    {
                        sIndexCondition.append(" = ?");
                        aIndexColumnPositions.push_back(columnName.second.nPosition);
                    }
                    sIndexCondition.append(aAnd);

                    break;
                }
            }
        }
    }
    aSql.append(sIndexCondition.makeStringAndClear());
    aSql.setLength(aSql.getLength()-5);

    // now create end execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql.makeStringAndClear()));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    sal_Int32 i = 1;
    for (auto const& keyColumnName : *m_pKeyColumnNames)
    {
        setParameter(i++,xParameter,(_rDeleteRow->get())[keyColumnName.second.nPosition],keyColumnName.second.nType,keyColumnName.second.nScale);
    }

    // now we have to set the index values
    auto aIter = m_pColumnNames->begin();
    for (auto const& indexColumnPosition : aIndexColumnPositions)
    {
        setParameter(i++,xParameter,(_rDeleteRow->get())[indexColumnPosition],(_rDeleteRow->get())[indexColumnPosition].getTypeKind(),aIter->second.nScale);
        ++aIter;
    }

    m_bDeleted = xPrep->executeUpdate() > 0;

    if(m_bDeleted)
    {
        sal_Int32 nBookmark = ::comphelper::getINT32((_rDeleteRow->get())[0].getAny());
        if(m_aKeyIter == m_aKeyMap.find(nBookmark) && m_aKeyIter != m_aKeyMap.end())
            ++m_aKeyIter;
        m_aKeyMap.erase(nBookmark);
        m_bDeleted = true;
    }
}

Reference<XNameAccess> OKeySet::getKeyColumns() const
{
    // use keys and indexes for exact positioning
    // first the keys

    Reference<XIndexAccess> xKeys = m_xTableKeys;
    if ( !xKeys.is() )
    {
        Reference<XPropertySet> xSet(m_xTable,UNO_QUERY);
        const Reference<XNameAccess> xPrimaryKeyColumns = getPrimaryKeyColumns_throw(xSet);
        return xPrimaryKeyColumns;
    }

    Reference<XColumnsSupplier> xKeyColsSup;
    Reference<XNameAccess> xKeyColumns;
    if(xKeys.is())
    {
        Reference<XPropertySet> xProp;
        sal_Int32 nCount = xKeys->getCount();
        for(sal_Int32 i = 0;i< nCount;++i)
        {
            xProp.set(xKeys->getByIndex(i),UNO_QUERY);
            if ( xProp.is() )
            {
                sal_Int32 nKeyType = 0;
                xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                if(KeyType::PRIMARY == nKeyType)
                {
                    xKeyColsSup.set(xProp,UNO_QUERY);
                    OSL_ENSURE(xKeyColsSup.is(),"Columnsupplier is null!");
                    xKeyColumns = xKeyColsSup->getColumns();
                    break;
                }
            }
        }
    }

    return xKeyColumns;
}

bool OKeySet::next()
{
    m_bInserted = m_bUpdated = m_bDeleted = false;

    if(isAfterLast())
        return false;
    ++m_aKeyIter;
    if(!m_bRowCountFinal && m_aKeyIter == m_aKeyMap.end())
    {
        // not yet all records fetched, but we reached the end of those we fetched
        // try to fetch one more row
        if (fetchRow())
        {
            OSL_ENSURE(!isAfterLast(), "fetchRow succeeded, but isAfterLast()");
            return true;
        }
        else
        {
            // nope, we arrived at end of data
            m_aKeyIter = m_aKeyMap.end();
            OSL_ENSURE(isAfterLast(), "fetchRow failed, but not end of data");
        }
    }

    invalidateRow();
    return !isAfterLast();
}

bool OKeySet::isBeforeFirst(  )
{
    return m_aKeyIter == m_aKeyMap.begin();
}

bool OKeySet::isAfterLast(  )
{
    return  m_bRowCountFinal && m_aKeyIter == m_aKeyMap.end();
}

void OKeySet::beforeFirst(  )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    m_aKeyIter = m_aKeyMap.begin();
    invalidateRow();
}

void OKeySet::afterLast(  )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    fillAllRows();
    m_aKeyIter = m_aKeyMap.end();
    invalidateRow();
}

bool OKeySet::first()
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    m_aKeyIter = m_aKeyMap.begin();
    ++m_aKeyIter;
    if(m_aKeyIter == m_aKeyMap.end())
    {
        if (!fetchRow())
        {
            m_aKeyIter = m_aKeyMap.end();
            return false;
        }
    }
    else
        invalidateRow();
    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}

bool OKeySet::last(  )
{
    return last_checked(true);
}

bool OKeySet::last_checked( bool /* i_bFetchRow */ )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    bool bFetchedRow = fillAllRows();

    m_aKeyIter = m_aKeyMap.end();
    --m_aKeyIter;
    if ( !bFetchedRow )
    {
        invalidateRow();
    }
    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}

sal_Int32 OKeySet::getRow(  )
{
    OSL_ENSURE(!isAfterLast(),"getRow is not allowed when afterlast record!");
    OSL_ENSURE(!isBeforeFirst(),"getRow is not allowed when beforefirst record!");

    return std::distance(m_aKeyMap.begin(),m_aKeyIter);
}

bool OKeySet::absolute( sal_Int32 row )
{
    return absolute_checked(row,true);
}

bool OKeySet::absolute_checked( sal_Int32 row, bool /* i_bFetchRow */ )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    OSL_ENSURE(row,"absolute(0) isn't allowed!");
    bool bFetchedRow = false;
    if(row < 0)
    {
        if(!m_bRowCountFinal)
            bFetchedRow = fillAllRows();

        row = std::min(std::abs(row), static_cast<sal_Int32>(std::distance(m_aKeyMap.begin(), m_aKeyIter)));
        m_aKeyIter = std::prev(m_aKeyIter, row);
    }
    else
    {
        if(row >= static_cast<sal_Int32>(m_aKeyMap.size()))
        {
            // we don't have this row
            if(!m_bRowCountFinal)
            {
                // but there may still be rows to fetch.
                bool bNext = true;
                for(sal_Int32 i=m_aKeyMap.size()-1;i < row && bNext;++i)
                    bNext = fetchRow();
                // it is guaranteed that the above loop has executed at least once,
                // that is fetchRow called at least once.
                if ( bNext )
                {
                    bFetchedRow = true;
                }
                else
                {
                    // reached end of data before desired row
                    m_aKeyIter = m_aKeyMap.end();
                    return false;
                }
            }
            else
            {
                // no more rows to fetch -> fail
                m_aKeyIter = m_aKeyMap.end();
                return false;
            }
        }
        else
        {
            m_aKeyIter = std::next(m_aKeyMap.begin(), row);
        }
    }
    if ( !bFetchedRow )
    {
        invalidateRow();
    }

    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}

bool OKeySet::previous_checked( bool /* i_bFetchRow */ )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    if(m_aKeyIter != m_aKeyMap.begin())
    {
        --m_aKeyIter;
        invalidateRow();
    }
    return m_aKeyIter != m_aKeyMap.begin();
}

bool OKeySet::previous(  )
{
    return previous_checked(true);
}

bool OKeySet::doTryRefetch_throw()
{
    ensureStatement( );
    // we just reassign the base members
    Reference< XParameters > xParameter(m_xStatement,UNO_QUERY);
    OSL_ENSURE(xParameter.is(),"No Parameter interface!");
    xParameter->clearParameters();

    sal_Int32 nPos=1;
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aParaIter;
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aParaEnd;
    OUpdatedParameter::const_iterator aUpdateFind = m_aUpdatedParameter.find(m_aKeyIter->first);
    if ( aUpdateFind == m_aUpdatedParameter.end() )
    {
        aParaIter = m_aParameterValueForCache->get().begin();
        aParaEnd = m_aParameterValueForCache->get().end();
    }
    else
    {
        aParaIter = aUpdateFind->second->get().begin();
        aParaEnd = aUpdateFind->second->get().end();
    }

    for(++aParaIter;aParaIter != aParaEnd;++aParaIter,++nPos)
    {
        ::dbtools::setObjectWithInfo( xParameter, nPos, aParaIter->makeAny(), aParaIter->getTypeKind() );
    }

    // now set the primary key column values
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aIter = m_aKeyIter->second.first->get().begin();
    for (auto const& keyColumnName : *m_pKeyColumnNames)
        setOneKeyColumnParameter(nPos,xParameter,*aIter++,keyColumnName.second.nType,keyColumnName.second.nScale);
    for (auto const& foreignColumnName : *m_pForeignColumnNames)
        setOneKeyColumnParameter(nPos,xParameter,*aIter++,foreignColumnName.second.nType,foreignColumnName.second.nScale);

    m_xSet = m_xStatement->executeQuery();
    OSL_ENSURE(m_xSet.is(),"No resultset from statement!");
    return m_xSet->next();
}

void OKeySet::refreshRow()
{
    invalidateRow();

    if(isBeforeFirst() || isAfterLast())
        return;

    if ( m_aKeyIter->second.second.second.is() )
    {
        m_xRow = m_aKeyIter->second.second.second;
        return;
    }

    bool bOK = doTryRefetch_throw();
    if ( !bOK )
    {
        // This row has disappeared; remove it.
        OKeySetMatrix::const_iterator aTemp = m_aKeyIter;
        // use *next* row
        ++m_aKeyIter;
        m_aKeyMap.erase(aTemp);

        // adjust RowCount for the row we have removed
        if (m_rRowCount > 0)
            --m_rRowCount;
        else
            SAL_WARN("dbaccess", "m_rRowCount got out of sync: non-empty m_aKeyMap, but m_rRowCount <= 0");

        if (m_aKeyIter == m_aKeyMap.end())
        {
            ::comphelper::disposeComponent(m_xSet);
            if (!isAfterLast())
            {
                // it was the last fetched row,
                // but there may be another one to fetch
                if (!fetchRow())
                {
                    // nope, that really was the last
                    m_aKeyIter = m_aKeyMap.end();
                    OSL_ENSURE(isAfterLast(), "fetchRow() failed but not isAfterLast()!");
                }
            }
            // Now, either fetchRow has set m_xRow or isAfterLast()
        }
        else
        {
            refreshRow();
        }
    }
    else
    {
        m_xRow.set(m_xSet,UNO_QUERY);
        OSL_ENSURE(m_xRow.is(),"No row from statement!");
    }
}

bool OKeySet::fetchRow()
{
    // fetch the next row and append on the keyset
    bool bRet = false;
    if ( !m_bRowCountFinal && (!m_nMaxRows || sal_Int32(m_aKeyMap.size()) < m_nMaxRows) )
        bRet = m_xDriverSet->next();
    if ( bRet )
    {
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >((*m_pKeyColumnNames).size() + m_pForeignColumnNames->size());

        ::comphelper::disposeComponent(m_xSet);
        m_xRow.set(m_xDriverRow, UNO_QUERY_THROW);

        connectivity::ORowVector< ORowSetValue >::Vector::iterator aIter = aKeyRow->get().begin();
        // copy key columns
        for (auto const& keyColumnName : *m_pKeyColumnNames)
        {
            const SelectColumnDescription& rColDesc = keyColumnName.second;
            aIter->fill(rColDesc.nPosition, rColDesc.nType, m_xRow);
            ++aIter;
        }
        // copy missing columns from other tables
        for (auto const& foreignColumnName : *m_pForeignColumnNames)
        {
            const SelectColumnDescription& rColDesc = foreignColumnName.second;
            aIter->fill(rColDesc.nPosition, rColDesc.nType, m_xRow);
            ++aIter;
        }
        m_aKeyIter = m_aKeyMap.emplace( m_aKeyMap.rbegin()->first+1,OKeySetValue(aKeyRow,std::pair<sal_Int32,Reference<XRow> >(0,Reference<XRow>())) ).first;
    }
    else
        m_bRowCountFinal = true;
    return bRet;
}

bool OKeySet::fillAllRows()
{
    if(m_bRowCountFinal)
    {
        return false;
    }
    else
    {
        while(fetchRow())
            ;
        return true;
    }
}

// XRow
sal_Bool SAL_CALL OKeySet::wasNull(  )
{
    if ( ! m_xRow.is() )
        throwGenericSQLException("Must call getFOO() for some FOO before wasNull()", *this);

    OSL_ENSURE(m_xRow.is(),"m_xRow is null! I've thrown, but function execution continued?");
    return m_xRow->wasNull();
}

inline void OKeySet::ensureRowForData( )
{
    if (! m_xRow.is() )
        refreshRow();
    if (! m_xRow.is() )
        throwSQLException("Failed to refetch row", "02000", *this, -2);

    OSL_ENSURE(m_xRow.is(),"m_xRow is null! I've called throwSQLException but execution continued?");
}

OUString SAL_CALL OKeySet::getString( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getString(columnIndex);
}

sal_Bool SAL_CALL OKeySet::getBoolean( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getBoolean(columnIndex);
}

sal_Int8 SAL_CALL OKeySet::getByte( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getByte(columnIndex);
}

sal_Int16 SAL_CALL OKeySet::getShort( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getShort(columnIndex);
}

sal_Int32 SAL_CALL OKeySet::getInt( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getInt(columnIndex);
}

sal_Int64 SAL_CALL OKeySet::getLong( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getLong(columnIndex);
}

float SAL_CALL OKeySet::getFloat( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getFloat(columnIndex);
}

double SAL_CALL OKeySet::getDouble( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getDouble(columnIndex);
}

Sequence< sal_Int8 > SAL_CALL OKeySet::getBytes( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getBytes(columnIndex);
}

css::util::Date SAL_CALL OKeySet::getDate( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getDate(columnIndex);
}

css::util::Time SAL_CALL OKeySet::getTime( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getTime(columnIndex);
}

css::util::DateTime SAL_CALL OKeySet::getTimestamp( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getTimestamp(columnIndex);
}

Reference< css::io::XInputStream > SAL_CALL OKeySet::getBinaryStream( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getBinaryStream(columnIndex);
}

Reference< css::io::XInputStream > SAL_CALL OKeySet::getCharacterStream( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getCharacterStream(columnIndex);
}

Any SAL_CALL OKeySet::getObject( sal_Int32 columnIndex, const Reference< css::container::XNameAccess >& typeMap )
{
    ensureRowForData();
    return m_xRow->getObject(columnIndex,typeMap);
}

Reference< XRef > SAL_CALL OKeySet::getRef( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getRef(columnIndex);
}

Reference< XBlob > SAL_CALL OKeySet::getBlob( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getBlob(columnIndex);
}

Reference< XClob > SAL_CALL OKeySet::getClob( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getClob(columnIndex);
}

Reference< XArray > SAL_CALL OKeySet::getArray( sal_Int32 columnIndex )
{
    ensureRowForData();
    return m_xRow->getArray(columnIndex);
}

bool OKeySet::rowUpdated(  )
{
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second.first == 2;
}

bool OKeySet::rowInserted(  )
{
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second.first == 1;
}

bool OKeySet::rowDeleted(  )
{
    bool bDeleted = m_bDeleted;
    m_bDeleted = false;
    return bDeleted;
}

namespace dbaccess
{

void getColumnPositions(const Reference<XNameAccess>& _rxQueryColumns,
                            const css::uno::Sequence< OUString >& _aColumnNames,
                            const OUString& _rsUpdateTableName,
                            SelectColumnsMetaData& o_rColumnNames,
                            bool i_bAppendTableName)
    {
        // get the real name of the columns
        Sequence< OUString> aSelNames(_rxQueryColumns->getElementNames());
        const OUString* pSelIter     = aSelNames.getConstArray();
        const OUString* pSelEnd      = pSelIter + aSelNames.getLength();

        const OUString* pTblColumnIter   = _aColumnNames.getConstArray();
        const OUString* pTblColumnEnd    = pTblColumnIter + _aColumnNames.getLength();

        ::comphelper::UStringMixEqual bCase(o_rColumnNames.key_comp().isCaseSensitive());

        for(sal_Int32 nPos = 1;pSelIter != pSelEnd;++pSelIter,++nPos)
        {
            Reference<XPropertySet> xQueryColumnProp(_rxQueryColumns->getByName(*pSelIter),UNO_QUERY_THROW);
            OUString sRealName,sTableName;
            OSL_ENSURE(xQueryColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
            OSL_ENSURE(xQueryColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");
            xQueryColumnProp->getPropertyValue(PROPERTY_REALNAME)   >>= sRealName;
            xQueryColumnProp->getPropertyValue(PROPERTY_TABLENAME)  >>= sTableName;

            for(;pTblColumnIter != pTblColumnEnd;++pTblColumnIter)
            {
                if(bCase(sRealName,*pTblColumnIter) && bCase(_rsUpdateTableName,sTableName) && o_rColumnNames.find(*pTblColumnIter) == o_rColumnNames.end())
                {
                    sal_Int32 nType = 0;
                    xQueryColumnProp->getPropertyValue(PROPERTY_TYPE)   >>= nType;
                    sal_Int32 nScale = 0;
                    xQueryColumnProp->getPropertyValue(PROPERTY_SCALE)  >>= nScale;
                    OUString sColumnDefault;
                    if ( xQueryColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_DEFAULTVALUE) )
                        xQueryColumnProp->getPropertyValue(PROPERTY_DEFAULTVALUE) >>= sColumnDefault;

                    sal_Int32 nNullable = ColumnValue::NULLABLE_UNKNOWN;
                    OSL_VERIFY( xQueryColumnProp->getPropertyValue( PROPERTY_ISNULLABLE ) >>= nNullable );

                    SelectColumnDescription aColDesc( nPos, nType, nScale, nNullable != sdbc::ColumnValue::NO_NULLS, sColumnDefault );
                    OUString sName;
                    if ( i_bAppendTableName )
                    {
                        sName = sTableName + "." + sRealName;
                        aColDesc.sRealName = sRealName;
                        aColDesc.sTableName = sTableName;
                    }
                    else
                    {
                        sName = sRealName;
                    }
                    o_rColumnNames[sName] = aColDesc;

                    break;
                }
            }
            pTblColumnIter = _aColumnNames.getConstArray();
        }
    }
}

void OKeySet::impl_convertValue_throw(const ORowSetRow& _rInsertRow,const SelectColumnDescription& i_aMetaData)
{
    ORowSetValue& aValue((_rInsertRow->get())[i_aMetaData.nPosition]);
    switch(i_aMetaData.nType)
    {
        case DataType::DECIMAL:
        case DataType::NUMERIC:
            {
                OUString sValue = aValue.getString();
                sal_Int32 nIndex = sValue.indexOf('.');
                if ( nIndex != -1 )
                {
                    aValue = sValue.copy(0,std::min(sValue.getLength(),nIndex + (i_aMetaData.nScale > 0 ? i_aMetaData.nScale + 1 : 0)));
                }
            }
            break;
        default:
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
