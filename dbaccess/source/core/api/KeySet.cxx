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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef DBACCESS_CORE_API_KEYSET_HXX
#include "KeySet.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#include <com/sun/star/sdbc/ColumnValue.hpp>
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCxParameterS_HPP_
#include <com/sun/star/sdbc/XParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XGENERATEDRESULTSET_HPP_
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XINDEXESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#include <list>
#include <algorithm>
#include <string.h>
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef DBACCESS_CORE_API_QUERYCOMPOSER_HXX
#include "querycomposer.hxx"
#endif
#ifndef DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX
#include "composertools.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include <string.h>
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
}
DBG_NAME(OKeySet)
// -------------------------------------------------------------------------
OKeySet::OKeySet(const connectivity::OSQLTable& _xTable,
                 const Reference< XIndexAccess>& _xTableKeys,
                 const ::rtl::OUString& _rUpdateTableName,    // this can be the alias or the full qualified name
                 const Reference< XSingleSelectQueryAnalyzer >& _xComposer,
                 const ORowSetValueVector& _aParameterValueForCache,
                 sal_Int32 i_nMaxRows)
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
            ,m_bRowCountFinal(sal_False)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::OKeySet" );
    DBG_CTOR(OKeySet,NULL);

}
// -----------------------------------------------------------------------------
OKeySet::~OKeySet()
{
    try
    {
        ::comphelper::disposeComponent(m_xStatement);
    }
    catch(Exception&)
    {
        m_xStatement = NULL;
    }
    catch(...)
    {
        OSL_ENSURE(0,"Unknown Exception occured");
    }
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
    if ( sUpdateTableName.getLength() == 0 )
    {
        OSL_ENSURE( false, "OKeySet::findTableColumnsMatching_throw: This is a fallback only - it won't work when the table has an alias name." );
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
        ::dbtools::throwGenericSQLException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Could not find any key column." ) ), *this );
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
::rtl::OUStringBuffer OKeySet::createKeyFilter()
{
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    const ::rtl::OUString aQuote    = getIdentifierQuoteString();
    ::rtl::OUStringBuffer aFilter;
    static ::rtl::OUString s_sDot(RTL_CONSTASCII_USTRINGPARAM("."));
    static ::rtl::OUString s_sParam(RTL_CONSTASCII_USTRINGPARAM(" = ?"));
    // create the where clause
    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    SelectColumnsMetaData::iterator aPosEnd = m_pKeyColumnNames->end();
    for(SelectColumnsMetaData::iterator aPosIter = m_pKeyColumnNames->begin();aPosIter != aPosEnd;)
    {
        aFilter.append(::dbtools::quoteTableName( xMeta,aPosIter->second.sTableName,::dbtools::eInDataManipulation));
        aFilter.append(s_sDot);
        aFilter.append(::dbtools::quoteName( aQuote,aPosIter->second.sRealName));
        aFilter.append(s_sParam);
        ++aPosIter;
        if(aPosIter != aPosEnd)
            aFilter.append(aAnd);
    }
    return aFilter;
}
// -----------------------------------------------------------------------------
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
    // without extra varaible to be set
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
        static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
        const ::rtl::OUString aQuote    = getIdentifierQuoteString();
        static ::rtl::OUString s_sDot(RTL_CONSTASCII_USTRINGPARAM("."));
        static ::rtl::OUString s_sParam(RTL_CONSTASCII_USTRINGPARAM(" = ?"));
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

                SelectColumnsMetaData::iterator aPosEnd = (*m_pForeignColumnNames).end();
                for(SelectColumnsMetaData::iterator aPosIter = (*m_pForeignColumnNames).begin();aPosIter != aPosEnd;++aPosIter)
                {
                    // look for columns not in the source columns to use them as filter as well
                    // if ( !xSourceColumns->hasByName(aPosIter->first) )
                    {
                        if ( aFilter.getLength() )
                            aFilter.append(aAnd);
                        aFilter.append(::dbtools::quoteName( aQuote,sSelectTableName));
                        aFilter.append(s_sDot);
                        aFilter.append(::dbtools::quoteName( aQuote,aPosIter->second.sRealName));
                        aFilter.append(s_sParam);
                    }
                }
                break;
            }
        }
    } // if ( aSeq.getLength() > 1 ) // special handling for join
    executeStatement(aFilter,i_sRowSetFilter,xAnalyzer);
}
void OKeySet::executeStatement(::rtl::OUStringBuffer& io_aFilter,const ::rtl::OUString& i_sRowSetFilter,Reference<XSingleSelectQueryComposer>& io_xAnalyzer)
{
    bool bFilterSet = i_sRowSetFilter.getLength() != 0;
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
                if ( !(pAnd->Value >>= sValue) || !(sValue.equalsAscii("?") || sValue.matchAsciiL(":",1,0)) )
                { // we have a criteria which has to be taken into account for updates
                    m_aFilterColumns.push_back(pAnd->Name);
                }
            }
        }
    }
    m_xStatement = m_xConnection->prepareStatement(io_xAnalyzer->getQueryWithSubstitution());
    ::comphelper::disposeComponent(io_xAnalyzer);
}
// -------------------------------------------------------------------------
Any SAL_CALL OKeySet::getBookmark() throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getBookmark" );
    OSL_ENSURE(m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin(),
        "getBookmark is only possible when we stand on a valid row!");
    return makeAny(m_aKeyIter->first);
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::moveToBookmark" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(bookmark));
    return m_aKeyIter != m_aKeyMap.end();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::moveRelativeToBookmark" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(bookmark));
    if(m_aKeyIter != m_aKeyMap.end())
    {
        relative(rows);
    }

    return !isBeforeFirst() && !isAfterLast();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OKeySet::compareBookmarks( const Any& _first, const Any& _second ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::compareBookmarks" );
    sal_Int32 nFirst = 0, nSecond = 0;
    _first >>= nFirst;
    _second >>= nSecond;

    return (nFirst != nSecond) ? CompareBookmark::NOT_EQUAL : CompareBookmark::EQUAL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::hasOrderedBookmarks" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OKeySet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::hashBookmark" );
    return ::comphelper::getINT32(bookmark);
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL OKeySet::deleteRows( const Sequence< Any >& rows ,const connectivity::OSQLTable& _xTable) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::deleteRows" );
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUStringBuffer aSql = ::rtl::OUString::createFromAscii("DELETE FROM ");
    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString::createFromAscii(" WHERE "));

    // list all cloumns that should be set
    const ::rtl::OUString aQuote    = getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    static ::rtl::OUString aOr      = ::rtl::OUString::createFromAscii(" OR ");
    static ::rtl::OUString aEqual   = ::rtl::OUString::createFromAscii(" = ?");


    // use keys and indexes for excat postioning
    // first the keys
    Reference<XNameAccess> xKeyColumns = getKeyColumns();

    ::rtl::OUStringBuffer aCondition = ::rtl::OUString::createFromAscii("( ");

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
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::updateRow" );
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUStringBuffer aSql = ::rtl::OUString::createFromAscii("UPDATE ");
    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString::createFromAscii(" SET "));
    // list all cloumns that should be set
    static ::rtl::OUString aPara    = ::rtl::OUString::createFromAscii(" = ?,");
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    ::rtl::OUString sIsNull(RTL_CONSTASCII_USTRINGPARAM(" IS NULL"));
    ::rtl::OUString sParam(RTL_CONSTASCII_USTRINGPARAM(" = ?"));

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
        //if(xKeyColumns.is() && xKeyColumns->hasByName(aIter->first))
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
        aSql.append(::rtl::OUString::createFromAscii(" WHERE "));
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
// -----------------------------------------------------------------------------
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
        if ( !i_sTableName.getLength() || aIter->second.sTableName == i_sTableName )
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
        if ( !i_sTableName.getLength() || aIter->second.sTableName == i_sTableName )
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
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::insertRow" );
    ::rtl::OUStringBuffer aSql(::rtl::OUString::createFromAscii("INSERT INTO "));
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString::createFromAscii(" ( "));
    // set values and column names
    ::rtl::OUStringBuffer aValues(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" VALUES ( ")));
    static ::rtl::OUString aPara(RTL_CONSTASCII_USTRINGPARAM("?,"));
    ::rtl::OUString aQuote = getIdentifierQuoteString();
    static ::rtl::OUString aComma(RTL_CONSTASCII_USTRINGPARAM(","));

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

    aSql.setCharAt(aSql.getLength()-1,')');
    aValues.setCharAt(aValues.getLength()-1,')');
    aSql.append(aValues.makeStringAndClear());
    // now create,fill and execute the prepared statement
    ::rtl::OUString sEmpty;
    executeInsert(_rInsertRow,aSql.makeStringAndClear(),sEmpty,bRefetch);
}
// -------------------------------------------------------------------------
void OKeySet::executeInsert( const ORowSetRow& _rInsertRow,const ::rtl::OUString& i_sSQL,const ::rtl::OUString& i_sTableName,bool bRefetch )
{
    // now create,fill and execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(i_sSQL));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(sal_Int32 i = 1;aIter != aEnd;++aIter)
    {
        if ( !i_sTableName.getLength() || aIter->second.sTableName == i_sTableName )
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
                            (_rInsertRow->get())[aFind->second.nPosition].fill(i,aFind->second.nType,aFind->second.bNullable,xRow);
                    }
                    bAutoValuesFetched = sal_True;
                }
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"Could not execute GeneratedKeys() stmt");
        }
    }

    ::comphelper::disposeComponent(xPrep);

    if ( !i_sTableName.getLength() && !bAutoValuesFetched && m_bInserted )
    {
        // first check if all key column values were set
        const ::rtl::OUString sMax(RTL_CONSTASCII_USTRINGPARAM(" MAX("));
        const ::rtl::OUString sMaxEnd(RTL_CONSTASCII_USTRINGPARAM("),"));
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

        if(sMaxStmt.getLength())
        {
            sMaxStmt = sMaxStmt.replaceAt(sMaxStmt.getLength()-1,1,::rtl::OUString::createFromAscii(" "));
            ::rtl::OUString sStmt = ::rtl::OUString::createFromAscii("SELECT ");
            sStmt += sMaxStmt;
            sStmt += ::rtl::OUString::createFromAscii("FROM ");
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
                            (_rInsertRow->get())[aFind->second.nPosition].fill(i,aFind->second.nType,aFind->second.bNullable,xRow);
                    }
                }
                ::comphelper::disposeComponent(xStatement);
            }
            catch(SQLException&)
            {
                OSL_ENSURE(0,"Could not fetch with MAX() ");
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
        // we just areassign the base members
        try
        {
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
            connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aIter2 = m_aKeyIter->second.first->get().begin();
            SelectColumnsMetaData::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
            SelectColumnsMetaData::const_iterator aPosEnd = (*m_pKeyColumnNames).end();
            for(;aPosIter != aPosEnd;++aPosIter,++aIter2,++nPos)
                setParameter(nPos,xParameter,*aIter2,aPosIter->second.nType,aPosIter->second.nScale);
            aPosIter = (*m_pForeignColumnNames).begin();
            aPosEnd = (*m_pForeignColumnNames).end();
            for(;aPosIter != aPosEnd;++aPosIter,++aIter2,++nPos)
                setParameter(nPos,xParameter,*aIter2,aPosIter->second.nType,aPosIter->second.nScale);

            m_xSet = m_xStatement->executeQuery();
            OSL_ENSURE(m_xSet.is(),"No resultset form statement!");
            bRefetch = m_xSet->next();
        }
        catch(Exception)
        {
            bRefetch = false;
        }
    }
    if ( !bRefetch )
    {
        m_aKeyIter->second.second.second = new OPrivateRow(_rInsertRow->get());
    }
}
// -----------------------------------------------------------------------------
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
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::deleteRow(const ORowSetRow& _rDeleteRow,const connectivity::OSQLTable& _xTable   ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::deleteRow" );
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUStringBuffer aSql = ::rtl::OUString::createFromAscii("DELETE FROM ");
    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString::createFromAscii(" WHERE "));

    // list all cloumns that should be set
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");

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
                OSL_ENSURE(0,"can a primary key be null");
                aSql.append(::rtl::OUString::createFromAscii(" IS NULL"));
            }
            else
                aSql.append(::rtl::OUString::createFromAscii(" = ?"));
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
                        sIndexCondition.append(::rtl::OUString::createFromAscii(" IS NULL"));
                    else
                    {
                        sIndexCondition.append(::rtl::OUString::createFromAscii(" = ?"));
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
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::cancelRowUpdates" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
}
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::moveToInsertRow" );
}
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::moveToCurrentRow" );
}
// -------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::next(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::next" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;

    if(isAfterLast())
        return sal_False;
    if(!m_bRowCountFinal) // not yet all records fetched
    {
        ++m_aKeyIter; // this is possible because we stand on begin() and this is the "beforefirst" row
        if(m_aKeyIter == m_aKeyMap.end() && !fetchRow())
            m_aKeyIter = m_aKeyMap.end();
    }
    else if(!isAfterLast())
        ++m_aKeyIter;

    refreshRow();
    return !isAfterLast();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::isBeforeFirst" );
    return m_aKeyIter == m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::isAfterLast" );
    return  m_bRowCountFinal && m_aKeyIter == m_aKeyMap.end();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::isFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::isFirst" );
    OKeySetMatrix::iterator aTemp = m_aKeyMap.begin();
    ++aTemp;
    return m_aKeyIter == aTemp && m_aKeyIter != m_aKeyMap.end();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::isLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::isLast" );
    if(!m_bRowCountFinal)
        return sal_False;

    OKeySetMatrix::iterator aTemp = m_aKeyMap.end();
    --aTemp;
    return m_aKeyIter == aTemp;
}
// -----------------------------------------------------------------------------
void SAL_CALL OKeySet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::beforeFirst" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.begin();
    m_xRow = NULL;
    ::comphelper::disposeComponent(m_xSet);
}
// -----------------------------------------------------------------------------
void SAL_CALL OKeySet::afterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::afterLast" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();
    m_aKeyIter = m_aKeyMap.end();
    m_xRow = NULL;
    ::comphelper::disposeComponent(m_xSet);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::first(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::first" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.begin();
    ++m_aKeyIter;
    if(m_aKeyIter == m_aKeyMap.end() && !fetchRow())
        m_aKeyIter = m_aKeyMap.end();

    refreshRow();
    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::last(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::last" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();

    m_aKeyIter = m_aKeyMap.end();
    --m_aKeyIter;
    refreshRow();
    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OKeySet::getRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getRow" );
    OSL_ENSURE(!isAfterLast(),"getRow is not allowed when afterlast record!");
    OSL_ENSURE(!isBeforeFirst(),"getRow is not allowed when beforefirst record!");

    return ::std::distance(m_aKeyMap.begin(),m_aKeyIter);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::absolute" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    OSL_ENSURE(row,"absolute(0) isn't allowed!");
    if(row < 0)
    {
        if(!m_bRowCountFinal)
            fillAllRows();

        for(;row < 0 && m_aKeyIter != m_aKeyMap.begin();++row)
            m_aKeyIter--;
    }
    else
    {
        if(row >= (sal_Int32)m_aKeyMap.size())
        {
            if(!m_bRowCountFinal)
            {
                sal_Bool bNext = sal_True;
                for(sal_Int32 i=m_aKeyMap.size()-1;i < row && bNext;++i)
                    bNext = fetchRow();
            }
            else
                m_aKeyIter = m_aKeyMap.end();
        }
        else
        {
            m_aKeyIter = m_aKeyMap.begin();
            for(;row > 0 && m_aKeyIter != m_aKeyMap.end();--row)
                ++m_aKeyIter;
        }
    }
    refreshRow();

    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::relative" );
    if(!rows)
    {
        refreshRow();
        return sal_True;
    }
    return absolute(getRow()+rows);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::previous(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::previous" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    if(m_aKeyIter != m_aKeyMap.begin())
    {
        --m_aKeyIter;
        refreshRow();
    }
    return m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
void SAL_CALL OKeySet::refreshRow() throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::refreshRow" );
    if(isBeforeFirst() || isAfterLast() || !m_xStatement.is())
        return;

    m_xRow = NULL;
    ::comphelper::disposeComponent(m_xSet);

    if ( m_aKeyIter->second.second.second.is() )
    {
        m_xRow = m_aKeyIter->second.second.second;
        return;
    }
    // we just areassign the base members
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
    for(;aPosIter != aPosEnd;++aPosIter,++aIter,++nPos)
        setParameter(nPos,xParameter,*aIter,aPosIter->second.nType,aPosIter->second.nScale);
    aPosIter = (*m_pForeignColumnNames).begin();
    aPosEnd = (*m_pForeignColumnNames).end();
    for(;aPosIter != aPosEnd;++aPosIter,++aIter,++nPos)
        setParameter(nPos,xParameter,*aIter,aPosIter->second.nType,aPosIter->second.nScale);

    m_xSet = m_xStatement->executeQuery();
    OSL_ENSURE(m_xSet.is(),"No resultset form statement!");
    sal_Bool bOK = m_xSet->next();
    if ( !bOK )
        m_aKeyIter = m_aKeyMap.end();
    m_xRow.set(m_xSet,UNO_QUERY);
    OSL_ENSURE(m_xRow.is(),"No row form statement!");
}
// -----------------------------------------------------------------------------
sal_Bool OKeySet::fetchRow()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::fetchRow" );
    // fetch the next row and append on the keyset
    sal_Bool bRet = sal_False;
    if ( !m_bRowCountFinal && (!m_nMaxRows || sal_Int32(m_aKeyMap.size()) < m_nMaxRows) )
        bRet = m_xDriverSet->next();
    if ( bRet )
    {
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >((*m_pKeyColumnNames).size() + m_pForeignColumnNames->size());
        connectivity::ORowVector< ORowSetValue >::Vector::iterator aIter = aKeyRow->get().begin();
        // first fetch the values needed for the key column
        SelectColumnsMetaData::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
        SelectColumnsMetaData::const_iterator aPosEnd = (*m_pKeyColumnNames).end();
        for(;aPosIter != aPosEnd;++aPosIter,++aIter)
        {
            const SelectColumnDescription& rColDesc = aPosIter->second;
            aIter->fill(rColDesc.nPosition,rColDesc.nType,rColDesc.bNullable,m_xDriverRow);
        }
        // now fetch the values from the missing columns from other tables
        aPosIter = (*m_pForeignColumnNames).begin();
        aPosEnd  = (*m_pForeignColumnNames).end();
        for(;aPosIter != aPosEnd;++aPosIter,++aIter)
        {
            const SelectColumnDescription& rColDesc = aPosIter->second;
            aIter->fill(rColDesc.nPosition,rColDesc.nType,rColDesc.bNullable,m_xDriverRow);
        }
        m_aKeyIter = m_aKeyMap.insert(OKeySetMatrix::value_type(m_aKeyMap.rbegin()->first+1,OKeySetValue(aKeyRow,::std::pair<sal_Int32,Reference<XRow> >(0,NULL)))).first;
    }
    else
        m_bRowCountFinal = sal_True;
    return bRet;
}
// -------------------------------------------------------------------------
void OKeySet::fillAllRows()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::fillAllRows" );
    if(!m_bRowCountFinal)
    {
        while(fetchRow())
            ;
    }
}
// XRow
sal_Bool SAL_CALL OKeySet::wasNull(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::wasNull" );
    return m_xRow->wasNull();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OKeySet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getString" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getString(columnIndex);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getBoolean" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBoolean(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL OKeySet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getByte" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getByte(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL OKeySet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getShort" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getShort(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OKeySet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getInt" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getInt(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL OKeySet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getLong" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getLong(columnIndex);
}
// -------------------------------------------------------------------------
float SAL_CALL OKeySet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getFloat" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getFloat(columnIndex);
}
// -------------------------------------------------------------------------
double SAL_CALL OKeySet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getDouble" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getDouble(columnIndex);
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OKeySet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getBytes" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBytes(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL OKeySet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getDate" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getDate(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL OKeySet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getTime" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getTime(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL OKeySet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getTimestamp" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getTimestamp(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OKeySet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getBinaryStream" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBinaryStream(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OKeySet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getCharacterStream" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getCharacterStream(columnIndex);
}
// -------------------------------------------------------------------------
Any SAL_CALL OKeySet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getObject" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getObject(columnIndex,typeMap);
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL OKeySet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getRef" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getRef(columnIndex);
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OKeySet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getBlob" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBlob(columnIndex);
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL OKeySet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getClob" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getClob(columnIndex);
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OKeySet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getArray" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getArray(columnIndex);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::rowUpdated" );
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second.first == 2;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::rowInserted" );
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second.first == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::rowDeleted" );
    sal_Bool bDeleted = m_bDeleted;
    m_bDeleted = sal_False;
    return bDeleted;
}
// -----------------------------------------------------------------------------
::rtl::OUString OKeySet::getComposedTableName(const ::rtl::OUString& _sCatalog,
                                              const ::rtl::OUString& _sSchema,
                                              const ::rtl::OUString& _sTable)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::getComposedTableName" );
    ::rtl::OUString aComposedName;
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();

    if( xMetaData.is() && xMetaData->supportsTableCorrelationNames() )
    {
        aComposedName = ::dbtools::composeTableName( xMetaData, _sCatalog, _sSchema, _sTable, sal_False, ::dbtools::eInDataManipulation );
        // first we have to check if the composed tablename is in the select clause or if an alias is used
        Reference<XTablesSupplier> xTabSup(m_xComposer,UNO_QUERY);
        Reference<XNameAccess> xSelectTables = xTabSup->getTables();
        OSL_ENSURE(xSelectTables.is(),"No Select tables!");
        if(xSelectTables.is())
        {
            if(!xSelectTables->hasByName(aComposedName))
            { // the composed name isn't used in the select clause so we have to find out which name is used instead
                ::rtl::OUString sCatalog,sSchema,sTable;
                ::dbtools::qualifiedNameComponents(xMetaData,m_sUpdateTableName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
                aComposedName = ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable );
            }
            else
                aComposedName = ::dbtools::composeTableNameForSelect( m_xConnection, _sCatalog, _sSchema, _sTable );
        }
    }
    else
        aComposedName = ::dbtools::composeTableNameForSelect( m_xConnection, _sCatalog, _sSchema, _sTable );

    return aComposedName;
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------

