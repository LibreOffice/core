/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle andor its affiliates.
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


#include "KeySet.hxx"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include "dbastrings.hrc"
#include "apitools.hxx"
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <boost/static_assert.hpp>
#include <list>
#include <algorithm>
#include <string.h>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include "querycomposer.hxx"
#include "composertools.hxx"
#include <tools/debug.hxx>
#include <rtl/logfile.hxx>
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

namespace
{
    void lcl_fillIndexColumns(const Reference<XIndexAccess>& _xIndexes, ::std::vector< Reference<XNameAccess> >& _rAllIndexColumns)
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

    template < typename T > inline void tryDispose( Reference<T> &r )
    {
        try
        {
            ::comphelper::disposeComponent(r);
        }
        catch(const Exception&)
        {
            r = NULL;
        }
        catch(...)
        {
            OSL_FAIL("Unknown Exception occurred");
        }
    }
}
DBG_NAME(OKeySet)

OKeySet::OKeySet(const connectivity::OSQLTable& _xTable,
                 const Reference< XIndexAccess>& _xTableKeys,
                 const ::rtl::OUString& _rUpdateTableName,    // this can be the alias or the full qualified name
                 const Reference< XSingleSelectQueryAnalyzer >& _xComposer,
                 const ORowSetValueVector& _aParameterValueForCache,
                 sal_Int32 i_nMaxRows,
                 sal_Int32& o_nRowCount)
            :OCacheSet(i_nMaxRows)
            ,m_aParameterValueForCache(_aParameterValueForCache)
            ,m_pKeyColumnNames(NULL)
            ,m_pColumnNames(NULL)
            ,m_pParameterNames(NULL)
            ,m_pForeignColumnNames(NULL)
            ,m_xTable(_xTable)
            ,m_xTableKeys(_xTableKeys)
            ,m_xComposer(_xComposer)
            ,m_sUpdateTableName(_rUpdateTableName)
            ,m_rRowCount(o_nRowCount)
            ,m_bRowCountFinal(sal_False)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::OKeySet" );
    DBG_CTOR(OKeySet,NULL);

}

OKeySet::~OKeySet()
{
    tryDispose(m_xStatement);
    tryDispose(m_xSet);

    m_xComposer = NULL;

    DBG_DTOR(OKeySet,NULL);
}
void OKeySet::initColumns()
{
    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    bool bCase = (xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers()) ? true : false;
    m_pKeyColumnNames.reset( new SelectColumnsMetaData(bCase) );
    m_pColumnNames.reset( new SelectColumnsMetaData(bCase) );
    m_pParameterNames.reset( new SelectColumnsMetaData(bCase) );
    m_pForeignColumnNames.reset( new SelectColumnsMetaData(bCase) );
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
void OKeySet::findTableColumnsMatching_throw(   const Any& i_aTable,
                                                const ::rtl::OUString& i_rUpdateTableName,
                                                const Reference<XDatabaseMetaData>& i_xMeta,
                                                const Reference<XNameAccess>& i_xQueryColumns,
                                                ::std::auto_ptr<SelectColumnsMetaData>& o_pKeyColumnNames)
{
    // first ask the database itself for the best columns which can be used
    Sequence< ::rtl::OUString> aBestColumnNames;
    Reference<XNameAccess> xKeyColumns  = getPrimaryKeyColumns_throw(i_aTable);
    if ( xKeyColumns.is() )
        aBestColumnNames = xKeyColumns->getElementNames();

    const Reference<XColumnsSupplier> xTblColSup(i_aTable,UNO_QUERY_THROW);
    const Reference<XNameAccess> xTblColumns = xTblColSup->getColumns();
    // locate parameter in select columns
    Reference<XParametersSupplier> xParaSup(m_xComposer,UNO_QUERY);
    Reference<XIndexAccess> xQueryParameters = xParaSup->getParameters();
    const sal_Int32 nParaCount = xQueryParameters->getCount();
    Sequence< ::rtl::OUString> aParameterColumns(nParaCount);
    for(sal_Int32 i = 0; i< nParaCount;++i)
    {
        Reference<XPropertySet> xPara(xQueryParameters->getByIndex(i),UNO_QUERY_THROW);
        xPara->getPropertyValue(PROPERTY_REALNAME) >>= aParameterColumns[i];
    }

    ::rtl::OUString sUpdateTableName( i_rUpdateTableName );
    if ( sUpdateTableName.isEmpty() )
    {
        OSL_FAIL( "OKeySet::findTableColumnsMatching_throw: This is a fallback only - it won't work when the table has an alias name." );
        // If i_aTable originates from a query composer, and is a table which appears with an alias in the SELECT statement,
        // then the below code will not produce correct results.
        // For instance, imagine a "SELECT alias.col FROM table AS alias". Now i_aTable would be the table named
        // "table", so our sUpdateTableName would be "table" as well - not the information about the "alias" is
        // already lost here.
        // now getColumnPositions would travers the columns, and check which of them belong to the table denoted
        // by sUpdateTableName. Since the latter is "table", but the columns only know that they belong to a table
        // named "alias", there will be no matching - so getColumnPositions wouldn't find anything.

        ::rtl::OUString sCatalog, sSchema, sTable;
        Reference<XPropertySet> xTableProp( i_aTable, UNO_QUERY_THROW );
        xTableProp->getPropertyValue( PROPERTY_CATALOGNAME )>>= sCatalog;
        xTableProp->getPropertyValue( PROPERTY_SCHEMANAME ) >>= sSchema;
        xTableProp->getPropertyValue( PROPERTY_NAME )       >>= sTable;
        sUpdateTableName = dbtools::composeTableName( i_xMeta, sCatalog, sSchema, sTable, sal_False, ::dbtools::eInDataManipulation );
    }

    ::dbaccess::getColumnPositions(i_xQueryColumns,aBestColumnNames,sUpdateTableName,(*o_pKeyColumnNames),true);
    ::dbaccess::getColumnPositions(i_xQueryColumns,xTblColumns->getElementNames(),sUpdateTableName,(*m_pColumnNames),true);
    ::dbaccess::getColumnPositions(i_xQueryColumns,aParameterColumns,sUpdateTableName,(*m_pParameterNames),true);

    if ( o_pKeyColumnNames->empty() )
    {
        ::dbtools::throwGenericSQLException( ::rtl::OUString( "Could not find any key column."  ), *this );
    }

    for (   SelectColumnsMetaData::const_iterator keyColumn = o_pKeyColumnNames->begin();
            keyColumn != o_pKeyColumnNames->end();
            ++keyColumn
        )
    {
        if ( !xTblColumns->hasByName( keyColumn->second.sRealName ) )
            continue;

        Reference<XPropertySet> xProp( xTblColumns->getByName( keyColumn->second.sRealName ), UNO_QUERY );
        sal_Bool bAuto = sal_False;
        if ( ( xProp->getPropertyValue( PROPERTY_ISAUTOINCREMENT ) >>= bAuto ) && bAuto )
            m_aAutoColumns.push_back( keyColumn->first );
    }
}
SAL_WNODEPRECATED_DECLARATIONS_POP

namespace
{
    void appendOneKeyColumnClause( const ::rtl::OUString &tblName, const ::rtl::OUString &colName, ::rtl::OUStringBuffer &o_buf )
    {
        static ::rtl::OUString s_sDot(".");
        static ::rtl::OUString s_sParam0(" ( 1 = ? AND ");
        static ::rtl::OUString s_sParam1(" = ? OR 1 = ? AND ");
        static ::rtl::OUString s_sParam2(" IS NULL ) ");
        o_buf.append(s_sParam0);
        o_buf.append(tblName);
        o_buf.append(s_sDot);
        o_buf.append(colName);
        o_buf.append(s_sParam1);
        o_buf.append(tblName);
        o_buf.append(s_sDot);
        o_buf.append(colName);
        o_buf.append(s_sParam2);
    }
}

void OKeySet::setOneKeyColumnParameter( sal_Int32 &nPos, const Reference< XParameters > &_xParameter, const connectivity::ORowSetValue &_rValue, sal_Int32 _nType, sal_Int32 _nScale ) const
{
    if ( _rValue.isNull() )
    {
        _xParameter->setByte( nPos++, 0 );
        // We do the full call so that the right sqlType is passed to setNull
        setParameter( nPos++, _xParameter, _rValue, _nType, _nScale );
        _xParameter->setByte( nPos++, 1 );
    }
    else
    {
        _xParameter->setByte( nPos++, 1 );
        setParameter( nPos++, _xParameter, _rValue, _nType, _nScale );
        _xParameter->setByte( nPos++, 0 );
    }
}

::rtl::OUStringBuffer OKeySet::createKeyFilter()
{
    static ::rtl::OUString aAnd(" AND ");
    const ::rtl::OUString aQuote    = getIdentifierQuoteString();
    ::rtl::OUStringBuffer aFilter;
    // create the where clause
    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    SelectColumnsMetaData::iterator aPosEnd = m_pKeyColumnNames->end();
    for(SelectColumnsMetaData::iterator aPosIter = m_pKeyColumnNames->begin();aPosIter != aPosEnd;)
    {
        appendOneKeyColumnClause(::dbtools::quoteTableName( xMeta,aPosIter->second.sTableName,::dbtools::eInDataManipulation),
                                 ::dbtools::quoteName( aQuote,aPosIter->second.sRealName),
                                 aFilter);
        ++aPosIter;
        if(aPosIter != aPosEnd)
            aFilter.append(aAnd);
    }
    return aFilter;
}

void OKeySet::construct(const Reference< XResultSet>& _xDriverSet,const ::rtl::OUString& i_sRowSetFilter)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::construct" );
    OCacheSet::construct(_xDriverSet,i_sRowSetFilter);
    initColumns();

    Reference<XNameAccess> xKeyColumns  = getKeyColumns();
    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    Reference<XColumnsSupplier> xQueryColSup(m_xComposer,UNO_QUERY);
    const Reference<XNameAccess> xQueryColumns = xQueryColSup->getColumns();
    findTableColumnsMatching_throw(makeAny(m_xTable),m_sUpdateTableName,xMeta,xQueryColumns,m_pKeyColumnNames);

    // the first row is empty because it's now easier for us to distinguish when we are beforefirst or first
    // without extra variable to be set
    m_aKeyMap.insert(OKeySetMatrix::value_type(0,OKeySetValue(NULL,::std::pair<sal_Int32,Reference<XRow> >(0,NULL))));
    m_aKeyIter = m_aKeyMap.begin();

    ::rtl::OUStringBuffer aFilter = createKeyFilter();

    Reference< XSingleSelectQueryComposer> xSourceComposer(m_xComposer,UNO_QUERY);
    Reference< XMultiServiceFactory >  xFactory(m_xConnection, UNO_QUERY_THROW);
    Reference<XSingleSelectQueryComposer> xAnalyzer(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
    xAnalyzer->setElementaryQuery(xSourceComposer->getElementaryQuery());
    Reference<XTablesSupplier> xTabSup(xAnalyzer,uno::UNO_QUERY);
    Reference<XNameAccess> xSelectTables(xTabSup->getTables(),uno::UNO_QUERY);
    const Sequence< ::rtl::OUString> aSeq = xSelectTables->getElementNames();
    if ( aSeq.getLength() > 1 ) // special handling for join
    {
        static ::rtl::OUString aAnd(" AND ");
        const ::rtl::OUString aQuote    = getIdentifierQuoteString();
        const ::rtl::OUString* pIter = aSeq.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if ( *pIter != m_sUpdateTableName )
            {
                connectivity::OSQLTable xSelColSup(xSelectTables->getByName(*pIter),uno::UNO_QUERY);
                Reference<XPropertySet> xProp(xSelColSup,uno::UNO_QUERY);
                ::rtl::OUString sSelectTableName = ::dbtools::composeTableName( xMeta, xProp, ::dbtools::eInDataManipulation, false, false, false );

                ::dbaccess::getColumnPositions(xQueryColumns,xSelColSup->getColumns()->getElementNames(),sSelectTableName,(*m_pForeignColumnNames));

                const SelectColumnsMetaData::iterator aPosEnd = (*m_pForeignColumnNames).end();
                for(SelectColumnsMetaData::iterator aPosIter = (*m_pForeignColumnNames).begin();aPosIter != aPosEnd;++aPosIter)
                {
                    // look for columns not in the source columns to use them as filter as well
                    if ( aFilter.getLength() )
                        aFilter.append(aAnd);
                    appendOneKeyColumnClause(::dbtools::quoteName( aQuote,sSelectTableName),
                                             ::dbtools::quoteName( aQuote,aPosIter->second.sRealName),
                                             aFilter);
                }
                break;
            }
        }
    }
    executeStatement(aFilter,i_sRowSetFilter,xAnalyzer);
}
void OKeySet::executeStatement(::rtl::OUStringBuffer& io_aFilter,const ::rtl::OUString& i_sRowSetFilter,Reference<XSingleSelectQueryComposer>& io_xAnalyzer)
{
    bool bFilterSet = !i_sRowSetFilter.isEmpty();
    if ( bFilterSet )
    {
        FilterCreator aFilterCreator;
        aFilterCreator.append( i_sRowSetFilter );
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
                ::rtl::OUString sValue;
                if ( !(pAnd->Value >>= sValue) || !( sValue == "?" || sValue.matchAsciiL( ":",1,0 ) ) )
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
    m_xRow = NULL;
    ::comphelper::disposeComponent(m_xSet);
}

Any SAL_CALL OKeySet::getBookmark() throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getBookmark" );
    OSL_ENSURE(m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin(),
        "getBookmark is only possible when we stand on a valid row!");
    return makeAny(m_aKeyIter->first);
}

sal_Bool SAL_CALL OKeySet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::moveToBookmark" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(bookmark));
    invalidateRow();
    return m_aKeyIter != m_aKeyMap.end();
}

sal_Bool SAL_CALL OKeySet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::moveRelativeToBookmark" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(bookmark));
    if(m_aKeyIter != m_aKeyMap.end())
    {
        return relative(rows);
    }

    invalidateRow();
    return false;
}

sal_Int32 SAL_CALL OKeySet::compareBookmarks( const Any& _first, const Any& _second ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::compareBookmarks" );
    sal_Int32 nFirst = 0, nSecond = 0;
    _first >>= nFirst;
    _second >>= nSecond;

    return (nFirst != nSecond) ? CompareBookmark::NOT_EQUAL : CompareBookmark::EQUAL;
}

sal_Bool SAL_CALL OKeySet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::hasOrderedBookmarks" );
    return sal_True;
}

sal_Int32 SAL_CALL OKeySet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::hashBookmark" );
    return ::comphelper::getINT32(bookmark);
}

// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL OKeySet::deleteRows( const Sequence< Any >& rows ,const connectivity::OSQLTable& _xTable) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::deleteRows" );
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUStringBuffer aSql = ::rtl::OUString("DELETE FROM ");
    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString(" WHERE "));

    // list all columns that should be set
    const ::rtl::OUString aQuote    = getIdentifierQuoteString();
    static ::rtl::OUString aAnd(" AND ");
    static ::rtl::OUString aOr(" OR ");
    static ::rtl::OUString aEqual(" = ?");


    // use keys and indexes for excat postioning
    // first the keys
    Reference<XNameAccess> xKeyColumns = getKeyColumns();

    ::rtl::OUStringBuffer aCondition = ::rtl::OUString("( ");

    SelectColumnsMetaData::const_iterator aIter = (*m_pKeyColumnNames).begin();
    SelectColumnsMetaData::const_iterator aPosEnd = (*m_pKeyColumnNames).end();
    for(;aIter != aPosEnd;++aIter)
    {
        aCondition.append(::dbtools::quoteName( aQuote,aIter->second.sRealName));
        aCondition.append(aEqual);
        aCondition.append(aAnd);
    }
    aCondition.setLength(aCondition.getLength()-5);
    const ::rtl::OUString sCon( aCondition.makeStringAndClear() );

    const Any* pBegin   = rows.getConstArray();
    const Any* pEnd     = pBegin + rows.getLength();

    Sequence< Any > aKeys;
    for(;pBegin != pEnd;++pBegin)
    {
        aSql.append(sCon);
        aSql.append(aOr);
    }
    aSql.setLength(aSql.getLength()-3);

    // now create end execute the prepared statement

    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql.makeStringAndClear()));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    pBegin  = rows.getConstArray();
    sal_Int32 i=1;
    for(;pBegin != pEnd;++pBegin)
    {
        m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(*pBegin));
        if(m_aKeyIter != m_aKeyMap.end())
        {
            connectivity::ORowVector< ORowSetValue >::Vector::iterator aKeyIter = m_aKeyIter->second.first->get().begin();
            connectivity::ORowVector< ORowSetValue >::Vector::iterator aKeyEnd = m_aKeyIter->second.first->get().end();
            SelectColumnsMetaData::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
            for(sal_uInt16 j = 0;aKeyIter != aKeyEnd;++aKeyIter,++j,++aPosIter)
            {
                setParameter(i++,xParameter,*aKeyIter,aPosIter->second.nType,aPosIter->second.nScale);
            }
        }
    }

    sal_Bool bOk = xPrep->executeUpdate() > 0;
    Sequence< sal_Int32 > aRet(rows.getLength());
    memset(aRet.getArray(),bOk,sizeof(sal_Int32)*aRet.getLength());
    if(bOk)
    {
        pBegin  = rows.getConstArray();
        pEnd    = pBegin + rows.getLength();

        for(;pBegin != pEnd;++pBegin)
        {
            sal_Int32 nPos = 0;
            *pBegin >>= nPos;
            if(m_aKeyIter == m_aKeyMap.find(nPos) && m_aKeyIter != m_aKeyMap.end())
                ++m_aKeyIter;
            m_aKeyMap.erase(nPos);
            m_bDeleted = sal_True;
        }
    }
    return aRet;
}

void SAL_CALL OKeySet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::updateRow" );
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUStringBuffer aSql = ::rtl::OUString("UPDATE ");
    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString(" SET "));
    // list all cloumns that should be set
    static ::rtl::OUString aPara(" = ?,");
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd(" AND ");
    ::rtl::OUString sIsNull(" IS NULL");
    ::rtl::OUString sParam(" = ?");

    // use keys and indexes for excat postioning
    // first the keys
    Reference<XNameAccess> xKeyColumns = getKeyColumns();

    // second the indexes
    Reference<XIndexesSupplier> xIndexSup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xIndexes;
    if ( xIndexSup.is() )
        xIndexes.set(xIndexSup->getIndexes(),UNO_QUERY);


    ::std::vector< Reference<XNameAccess> > aAllIndexColumns;
    lcl_fillIndexColumns(xIndexes,aAllIndexColumns);

    ::rtl::OUString aColumnName;
    ::rtl::OUStringBuffer sKeyCondition,sIndexCondition;
    ::std::vector<sal_Int32> aIndexColumnPositions;

    const sal_Int32 nOldLength = aSql.getLength();
    sal_Int32 i = 1;
    // here we build the condition part for the update statement
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(;aIter != aEnd;++aIter,++i)
    {
        if ( m_pKeyColumnNames->find(aIter->first) != m_pKeyColumnNames->end() )
        {
            sKeyCondition.append(::dbtools::quoteName( aQuote,aIter->second.sRealName));
            if((_rOrginalRow->get())[aIter->second.nPosition].isNull())
                sKeyCondition.append(sIsNull);
            else
                sKeyCondition.append(sParam);
            sKeyCondition.append(aAnd);
        }
        else
        {
            ::std::vector< Reference<XNameAccess> >::const_iterator aIndexEnd = aAllIndexColumns.end();
            for( ::std::vector< Reference<XNameAccess> >::const_iterator aIndexIter = aAllIndexColumns.begin();
                aIndexIter != aIndexEnd;++aIndexIter)
            {
                if((*aIndexIter)->hasByName(aIter->first))
                {
                    sIndexCondition.append(::dbtools::quoteName( aQuote,aIter->second.sRealName));
                    if((_rOrginalRow->get())[aIter->second.nPosition].isNull())
                        sIndexCondition.append(sIsNull);
                    else
                    {
                        sIndexCondition.append(sParam);
                        aIndexColumnPositions.push_back(aIter->second.nPosition);
                    }
                    sIndexCondition.append(aAnd);
                    break;
                }
            }
        }
        if((_rInsertRow->get())[aIter->second.nPosition].isModified())
        {
            aSql.append(::dbtools::quoteName( aQuote,aIter->second.sRealName));
            aSql.append(aPara);
        }
    }

    if( aSql.getLength() != nOldLength )
    {
        aSql.setLength(aSql.getLength()-1);
    }
    else
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_VALUE_CHANGED ), SQL_GENERAL_ERROR, m_xConnection );

    if(sKeyCondition.getLength() || sIndexCondition.getLength())
    {
        aSql.append(::rtl::OUString(" WHERE "));
        if(sKeyCondition.getLength() && sIndexCondition.getLength())
        {
            aSql.append(sKeyCondition.makeStringAndClear());
            aSql.append(sIndexCondition.makeStringAndClear());
        }
        else if(sKeyCondition.getLength())
        {
            aSql.append(sKeyCondition.makeStringAndClear());
        }
        else if(sIndexCondition.getLength())
        {
            aSql.append(sIndexCondition.makeStringAndClear());
        }
        aSql.setLength(aSql.getLength()-5); // remove the last AND
    }
    else
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_CONDITION_FOR_PK ), SQL_GENERAL_ERROR, m_xConnection );

    // now create end execute the prepared statement
    ::rtl::OUString sEmpty;
    executeUpdate(_rInsertRow ,_rOrginalRow,aSql.makeStringAndClear(),sEmpty,aIndexColumnPositions);
}

void OKeySet::executeUpdate(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const ::rtl::OUString& i_sSQL,const ::rtl::OUString& i_sTableName,const ::std::vector<sal_Int32>& _aIndexColumnPositions)
{
    // now create end execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(i_sSQL));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    bool bRefetch = true;
    Reference<XRow> xRow;
    sal_Int32 i = 1;
    // first the set values
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    sal_uInt16 j = 0;
    for(;aIter != aEnd;++aIter,++j)
    {
        if ( i_sTableName.isEmpty() || aIter->second.sTableName == i_sTableName )
        {
            sal_Int32 nPos = aIter->second.nPosition;
            if((_rInsertRow->get())[nPos].isModified())
            {
                if ( bRefetch )
                {
                    bRefetch = ::std::find(m_aFilterColumns.begin(),m_aFilterColumns.end(),aIter->second.sRealName) == m_aFilterColumns.end();
                }
                impl_convertValue_throw(_rInsertRow,aIter->second);
                (_rInsertRow->get())[nPos].setSigned((_rOrginalRow->get())[nPos].isSigned());
                setParameter(i++,xParameter,(_rInsertRow->get())[nPos],aIter->second.nType,aIter->second.nScale);
            }
        }
    }
    // and then the values of the where condition
    aIter = m_pKeyColumnNames->begin();
    aEnd = m_pKeyColumnNames->end();
    j = 0;
    for(;aIter != aEnd;++aIter,++j)
    {
        if ( i_sTableName.isEmpty() || aIter->second.sTableName == i_sTableName )
        {
            setParameter(i++,xParameter,(_rOrginalRow->get())[aIter->second.nPosition],aIter->second.nType,aIter->second.nScale);
        }
    }
    if ( !_aIndexColumnPositions.empty() )
    {
        // now we have to set the index values
        ::std::vector<sal_Int32>::const_iterator aIdxColIter = _aIndexColumnPositions.begin();
        ::std::vector<sal_Int32>::const_iterator aIdxColEnd = _aIndexColumnPositions.end();
        j = 0;
        aIter = m_pColumnNames->begin();
        for(;aIdxColIter != aIdxColEnd;++aIdxColIter,++i,++j,++aIter)
        {
            setParameter(i,xParameter,(_rOrginalRow->get())[*aIdxColIter],(_rOrginalRow->get())[*aIdxColIter].getTypeKind(),aIter->second.nScale);
        }
    }
    const sal_Int32 nRowsUpdated = xPrep->executeUpdate();
     m_bUpdated =  nRowsUpdated > 0;
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

void SAL_CALL OKeySet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::insertRow" );
    ::rtl::OUStringBuffer aSql(::rtl::OUString("INSERT INTO "));
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString(" ( "));
    // set values and column names
    ::rtl::OUStringBuffer aValues(::rtl::OUString(" VALUES ( "));
    static ::rtl::OUString aPara("?,");
    ::rtl::OUString aQuote = getIdentifierQuoteString();
    static ::rtl::OUString aComma(",");

    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    sal_Int32 j = 1;
    bool bRefetch = true;
    sal_Bool bModified = sal_False;
    for(;aIter != aEnd;++aIter,++j)
    {
        if((_rInsertRow->get())[aIter->second.nPosition].isModified())
        {
            if ( bRefetch )
            {
                bRefetch = ::std::find(m_aFilterColumns.begin(),m_aFilterColumns.end(),aIter->second.sRealName) == m_aFilterColumns.end();
            }
            aSql.append(::dbtools::quoteName( aQuote,aIter->second.sRealName));
            aSql.append(aComma);
            aValues.append(aPara);
            bModified = sal_True;
        }
    }
    if ( !bModified )
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_VALUE_CHANGED ), SQL_GENERAL_ERROR, m_xConnection );

    aSql[aSql.getLength() - 1] = ')';
    aValues[aValues.getLength() - 1] = ')';
    aSql.append(aValues.makeStringAndClear());
    // now create,fill and execute the prepared statement
    ::rtl::OUString sEmpty;
    executeInsert(_rInsertRow,aSql.makeStringAndClear(),sEmpty,bRefetch);
}

void OKeySet::executeInsert( const ORowSetRow& _rInsertRow,const ::rtl::OUString& i_sSQL,const ::rtl::OUString& i_sTableName,bool bRefetch )
{
    // now create,fill and execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(i_sSQL));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(sal_Int32 i = 1;aIter != aEnd;++aIter)
    {
        if ( i_sTableName.isEmpty() || aIter->second.sTableName == i_sTableName )
        {
            const sal_Int32 nPos = aIter->second.nPosition;
            if((_rInsertRow->get())[nPos].isModified())
            {
                if((_rInsertRow->get())[nPos].isNull())
                    xParameter->setNull(i++,(_rInsertRow->get())[nPos].getTypeKind());
                else
                {
                    impl_convertValue_throw(_rInsertRow,aIter->second);
                    (_rInsertRow->get())[nPos].setSigned(m_aSignedFlags[nPos-1]);
                    setParameter(i++,xParameter,(_rInsertRow->get())[nPos],aIter->second.nType,aIter->second.nScale);
                }
            }
        }
    }

    m_bInserted = xPrep->executeUpdate() > 0;
    sal_Bool bAutoValuesFetched = sal_False;
    if ( m_bInserted )
    {
        // first insert the default values into the insertrow
        aIter = m_pColumnNames->begin();
        for(;aIter != aEnd;++aIter)
        {
            if ( !(_rInsertRow->get())[aIter->second.nPosition].isModified() )
                (_rInsertRow->get())[aIter->second.nPosition] = aIter->second.sDefaultValue;
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
                    ::std::vector< ::rtl::OUString >::iterator aAutoIter = m_aAutoColumns.begin();
                    ::std::vector< ::rtl::OUString >::iterator aAutoEnd = m_aAutoColumns.end();
                    for (sal_Int32 i = 1;aAutoIter !=  aAutoEnd && i <= nColumnCount; ++aAutoIter,++i)
                    {
#if OSL_DEBUG_LEVEL > 1
                        ::rtl::OUString sColumnName( xMd->getColumnName(i) );
#endif
                        SelectColumnsMetaData::iterator aFind = m_pKeyColumnNames->find(*aAutoIter);
                        if ( aFind != m_pKeyColumnNames->end() )
                            (_rInsertRow->get())[aFind->second.nPosition].fill(i, aFind->second.nType, xRow);
                    }
                    bAutoValuesFetched = sal_True;
                }
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("Could not execute GeneratedKeys() stmt");
        }
    }

    ::comphelper::disposeComponent(xPrep);

    if ( i_sTableName.isEmpty() && !bAutoValuesFetched && m_bInserted )
    {
        // first check if all key column values were set
        const ::rtl::OUString sMax(" MAX(");
        const ::rtl::OUString sMaxEnd("),");
        const ::rtl::OUString sQuote = getIdentifierQuoteString();
        ::rtl::OUString sMaxStmt;
        aEnd = m_pKeyColumnNames->end();
        ::std::vector< ::rtl::OUString >::iterator aAutoIter = m_aAutoColumns.begin();
        ::std::vector< ::rtl::OUString >::iterator aAutoEnd = m_aAutoColumns.end();
        for (;aAutoIter !=  aAutoEnd; ++aAutoIter)
        {
            // we will only fetch values which are keycolumns
            SelectColumnsMetaData::iterator aFind = m_pKeyColumnNames->find(*aAutoIter);
            if ( aFind != aEnd )
            {
                sMaxStmt += sMax;
                sMaxStmt += ::dbtools::quoteName( sQuote,aFind->second.sRealName
);
                sMaxStmt += sMaxEnd;
            }
        }

        if(!sMaxStmt.isEmpty())
        {
            sMaxStmt = sMaxStmt.replaceAt(sMaxStmt.getLength()-1,1,::rtl::OUString(" "));
            ::rtl::OUString sStmt = ::rtl::OUString("SELECT ");
            sStmt += sMaxStmt;
            sStmt += ::rtl::OUString("FROM ");
            ::rtl::OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(m_xConnection->getMetaData(),m_sUpdateTableName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
            sStmt += ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable );
            try
            {
                // now fetch the autoincrement values
                Reference<XStatement> xStatement = m_xConnection->createStatement();
                Reference<XResultSet> xRes = xStatement->executeQuery(sStmt);
                Reference<XRow> xRow(xRes,UNO_QUERY);
                if(xRow.is() && xRes->next())
                {
                    aAutoIter = m_aAutoColumns.begin();
                    for (sal_Int32 i=1;aAutoIter != aAutoEnd; ++aAutoIter,++i)
                    {
                        // we will only fetch values which are keycolumns
                        SelectColumnsMetaData::iterator aFind = m_pKeyColumnNames->find(*aAutoIter);
                        if ( aFind != aEnd )
                            (_rInsertRow->get())[aFind->second.nPosition].fill(i, aFind->second.nType, xRow);
                    }
                }
                ::comphelper::disposeComponent(xStatement);
            }
            catch(SQLException&)
            {
                OSL_FAIL("Could not fetch with MAX() ");
            }
        }
    }
    if ( m_bInserted )
    {
        OKeySetMatrix::iterator aKeyIter = m_aKeyMap.end();
        --aKeyIter;
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >(m_pKeyColumnNames->size());
        copyRowValue(_rInsertRow,aKeyRow,aKeyIter->first + 1);

        m_aKeyIter = m_aKeyMap.insert(OKeySetMatrix::value_type(aKeyIter->first + 1,OKeySetValue(aKeyRow,::std::pair<sal_Int32,Reference<XRow> >(1,NULL)))).first;
        // now we set the bookmark for this row
        (_rInsertRow->get())[0] = makeAny((sal_Int32)m_aKeyIter->first);
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

void OKeySet::copyRowValue(const ORowSetRow& _rInsertRow,ORowSetRow& _rKeyRow,sal_Int32 i_nBookmark)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::copyRowValue" );
    connectivity::ORowVector< ORowSetValue >::Vector::iterator aIter = _rKeyRow->get().begin();

    // check the if the parameter values have been changed
    OSL_ENSURE((m_aParameterValueForCache.get().size()-1) == m_pParameterNames->size(),"OKeySet::copyRowValue: Parameter values and names differ!");
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aParaValuesIter = m_aParameterValueForCache.get().begin() +1;

    bool bChanged = false;
    SelectColumnsMetaData::const_iterator aParaIter = (*m_pParameterNames).begin();
    SelectColumnsMetaData::const_iterator aParaEnd = (*m_pParameterNames).end();
    for(sal_Int32 i = 1;aParaIter != aParaEnd;++aParaIter,++aParaValuesIter,++i)
    {
        ORowSetValue aValue(*aParaValuesIter);
        aValue.setSigned(m_aSignedFlags[aParaIter->second.nPosition]);
        if ( (_rInsertRow->get())[aParaIter->second.nPosition] != aValue )
        {
            ORowSetValueVector aCopy(m_aParameterValueForCache);
            (aCopy.get())[i] = (_rInsertRow->get())[aParaIter->second.nPosition];
            m_aUpdatedParameter[i_nBookmark] = aCopy;
            bChanged = true;
        }
    }
    if ( !bChanged )
    {
        m_aUpdatedParameter.erase(i_nBookmark);
    }

    // update the key values
    SelectColumnsMetaData::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
    SelectColumnsMetaData::const_iterator aPosEnd = (*m_pKeyColumnNames).end();
    for(;aPosIter != aPosEnd;++aPosIter,++aIter)
    {
        impl_convertValue_throw(_rInsertRow,aPosIter->second);
        *aIter = (_rInsertRow->get())[aPosIter->second.nPosition];
        aIter->setTypeKind(aPosIter->second.nType);
    }
}

void SAL_CALL OKeySet::deleteRow(const ORowSetRow& _rDeleteRow,const connectivity::OSQLTable& _xTable   ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::deleteRow" );
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUStringBuffer aSql = ::rtl::OUString("DELETE FROM ");
    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString(" WHERE "));

    // list all cloumns that should be set
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd(" AND ");

    // use keys and indexes for excat postioning
    Reference<XNameAccess> xKeyColumns = getKeyColumns();
    // second the indexes
    Reference<XIndexesSupplier> xIndexSup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xIndexes;
    if ( xIndexSup.is() )
        xIndexes.set(xIndexSup->getIndexes(),UNO_QUERY);

    //  Reference<XColumnsSupplier>
    ::std::vector< Reference<XNameAccess> > aAllIndexColumns;
    lcl_fillIndexColumns(xIndexes,aAllIndexColumns);

    ::rtl::OUString aColumnName;
    ::rtl::OUStringBuffer sIndexCondition;
    ::std::vector<sal_Int32> aIndexColumnPositions;
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();

    sal_Int32 i = 1;
    for(i = 1;aIter != aEnd;++aIter,++i)
    {
        if ( m_pKeyColumnNames->find(aIter->first) != m_pKeyColumnNames->end() )
        {
            aSql.append(::dbtools::quoteName( aQuote,aIter->second.sRealName));
            if((_rDeleteRow->get())[aIter->second.nPosition].isNull())
            {
                OSL_FAIL("can a primary key be null");
                aSql.append(::rtl::OUString(" IS NULL"));
            }
            else
                aSql.append(::rtl::OUString(" = ?"));
            aSql.append(aAnd);
        }
        else
        {
            ::std::vector< Reference<XNameAccess> >::const_iterator aIndexEnd = aAllIndexColumns.end();
            for( ::std::vector< Reference<XNameAccess> >::const_iterator aIndexIter = aAllIndexColumns.begin();
                    aIndexIter != aIndexEnd;++aIndexIter)
            {
                if((*aIndexIter)->hasByName(aIter->first))
                {
                    sIndexCondition.append(::dbtools::quoteName( aQuote,aIter->second.sRealName));
                    if((_rDeleteRow->get())[aIter->second.nPosition].isNull())
                        sIndexCondition.append(::rtl::OUString(" IS NULL"));
                    else
                    {
                        sIndexCondition.append(::rtl::OUString(" = ?"));
                        aIndexColumnPositions.push_back(aIter->second.nPosition);
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

    aIter = (*m_pKeyColumnNames).begin();
    aEnd = (*m_pKeyColumnNames).end();
    i = 1;
    for(;aIter != aEnd;++aIter,++i)
    {
        setParameter(i,xParameter,(_rDeleteRow->get())[aIter->second.nPosition],aIter->second.nType,aIter->second.nScale);
    }

    // now we have to set the index values
    ::std::vector<sal_Int32>::iterator aIdxColIter = aIndexColumnPositions.begin();
    ::std::vector<sal_Int32>::iterator aIdxColEnd = aIndexColumnPositions.end();
    aIter = m_pColumnNames->begin();
    for(;aIdxColIter != aIdxColEnd;++aIdxColIter,++i,++aIter)
    {
        setParameter(i,xParameter,(_rDeleteRow->get())[*aIdxColIter],(_rDeleteRow->get())[*aIdxColIter].getTypeKind(),aIter->second.nScale);
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

void SAL_CALL OKeySet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::cancelRowUpdates" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
}

void SAL_CALL OKeySet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::moveToInsertRow" );
}

void SAL_CALL OKeySet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::moveToCurrentRow" );
}

Reference<XNameAccess> OKeySet::getKeyColumns() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getKeyColumns" );
    // use keys and indexes for excat postioning
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

sal_Bool SAL_CALL OKeySet::next(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::next" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;

    if(isAfterLast())
        return sal_False;
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

sal_Bool SAL_CALL OKeySet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::isBeforeFirst" );
    return m_aKeyIter == m_aKeyMap.begin();
}

sal_Bool SAL_CALL OKeySet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::isAfterLast" );
    return  m_bRowCountFinal && m_aKeyIter == m_aKeyMap.end();
}

sal_Bool SAL_CALL OKeySet::isFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::isFirst" );
    OKeySetMatrix::iterator aTemp = m_aKeyMap.begin();
    ++aTemp;
    return m_aKeyIter == aTemp && m_aKeyIter != m_aKeyMap.end();
}

sal_Bool SAL_CALL OKeySet::isLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::isLast" );
    if(!m_bRowCountFinal)
        return sal_False;

    OKeySetMatrix::iterator aTemp = m_aKeyMap.end();
    --aTemp;
    return m_aKeyIter == aTemp;
}

void SAL_CALL OKeySet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::beforeFirst" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.begin();
    invalidateRow();
}

void SAL_CALL OKeySet::afterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::afterLast" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();
    m_aKeyIter = m_aKeyMap.end();
    invalidateRow();
}

sal_Bool SAL_CALL OKeySet::first(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::first" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
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

sal_Bool SAL_CALL OKeySet::last(  ) throw(SQLException, RuntimeException)
{
    return last_checked(sal_True);
}

sal_Bool OKeySet::last_checked( sal_Bool /* i_bFetchRow */ )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::last_checked" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    bool fetchedRow = fillAllRows();

    m_aKeyIter = m_aKeyMap.end();
    --m_aKeyIter;
    if ( !fetchedRow )
    {
        invalidateRow();
    }
    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}

sal_Int32 SAL_CALL OKeySet::getRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getRow" );
    OSL_ENSURE(!isAfterLast(),"getRow is not allowed when afterlast record!");
    OSL_ENSURE(!isBeforeFirst(),"getRow is not allowed when beforefirst record!");

    return ::std::distance(m_aKeyMap.begin(),m_aKeyIter);
}

sal_Bool SAL_CALL OKeySet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    return absolute_checked(row,sal_True);
}
sal_Bool OKeySet::absolute_checked( sal_Int32 row, sal_Bool /* i_bFetchRow */ )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::absolute" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    OSL_ENSURE(row,"absolute(0) isn't allowed!");
    bool fetchedRow = false;
    if(row < 0)
    {
        if(!m_bRowCountFinal)
            fetchedRow = fillAllRows();

        for(;row < 0 && m_aKeyIter != m_aKeyMap.begin();++row)
            --m_aKeyIter;
    }
    else
    {
        if(row >= (sal_Int32)m_aKeyMap.size())
        {
            // we don't have this row
            if(!m_bRowCountFinal)
            {
                // but there may still be rows to fetch.
                sal_Bool bNext = sal_True;
                for(sal_Int32 i=m_aKeyMap.size()-1;i < row && bNext;++i)
                    bNext = fetchRow();
                // it is guaranteed that the above loop has executed at least once,
                // that is fetchRow called at least once.
                if ( bNext )
                {
                    fetchedRow = true;
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
            m_aKeyIter = m_aKeyMap.begin();
            for(;row > 0 && m_aKeyIter != m_aKeyMap.end();--row)
                ++m_aKeyIter;
        }
    }
    if ( !fetchedRow )
    {
        invalidateRow();
    }

    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}

sal_Bool SAL_CALL OKeySet::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::relative" );
    if(!rows)
    {
        invalidateRow();
        return sal_True;
    }
    return absolute(getRow()+rows);
}

sal_Bool OKeySet::previous_checked( sal_Bool /* i_bFetchRow */ )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::previous" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    if(m_aKeyIter != m_aKeyMap.begin())
    {
        --m_aKeyIter;
        invalidateRow();
    }
    return m_aKeyIter != m_aKeyMap.begin();
}
sal_Bool SAL_CALL OKeySet::previous(  ) throw(SQLException, RuntimeException)
{
    return previous_checked(sal_True);
}

bool OKeySet::doTryRefetch_throw()  throw(SQLException, RuntimeException)
{
    // we just reassign the base members
    Reference< XParameters > xParameter(m_xStatement,UNO_QUERY);
    OSL_ENSURE(xParameter.is(),"No Parameter interface!");
    xParameter->clearParameters();

    sal_Int32 nPos=1;
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aParaIter;
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aParaEnd;
    OUpdatedParameter::iterator aUpdateFind = m_aUpdatedParameter.find(m_aKeyIter->first);
    if ( aUpdateFind == m_aUpdatedParameter.end() )
    {
        aParaIter = m_aParameterValueForCache.get().begin();
        aParaEnd = m_aParameterValueForCache.get().end();
    }
    else
    {
        aParaIter = aUpdateFind->second.get().begin();
        aParaEnd = aUpdateFind->second.get().end();
    }

    for(++aParaIter;aParaIter != aParaEnd;++aParaIter,++nPos)
    {
        ::dbtools::setObjectWithInfo( xParameter, nPos, aParaIter->makeAny(), aParaIter->getTypeKind() );
    }

    // now set the primary key column values
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aIter = m_aKeyIter->second.first->get().begin();
    SelectColumnsMetaData::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
    SelectColumnsMetaData::const_iterator aPosEnd = (*m_pKeyColumnNames).end();
    for(;aPosIter != aPosEnd;++aPosIter,++aIter)
        setOneKeyColumnParameter(nPos,xParameter,*aIter,aPosIter->second.nType,aPosIter->second.nScale);
    aPosIter = (*m_pForeignColumnNames).begin();
    aPosEnd = (*m_pForeignColumnNames).end();
    for(;aPosIter != aPosEnd;++aPosIter,++aIter)
        setOneKeyColumnParameter(nPos,xParameter,*aIter,aPosIter->second.nType,aPosIter->second.nScale);

    m_xSet = m_xStatement->executeQuery();
    OSL_ENSURE(m_xSet.is(),"No resultset from statement!");
    return m_xSet->next();
}

// -----------------------------------------------------------------------------
void SAL_CALL OKeySet::refreshRow() throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::refreshRow" );

    invalidateRow();

    if(isBeforeFirst() || isAfterLast() || !m_xStatement.is())
        return;

    if ( m_aKeyIter->second.second.second.is() )
    {
        m_xRow = m_aKeyIter->second.second.second;
        return;
    }

    sal_Bool bOK = doTryRefetch_throw();
    if ( !bOK )
    {
        // This row has disappeared; remove it.
        OKeySetMatrix::iterator aTemp = m_aKeyIter;
        // use *next* row
        ++m_aKeyIter;
        m_aKeyMap.erase(aTemp);

        // adjust RowCount for the row we have removed
        if (m_rRowCount > 0)
            --m_rRowCount;
        else
            OSL_FAIL("m_rRowCount got out of sync: non-empty m_aKeyMap, but m_rRowCount <= 0");

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

sal_Bool OKeySet::fetchRow()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::fetchRow" );
    // fetch the next row and append on the keyset
    sal_Bool bRet = sal_False;
    if ( !m_bRowCountFinal && (!m_nMaxRows || sal_Int32(m_aKeyMap.size()) < m_nMaxRows) )
        bRet = m_xDriverSet->next();
    if ( bRet )
    {
        const int cc = m_xSetMetaData->getColumnCount();

        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >((*m_pKeyColumnNames).size() + m_pForeignColumnNames->size());
        ORowSetRow aFullRow = new connectivity::ORowVector< ORowSetValue >(cc);

        // Fetch the columns only once and in order, to satisfy restrictive backends such as ODBC
        connectivity::ORowVector< ORowSetValue >::Vector::iterator aFRIter = aFullRow->get().begin();
        // Column 0 is reserved for the bookmark; unused here.
        ++aFRIter;
        BOOST_STATIC_ASSERT(sizeof(int) >= sizeof(sal_Int32)); // "At least a 32 bit word expected"
        for (int i = 1; i <= cc; ++i, ++aFRIter )
        {
            aFRIter->fill(i, m_xSetMetaData->getColumnType(i), m_xDriverRow);
        }

        ::comphelper::disposeComponent(m_xSet);
        m_xRow.set(new OPrivateRow(aFullRow->get()));

        connectivity::ORowVector< ORowSetValue >::Vector::iterator aIter = aKeyRow->get().begin();
        // copy key columns
        SelectColumnsMetaData::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
        SelectColumnsMetaData::const_iterator aPosEnd = (*m_pKeyColumnNames).end();
        for(;aPosIter != aPosEnd;++aPosIter,++aIter)
        {
            const SelectColumnDescription& rColDesc = aPosIter->second;
            aIter->fill(rColDesc.nPosition, rColDesc.nType, m_xRow);
        }
        // copy missing columns from other tables
        aPosIter = (*m_pForeignColumnNames).begin();
        aPosEnd  = (*m_pForeignColumnNames).end();
        for(;aPosIter != aPosEnd;++aPosIter,++aIter)
        {
            const SelectColumnDescription& rColDesc = aPosIter->second;
            aIter->fill(rColDesc.nPosition, rColDesc.nType, m_xRow);
        }
        m_aKeyIter = m_aKeyMap.insert(OKeySetMatrix::value_type(m_aKeyMap.rbegin()->first+1,OKeySetValue(aKeyRow,::std::pair<sal_Int32,Reference<XRow> >(0,NULL)))).first;
    }
    else
        m_bRowCountFinal = sal_True;
    return bRet;
}

bool OKeySet::fillAllRows()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::fillAllRows" );
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
sal_Bool SAL_CALL OKeySet::wasNull(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::wasNull" );
    if ( ! m_xRow.is() )
        throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Must call getFOO() for some FOO before wasNull()")), *this);

    OSL_ENSURE(m_xRow.is(),"m_xRow is null! I've thrown, but function execution continued?");
    return m_xRow->wasNull();
}

inline void OKeySet::ensureRowForData( ) throw(SQLException, RuntimeException)
{
    if (! m_xRow.is() )
        refreshRow();
    if (! m_xRow.is() )
        throwSQLException("Failed to refetch row", "02000", *this, -2);

    OSL_ENSURE(m_xRow.is(),"m_xRow is null! I've called throwSQLException but execution continued?");
}

::rtl::OUString SAL_CALL OKeySet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getString" );
    ensureRowForData();
    return m_xRow->getString(columnIndex);
}

sal_Bool SAL_CALL OKeySet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getBoolean" );
    ensureRowForData();
    return m_xRow->getBoolean(columnIndex);
}

sal_Int8 SAL_CALL OKeySet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getByte" );
    ensureRowForData();
    return m_xRow->getByte(columnIndex);
}

sal_Int16 SAL_CALL OKeySet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getShort" );
    ensureRowForData();
    return m_xRow->getShort(columnIndex);
}

sal_Int32 SAL_CALL OKeySet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getInt" );
    ensureRowForData();
    return m_xRow->getInt(columnIndex);
}

sal_Int64 SAL_CALL OKeySet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getLong" );
    ensureRowForData();
    return m_xRow->getLong(columnIndex);
}

float SAL_CALL OKeySet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getFloat" );
    ensureRowForData();
    return m_xRow->getFloat(columnIndex);
}

double SAL_CALL OKeySet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getDouble" );
    ensureRowForData();
    return m_xRow->getDouble(columnIndex);
}

Sequence< sal_Int8 > SAL_CALL OKeySet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getBytes" );
    ensureRowForData();
    return m_xRow->getBytes(columnIndex);
}

::com::sun::star::util::Date SAL_CALL OKeySet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getDate" );
    ensureRowForData();
    return m_xRow->getDate(columnIndex);
}

::com::sun::star::util::Time SAL_CALL OKeySet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getTime" );
    ensureRowForData();
    return m_xRow->getTime(columnIndex);
}

::com::sun::star::util::DateTime SAL_CALL OKeySet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getTimestamp" );
    ensureRowForData();
    return m_xRow->getTimestamp(columnIndex);
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OKeySet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getBinaryStream" );
    ensureRowForData();
    return m_xRow->getBinaryStream(columnIndex);
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OKeySet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getCharacterStream" );
    ensureRowForData();
    return m_xRow->getCharacterStream(columnIndex);
}

Any SAL_CALL OKeySet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getObject" );
    ensureRowForData();
    return m_xRow->getObject(columnIndex,typeMap);
}

Reference< XRef > SAL_CALL OKeySet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getRef" );
    ensureRowForData();
    return m_xRow->getRef(columnIndex);
}

Reference< XBlob > SAL_CALL OKeySet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getBlob" );
    ensureRowForData();
    return m_xRow->getBlob(columnIndex);
}

Reference< XClob > SAL_CALL OKeySet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getClob" );
    ensureRowForData();
    return m_xRow->getClob(columnIndex);
}

Reference< XArray > SAL_CALL OKeySet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getArray" );
    ensureRowForData();
    return m_xRow->getArray(columnIndex);
}

sal_Bool SAL_CALL OKeySet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::rowUpdated" );
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second.first == 2;
}

sal_Bool SAL_CALL OKeySet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::rowInserted" );
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second.first == 1;
}

sal_Bool SAL_CALL OKeySet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::rowDeleted" );
    sal_Bool bDeleted = m_bDeleted;
    m_bDeleted = sal_False;
    return bDeleted;
}

namespace dbaccess
{

void getColumnPositions(const Reference<XNameAccess>& _rxQueryColumns,
                            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _aColumnNames,
                            const ::rtl::OUString& _rsUpdateTableName,
                            SelectColumnsMetaData& o_rColumnNames,
                            bool i_bAppendTableName)
    {
        // get the real name of the columns
        Sequence< ::rtl::OUString> aSelNames(_rxQueryColumns->getElementNames());
        const ::rtl::OUString* pSelIter     = aSelNames.getConstArray();
        const ::rtl::OUString* pSelEnd      = pSelIter + aSelNames.getLength();

        const ::rtl::OUString* pTblColumnIter   = _aColumnNames.getConstArray();
        const ::rtl::OUString* pTblColumnEnd    = pTblColumnIter + _aColumnNames.getLength();


        ::comphelper::UStringMixLess aTmp(o_rColumnNames.key_comp());
        ::comphelper::UStringMixEqual bCase(static_cast< ::comphelper::UStringMixLess*>(&aTmp)->isCaseSensitive());

        for(sal_Int32 nPos = 1;pSelIter != pSelEnd;++pSelIter,++nPos)
        {
            Reference<XPropertySet> xQueryColumnProp(_rxQueryColumns->getByName(*pSelIter),UNO_QUERY_THROW);
            ::rtl::OUString sRealName,sTableName;
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
                    ::rtl::OUString sColumnDefault;
                    if ( xQueryColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_DEFAULTVALUE) )
                        xQueryColumnProp->getPropertyValue(PROPERTY_DEFAULTVALUE) >>= sColumnDefault;

                    sal_Int32 nNullable = ColumnValue::NULLABLE_UNKNOWN;
                    OSL_VERIFY( xQueryColumnProp->getPropertyValue( PROPERTY_ISNULLABLE ) >>= nNullable );

                    if ( i_bAppendTableName )
                    {
                        ::rtl::OUStringBuffer sName;
                        sName.append(sTableName);
                        sName.appendAscii(".");
                        sName.append(sRealName);
                        SelectColumnDescription aColDesc( nPos, nType,nScale,nNullable != sdbc::ColumnValue::NO_NULLS, sColumnDefault );
                        aColDesc.sRealName = sRealName;
                        aColDesc.sTableName = sTableName;
                        o_rColumnNames[sName.makeStringAndClear()] = aColDesc;
                    }
                    else
                        o_rColumnNames[sRealName] = SelectColumnDescription( nPos, nType,nScale,nNullable != sdbc::ColumnValue::NO_NULLS, sColumnDefault );

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
                ::rtl::OUString sValue = aValue.getString();
                sal_Int32 nIndex = sValue.indexOf('.');
                if ( nIndex != -1 )
                {
                    aValue = sValue.copy(0,::std::min(sValue.getLength(),nIndex + (i_aMetaData.nScale > 0 ? i_aMetaData.nScale + 1 : 0)));
                }
            }
            break;
        default:
            break;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
