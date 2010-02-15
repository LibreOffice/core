/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OptimisticSet.cxx,v $
 * $Revision: 1.73 $
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
#include "querycomposer.hxx"
#include "composertools.hxx"
#include <tools/debug.hxx>
#include <string.h>
#include <rtl/logfile.hxx>

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

DECLARE_STL_USTRINGACCESS_MAP(::rtl::OUStringBuffer,TSQLStatements);
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
    void lcl_fillKeyCondition(const ::rtl::OUString& i_sTableName,const ::rtl::OUString& i_sQuotedColumnName,const ORowSetValue& i_aValue,TSQLStatements& io_aKeyConditions)
    {
        ::rtl::OUStringBuffer& rKeyCondition = io_aKeyConditions[i_sTableName];
        if ( rKeyCondition.getLength() )
            rKeyCondition.appendAscii(" AND ");
        rKeyCondition.append(i_sQuotedColumnName);
        if ( i_aValue.isNull() )
            rKeyCondition.appendAscii(" IS NULL");
        else
            rKeyCondition.appendAscii(" = ?");
    }
}

DBG_NAME(OptimisticSet)
// -------------------------------------------------------------------------
OptimisticSet::OptimisticSet(const ::comphelper::ComponentContext& _rContext,
                             const Reference< XConnection>& i_xConnection,
                             const Reference< XSingleSelectQueryAnalyzer >& _xComposer,
                             const ORowSetValueVector& _aParameterValueForCache)
            :m_aSqlParser( _rContext.getLegacyServiceFactory() )
            ,m_aSqlIterator( i_xConnection, Reference<XTablesSupplier>(_xComposer,UNO_QUERY)->getTables(), m_aSqlParser, NULL )
            ,m_aParameterValueForCache(_aParameterValueForCache)
            ,m_pKeyColumnNames(NULL)
            ,m_pColumnNames(NULL)
            ,m_pParameterNames(NULL)
            ,m_xComposer(_xComposer)
            ,m_bRowCountFinal(sal_False)
            ,m_bResultSetChanged(false)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::OptimisticSet" );
    DBG_CTOR(OptimisticSet,NULL);
}
// -----------------------------------------------------------------------------
OptimisticSet::~OptimisticSet()
{
    try
    {
        ::comphelper::disposeComponent(m_xStatement);
    }
    catch(Exception&)
    {
        m_xStatement.clear();
    }
    catch(...)
    {
        OSL_ENSURE(0,"Unknown Exception occured");
    }
    m_xComposer.clear();

    DBG_DTOR(OptimisticSet,NULL);
}
// -----------------------------------------------------------------------------
void OptimisticSet::construct(const Reference< XResultSet>& _xDriverSet,const ::rtl::OUString& i_sRowSetFilter)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::construct" );
    OCacheSet::construct(_xDriverSet,i_sRowSetFilter);

    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    bool bCase = (xMeta.is() && xMeta->storesMixedCaseQuotedIdentifiers()) ? true : false;

    m_pKeyColumnNames.reset( new SelectColumnsMetaData(bCase) );
    m_pColumnNames.reset( new SelectColumnsMetaData(bCase) );
    m_pParameterNames.reset( new SelectColumnsMetaData(bCase) );

    Reference<XColumnsSupplier> xQueryColSup(m_xComposer,UNO_QUERY);
    const Reference<XNameAccess> xQueryColumns = xQueryColSup->getColumns();
    const Reference<XTablesSupplier> xTabSup(m_xComposer,UNO_QUERY);
    const Reference<XNameAccess> xTables = xTabSup->getTables();
    const Sequence< ::rtl::OUString> aTableNames = xTables->getElementNames();
    const ::rtl::OUString* pTableNameIter = aTableNames.getConstArray();
    const ::rtl::OUString* pTableNameEnd = pTableNameIter + aTableNames.getLength();
    for( ; pTableNameIter != pTableNameEnd ; ++pTableNameIter)
    {
        findTableColumnsMatching_throw(xTables->getByName(*pTableNameIter),xMeta,xQueryColumns);
    }

    // the first row is empty because it's now easier for us to distinguish when we are beforefirst or first
    // without extra variable to be set
    m_aKeyMap.insert(OKeySetMatrix::value_type(0,OKeySetValue(NULL,0)));
    m_aKeyIter = m_aKeyMap.begin();

    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    const ::rtl::OUString aQuote    = getIdentifierQuoteString();
    ::rtl::OUStringBuffer aFilter;
    static ::rtl::OUString s_sDot(RTL_CONSTASCII_USTRINGPARAM("."));
    static ::rtl::OUString s_sParam(RTL_CONSTASCII_USTRINGPARAM(" = ?"));
    // create the where clause
    SelectColumnsMetaData::iterator aPosEnd = m_pKeyColumnNames->end();
    for(SelectColumnsMetaData::iterator aPosIter = m_pKeyColumnNames->begin();aPosIter != aPosEnd;)
    {
        aFilter.append(::dbtools::quoteName( aQuote,aPosIter->second.sTableName));
        aFilter.append(s_sDot);
        aFilter.append(::dbtools::quoteName( aQuote,aPosIter->second.sRealName));
        aFilter.append(s_sParam);
        ++aPosIter;
        if(aPosIter != aPosEnd)
            aFilter.append(aAnd);
    }

    Reference< XMultiServiceFactory >  xFactory(m_xConnection, UNO_QUERY_THROW);
    Reference<XSingleSelectQueryComposer> xAnalyzer(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
    Reference<XSingleSelectQueryComposer> xComp2(m_xComposer,UNO_QUERY_THROW);
    ::rtl::OUString sQuery = xComp2->getQuery();
    xAnalyzer->setQuery(sQuery);
    // check for joins
    ::rtl::OUString aErrorMsg;
    ::std::auto_ptr<OSQLParseNode> pStatementNode( m_aSqlParser.parseTree( aErrorMsg, sQuery ) );
    m_aSqlIterator.setParseTree( pStatementNode.get() );
    m_aSqlIterator.traverseAll();
    fillJoinedColumns_throw(m_aSqlIterator.getJoinConditions());

    const ::rtl::OUString sComposerFilter = m_xComposer->getFilter();
    if ( i_sRowSetFilter.getLength() || (sComposerFilter.getLength() && sComposerFilter != i_sRowSetFilter) )
    {
        FilterCreator aFilterCreator;
        if ( sComposerFilter.getLength() && sComposerFilter != i_sRowSetFilter )
            aFilterCreator.append( sComposerFilter );
        aFilterCreator.append( i_sRowSetFilter );
        aFilterCreator.append( aFilter.makeStringAndClear() );
        aFilter = aFilterCreator.getComposedAndClear();
    }
    xAnalyzer->setFilter(aFilter.makeStringAndClear());
    m_xStatement = m_xConnection->prepareStatement(xAnalyzer->getQueryWithSubstitution());
    ::comphelper::disposeComponent(xAnalyzer);
}
// -------------------------------------------------------------------------
Any SAL_CALL OptimisticSet::getBookmark() throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getBookmark" );
    OSL_ENSURE(m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin(),
        "getBookmark is only possible when we stand on a valid row!");
    return makeAny(m_aKeyIter->first);
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::moveToBookmark" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(bookmark));
    return m_aKeyIter != m_aKeyMap.end();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::moveRelativeToBookmark" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(bookmark));
    if(m_aKeyIter != m_aKeyMap.end())
    {
        relative(rows);
    }

    return !isBeforeFirst() && !isAfterLast();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OptimisticSet::compareBookmarks( const Any& _first, const Any& _second ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::compareBookmarks" );
    sal_Int32 nFirst = 0, nSecond = 0;
    _first >>= nFirst;
    _second >>= nSecond;

    return (nFirst != nSecond) ? CompareBookmark::NOT_EQUAL : CompareBookmark::EQUAL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::hasOrderedBookmarks" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OptimisticSet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::hashBookmark" );
    return ::comphelper::getINT32(bookmark);
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL OptimisticSet::deleteRows( const Sequence< Any >& /*rows*/ ,const connectivity::OSQLTable& /*_xTable*/) throw(SQLException, RuntimeException)
{
    Sequence< sal_Int32 > aRet;
    return aRet;
}
// -------------------------------------------------------------------------
void SAL_CALL OptimisticSet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& /*_xTable*/  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::updateRow" );
    if ( m_aJoinedKeyColumns.empty() )
        throw SQLException();
    // list all cloumns that should be set
    static ::rtl::OUString s_sPara  = ::rtl::OUString::createFromAscii(" = ?");
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    ::rtl::OUString sIsNull(RTL_CONSTASCII_USTRINGPARAM(" IS NULL"));
    ::rtl::OUString sParam(RTL_CONSTASCII_USTRINGPARAM(" = ?"));

    ::rtl::OUString aColumnName;
    ::rtl::OUStringBuffer sKeyCondition;
    ::std::map< ::rtl::OUString,bool > aResultSetChanged;
    TSQLStatements aKeyConditions;
    TSQLStatements aIndexConditions;
    TSQLStatements aSql;

    // sal_Int32 i = 1;
    // here we build the condition part for the update statement
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(;aIter != aEnd;++aIter)
    {
        if ( aResultSetChanged.find( aIter->second.sTableName ) == aResultSetChanged.end() )
            aResultSetChanged[aIter->second.sTableName] = false;
        const ::rtl::OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,aIter->second.sRealName);
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
            ::rtl::OUStringBuffer& rPart = aSql[aIter->second.sTableName];
            if ( rPart.getLength() )
                rPart.appendAscii(", ");
            rPart.append(sQuotedColumnName);
            rPart.append(s_sPara);
        }
    }

    if( aSql.empty() )
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_VALUE_CHANGED ), SQL_GENERAL_ERROR, m_xConnection );

    if( aKeyConditions.empty() )
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_CONDITION_FOR_PK ), SQL_GENERAL_ERROR, m_xConnection );

    static const ::rtl::OUString s_sUPDATE(RTL_CONSTASCII_USTRINGPARAM("UPDATE "));
    static const ::rtl::OUString s_sSET(RTL_CONSTASCII_USTRINGPARAM(" SET "));

    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();

    TSQLStatements::iterator aSqlIter = aSql.begin();
    TSQLStatements::iterator aSqlEnd  = aSql.end();
    for(;aSqlIter != aSqlEnd ; ++aSqlIter)
    {
        if ( aSqlIter->second.getLength() )
        {
            m_bResultSetChanged = m_bResultSetChanged || aResultSetChanged[aSqlIter->first];
            ::rtl::OUStringBuffer sSql(s_sUPDATE);
            ::rtl::OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(xMetaData,aSqlIter->first,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
            sSql.append( ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable ) );
            sSql.append(s_sSET);
            sSql.append(aSqlIter->second);
            ::rtl::OUStringBuffer& rCondition = aKeyConditions[aSqlIter->first];
            bool bAddWhere = true;
            if ( rCondition.getLength() )
            {
                bAddWhere = false;
                sSql.appendAscii(" WHERE ");
                sSql.append( rCondition );
            }
            executeUpdate(_rInsertRow ,_rOrginalRow,sSql.makeStringAndClear(),aSqlIter->first);
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OptimisticSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& /*_xTable*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::insertRow" );
    TSQLStatements aSql;
    TSQLStatements aParameter;
    TSQLStatements aKeyConditions;
    ::std::map< ::rtl::OUString,bool > aResultSetChanged;
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    ::rtl::OUString sIsNull(RTL_CONSTASCII_USTRINGPARAM(" IS NULL"));
    ::rtl::OUString sParam(RTL_CONSTASCII_USTRINGPARAM(" = ?"));

    // here we build the condition part for the update statement
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(;aIter != aEnd;++aIter)
    {
        if ( aResultSetChanged.find( aIter->second.sTableName ) == aResultSetChanged.end() )
            aResultSetChanged[aIter->second.sTableName] = false;

        const ::rtl::OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,aIter->second.sRealName);
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
            ::rtl::OUStringBuffer& rPart = aSql[aIter->second.sTableName];
            if ( rPart.getLength() )
                rPart.appendAscii(", ");
            rPart.append(sQuotedColumnName);
            ::rtl::OUStringBuffer& rParam = aParameter[aIter->second.sTableName];
            if ( rParam.getLength() )
                rParam.appendAscii(", ");
            rParam.appendAscii("?");
        }
    }
    if ( aParameter.empty() )
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_VALUE_CHANGED ), SQL_GENERAL_ERROR, m_xConnection );

    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    static const ::rtl::OUString s_sINSERT(RTL_CONSTASCII_USTRINGPARAM("INSERT INTO "));
    static const ::rtl::OUString s_sVALUES(RTL_CONSTASCII_USTRINGPARAM(") VALUES ( "));
    TSQLStatements::iterator aSqlIter = aSql.begin();
    TSQLStatements::iterator aSqlEnd  = aSql.end();
    for(;aSqlIter != aSqlEnd ; ++aSqlIter)
    {
        if ( aSqlIter->second.getLength() )
        {
            m_bResultSetChanged = m_bResultSetChanged || aResultSetChanged[aSqlIter->first];
            ::rtl::OUStringBuffer sSql(s_sINSERT);
            ::rtl::OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(xMetaData,aSqlIter->first,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
            ::rtl::OUString sComposedTableName = ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable );
            sSql.append(sComposedTableName);
            sSql.appendAscii(" ( ");
            sSql.append(aSqlIter->second);
            sSql.append(s_sVALUES);
            sSql.append(aParameter[aSqlIter->first]);
            sSql.appendAscii(" )");

            ::rtl::OUStringBuffer& rCondition = aKeyConditions[aSqlIter->first];
            if ( rCondition.getLength() )
            {
                ::rtl::OUStringBuffer sQuery;
                sQuery.appendAscii("SELECT ");
                sQuery.append(aSqlIter->second);
                sQuery.appendAscii(" FROM ");
                sQuery.append(sComposedTableName);
                sQuery.appendAscii(" WHERE ");
                sQuery.append(rCondition);

                try
                {
                    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(sQuery.makeStringAndClear()));
                    Reference< XParameters > xParameter(xPrep,UNO_QUERY);
                    // and then the values of the where condition
                    SelectColumnsMetaData::iterator aIter = m_pKeyColumnNames->begin();
                    SelectColumnsMetaData::iterator aEnd = m_pKeyColumnNames->end();
                    sal_Int32 i = 1;
                    for(;aIter != aEnd;++aIter)
                    {
                        if ( aIter->second.sTableName == aSqlIter->first )
                        {
                            setParameter(i++,xParameter,(_rInsertRow->get())[aIter->second.nPosition],aIter->second.nType,aIter->second.nScale);
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

            executeInsert(_rInsertRow,sSql.makeStringAndClear(),aSqlIter->first);
        }
    }
}
// -------------------------------------------------------------------------
void OptimisticSet::executeInsert( const ORowSetRow& _rInsertRow,const ::rtl::OUString& i_sSQL,const ::rtl::OUString& i_sTableName )
{
    // now create,fill and execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(i_sSQL));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(sal_Int32 i = 1;aIter != aEnd;++aIter)
    {
        if ( aIter->second.sTableName == i_sTableName )
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
/*
    if ( !bAutoValuesFetched && m_bInserted )
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
            if ( m_pKeyColumnNames->find(*aAutoIter) != aEnd )
            {
                sMaxStmt += sMax;
                sMaxStmt += ::dbtools::quoteName( sQuote,*aAutoIter);
                sMaxStmt += sMaxEnd;
            }
        }

        if(sMaxStmt.getLength())
        {
            sMaxStmt = sMaxStmt.replaceAt(sMaxStmt.getLength()-1,1,::rtl::OUString::createFromAscii(" "));
            ::rtl::OUString sStmt = ::rtl::OUString::createFromAscii("SELECT ");
            sStmt += sMaxStmt;
            sStmt += ::rtl::OUString::createFromAscii("FROM ");
            sStmt += m_aSelectComposedTableName;
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
    */
    if ( m_bInserted )
    {
        OKeySetMatrix::iterator aKeyIter = m_aKeyMap.end();
        --aKeyIter;
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >(m_pKeyColumnNames->size());
        copyRowValue(_rInsertRow,aKeyRow,aKeyIter->first + 1);

        m_aKeyIter = m_aKeyMap.insert(OKeySetMatrix::value_type(aKeyIter->first + 1,OKeySetValue(aKeyRow,1))).first;
        // now we set the bookmark for this row
        (_rInsertRow->get())[0] = makeAny((sal_Int32)m_aKeyIter->first);
    }
}
// -----------------------------------------------------------------------------
void OptimisticSet::copyRowValue(const ORowSetRow& _rInsertRow,ORowSetRow& _rKeyRow,sal_Int32 i_nBookmark)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::copyRowValue" );
    connectivity::ORowVector< ORowSetValue >::Vector::iterator aIter = _rKeyRow->get().begin();

    // check the if the parameter values have been changed
    OSL_ENSURE((m_aParameterValueForCache.get().size()-1) == m_pParameterNames->size(),"OptimisticSet::copyRowValue: Parameter values and names differ!");
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
    SelectColumnsMetaData::const_iterator aPosIter = m_pKeyColumnNames->begin();
    SelectColumnsMetaData::const_iterator aPosEnd = m_pKeyColumnNames->end();
    for(;aPosIter != aPosEnd;++aPosIter,++aIter)
    {
        ORowSetValue aValue((_rInsertRow->get())[aPosIter->second.nPosition]);
        switch(aPosIter->second.nType)
        {
            case DataType::DECIMAL:
            case DataType::NUMERIC:
                {
                    ::rtl::OUString sValue = aValue.getString();
                    sal_Int32 nIndex = sValue.indexOf('.');
                    if ( nIndex != -1 )
                    {
                        aValue = sValue.copy(0,nIndex + (aPosIter->second.nScale > 0 ? aPosIter->second.nScale + 1 : 0));
                    }
                }
                break;
            default:
                break;
        }
        *aIter = aValue;
        aIter->setTypeKind(aPosIter->second.nType);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OptimisticSet::deleteRow(const ORowSetRow& _rDeleteRow,const connectivity::OSQLTable& /*_xTable*/   ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString sParam(RTL_CONSTASCII_USTRINGPARAM(" = ?"));
    ::rtl::OUString sIsNull(RTL_CONSTASCII_USTRINGPARAM(" IS NULL"));
    static const ::rtl::OUString s_sAnd(RTL_CONSTASCII_USTRINGPARAM(" AND "));
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    ::rtl::OUString aColumnName;
    ::rtl::OUStringBuffer sKeyCondition,sIndexCondition;
    ::std::vector<sal_Int32> aIndexColumnPositions;
    TSQLStatements aKeyConditions;
    TSQLStatements aIndexConditions;
    TSQLStatements aSql;

    // sal_Int32 i = 1;
    // here we build the condition part for the update statement
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    for(;aIter != aEnd;++aIter)
    {
        if ( m_aJoinedKeyColumns.find(aIter->second.nPosition) == m_aJoinedKeyColumns.end() && m_pKeyColumnNames->find(aIter->first) != m_pKeyColumnNames->end() )
        {
            // only delete rows which aren't the key in the join
            const ::rtl::OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,aIter->second.sRealName);
            lcl_fillKeyCondition(aIter->second.sTableName,sQuotedColumnName,(_rDeleteRow->get())[aIter->second.nPosition],aKeyConditions);
        }
    }
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    TSQLStatements::iterator aSqlIter = aKeyConditions.begin();
    TSQLStatements::iterator aSqlEnd  = aKeyConditions.end();
    for(;aSqlIter != aSqlEnd ; ++aSqlIter)
    {
        ::rtl::OUStringBuffer& rCondition = aSqlIter->second;
        if ( rCondition.getLength() )
        {
            ::rtl::OUStringBuffer sSql;
            sSql.appendAscii("DELETE FROM ");
            ::rtl::OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(xMetaData,aSqlIter->first,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
            sSql.append( ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable ) );
            sSql.appendAscii(" WHERE ");
            sSql.append( rCondition );
            executeDelete(_rDeleteRow,sSql.makeStringAndClear(),aSqlIter->first);
        }
    }
}
// -------------------------------------------------------------------------
void OptimisticSet::executeDelete(const ORowSetRow& _rDeleteRow,const ::rtl::OUString& i_sSQL,const ::rtl::OUString& i_sTableName)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::executeDelete" );

    // now create end execute the prepared statement
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
// -------------------------------------------------------------------------
void SAL_CALL OptimisticSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::cancelRowUpdates" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
}
// -------------------------------------------------------------------------
void SAL_CALL OptimisticSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::moveToInsertRow" );
}
// -------------------------------------------------------------------------
void SAL_CALL OptimisticSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::moveToCurrentRow" );
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::next(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::next" );
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
sal_Bool SAL_CALL OptimisticSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::isBeforeFirst" );
    return m_aKeyIter == m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::isAfterLast" );
    return  m_bRowCountFinal && m_aKeyIter == m_aKeyMap.end();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::isFirst" );
    OKeySetMatrix::iterator aTemp = m_aKeyMap.begin();
    ++aTemp;
    return m_aKeyIter == aTemp && m_aKeyIter != m_aKeyMap.end();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::isLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::isLast" );
    if(!m_bRowCountFinal)
        return sal_False;

    OKeySetMatrix::iterator aTemp = m_aKeyMap.end();
    --aTemp;
    return m_aKeyIter == aTemp;
}
// -----------------------------------------------------------------------------
void SAL_CALL OptimisticSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::beforeFirst" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.begin();
    m_xSet = NULL;
    ::comphelper::disposeComponent(m_xRow);
}
// -----------------------------------------------------------------------------
void SAL_CALL OptimisticSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::afterLast" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();
    m_aKeyIter = m_aKeyMap.end();
    m_xSet = NULL;
    ::comphelper::disposeComponent(m_xRow);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::first(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::first" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.begin();
    ++m_aKeyIter;
    if(m_aKeyIter == m_aKeyMap.end() && !fetchRow())
        m_aKeyIter = m_aKeyMap.end();

    refreshRow();
    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::last(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::last" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();

    m_aKeyIter = m_aKeyMap.end();
    --m_aKeyIter;
    refreshRow();
    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OptimisticSet::getRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getRow" );
    OSL_ENSURE(!isAfterLast(),"getRow is not allowed when afterlast record!");
    OSL_ENSURE(!isBeforeFirst(),"getRow is not allowed when beforefirst record!");

    return ::std::distance(m_aKeyMap.begin(),m_aKeyIter);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::absolute" );
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
sal_Bool SAL_CALL OptimisticSet::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::relative" );
    if(!rows)
    {
        refreshRow();
        return sal_True;
    }
    return absolute(getRow()+rows);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::previous(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::previous" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    if(m_aKeyIter != m_aKeyMap.begin())
    {
        --m_aKeyIter;
        refreshRow();
    }
    return m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
void SAL_CALL OptimisticSet::refreshRow() throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::refreshRow" );
    if(isBeforeFirst() || isAfterLast() || !m_xStatement.is())
        return;

    m_xSet.clear();
    ::comphelper::disposeComponent(m_xRow);
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

    // first put the parameters set by the outer rowset
    for(++aParaIter;aParaIter != aParaEnd;++aParaIter,++nPos)
    {
        ::dbtools::setObjectWithInfo( xParameter, nPos, aParaIter->makeAny(), aParaIter->getTypeKind() );
    }

    // now set the primary key column values
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aIter = m_aKeyIter->second.first->get().begin();
    SelectColumnsMetaData::const_iterator aPosIter = m_pKeyColumnNames->begin();
    SelectColumnsMetaData::const_iterator aPosEnd = m_pKeyColumnNames->end();
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
sal_Bool OptimisticSet::fetchRow()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::fetchRow" );
    // fetch the next row and append on the OptimisticSet
    sal_Bool bRet = sal_False;
    if ( !m_bRowCountFinal )
        bRet = m_xDriverSet->next();
    if ( bRet )
    {
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >(m_pKeyColumnNames->size());
        connectivity::ORowVector< ORowSetValue >::Vector::iterator aIter = aKeyRow->get().begin();
        // first fetch the values needed for the key column
        SelectColumnsMetaData::const_iterator aPosIter = m_pKeyColumnNames->begin();
        SelectColumnsMetaData::const_iterator aPosEnd = m_pKeyColumnNames->end();
        for(;aPosIter != aPosEnd;++aPosIter,++aIter)
        {
            const SelectColumnDescription& rColDesc = aPosIter->second;
            aIter->fill(rColDesc.nPosition,rColDesc.nType,rColDesc.bNullable,m_xDriverRow);
        }
        m_aKeyIter = m_aKeyMap.insert(OKeySetMatrix::value_type(m_aKeyMap.rbegin()->first+1,OKeySetValue(aKeyRow,0))).first;
    }
    else
        m_bRowCountFinal = sal_True;
    return bRet;
}
// -------------------------------------------------------------------------
void OptimisticSet::fillAllRows()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::fillAllRows" );
    if(!m_bRowCountFinal)
    {
        while(fetchRow())
            ;
    }
}
// XRow
sal_Bool SAL_CALL OptimisticSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::wasNull" );
    return m_xRow->wasNull();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OptimisticSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getString" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getString(columnIndex);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getBoolean" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBoolean(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL OptimisticSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getByte" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getByte(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL OptimisticSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getShort" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getShort(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OptimisticSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getInt" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getInt(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL OptimisticSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getLong" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getLong(columnIndex);
}
// -------------------------------------------------------------------------
float SAL_CALL OptimisticSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getFloat" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getFloat(columnIndex);
}
// -------------------------------------------------------------------------
double SAL_CALL OptimisticSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getDouble" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getDouble(columnIndex);
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OptimisticSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getBytes" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBytes(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL OptimisticSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getDate" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getDate(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL OptimisticSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getTime" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getTime(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL OptimisticSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getTimestamp" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getTimestamp(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OptimisticSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getBinaryStream" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBinaryStream(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OptimisticSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getCharacterStream" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getCharacterStream(columnIndex);
}
// -------------------------------------------------------------------------
Any SAL_CALL OptimisticSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getObject" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getObject(columnIndex,typeMap);
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL OptimisticSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getRef" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getRef(columnIndex);
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OptimisticSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getBlob" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBlob(columnIndex);
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL OptimisticSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getClob" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getClob(columnIndex);
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OptimisticSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getArray" );
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getArray(columnIndex);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::rowUpdated" );
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second == 2;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::rowInserted" );
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OptimisticSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::rowDeleted" );
    sal_Bool bDeleted = m_bDeleted;
    m_bDeleted = sal_False;
    return bDeleted;
}
// -----------------------------------------------------------------------------
::rtl::OUString OptimisticSet::getComposedTableName(const ::rtl::OUString& /*_sCatalog*/,
                                              const ::rtl::OUString& /*_sSchema*/,
                                              const ::rtl::OUString& /*_sTable*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OptimisticSet::getComposedTableName" );
    ::rtl::OUString aComposedName;
/*
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
*/
    return aComposedName;
}
// -----------------------------------------------------------------------------
/*
namespace dbaccess
{
    void getColumnPositions(const Reference<XNameAccess>& _rxQueryColumns,
                            const Sequence< ::rtl::OUString>& _aColumnNames,
                            const ::rtl::OUString& _rsUpdateTableName,
                            SelectColumnsMetaData& _rColumnNames)
    {
        // get the real name of the columns
        Sequence< ::rtl::OUString> aSelNames(_rxQueryColumns->getElementNames());
        const ::rtl::OUString* pSelBegin    = aSelNames.getConstArray();
        const ::rtl::OUString* pSelEnd      = pSelBegin + aSelNames.getLength();

        const ::rtl::OUString* pColumnIter  = _aColumnNames.getConstArray();
        const ::rtl::OUString* pColumnEnd   = pColumnIter + _aColumnNames.getLength();

        ::comphelper::UStringMixLess aTmp(_rColumnNames.key_comp());
        ::comphelper::UStringMixEqual bCase(static_cast< ::comphelper::UStringMixLess*>(&aTmp)->isCaseSensitive());

        for(sal_Int32 nPos = 1;pSelBegin != pSelEnd;++pSelBegin,++nPos)
        {
            Reference<XPropertySet> xColumnProp(_rxQueryColumns->getByName(*pSelBegin),UNO_QUERY);
            ::rtl::OUString sRealName,sTableName;
            OSL_ENSURE(xColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
            OSL_ENSURE(xColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");
            xColumnProp->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
            xColumnProp->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;

            for(;pColumnIter != pColumnEnd;++pColumnIter)
            {
                if(bCase(sRealName,*pColumnIter) && bCase(_rsUpdateTableName,sTableName) && _rColumnNames.find(*pColumnIter) == _rColumnNames.end())
                {
                    sal_Int32 nType = 0;
                    xColumnProp->getPropertyValue(PROPERTY_TYPE)    >>= nType;
                    sal_Int32 nScale = 0;
                    xColumnProp->getPropertyValue(PROPERTY_SCALE)   >>= nScale;
                    ::rtl::OUString sColumnDefault;
                    if ( xColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_DEFAULTVALUE) )
                        xColumnProp->getPropertyValue(PROPERTY_DEFAULTVALUE) >>= sColumnDefault;

                    sal_Int32 bNullable = sal_False;
                    xColumnProp->getPropertyValue(PROPERTY_ISNULLABLE)  >>= bNullable;


                    _rColumnNames[sRealName] = SelectColumnDescription( nPos, nType,nScale,bNullable != sdbc::ColumnValue::NO_NULLS, sColumnDefault );
                    break;
                }
            }
            pColumnIter = _aColumnNames.getConstArray();
        }
    }
}
*/
// -----------------------------------------------------------------------------
void OptimisticSet::impl_convertValue_throw(const ORowSetRow& _rInsertRow,const SelectColumnDescription& i_aMetaData)
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
                    aValue = sValue.copy(0,nIndex + (i_aMetaData.nScale > 0 ? i_aMetaData.nScale + 1 : 0));
                }
            }
            break;
        default:
            break;
    }
}
// -----------------------------------------------------------------------------
void OptimisticSet::findTableColumnsMatching_throw(const Any& i_aTable
                                                   ,const Reference<XDatabaseMetaData>& i_xMeta
                                                   ,const Reference<XNameAccess>& i_xQueryColumns)
{
    ::rtl::OUString sCatalog,sSchema,sTable;
    Reference<XPropertySet> xTableProp(i_aTable,UNO_QUERY);
    Any aCatalog = xTableProp->getPropertyValue(PROPERTY_CATALOGNAME);
    aCatalog >>= sCatalog;
    xTableProp->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;
    xTableProp->getPropertyValue(PROPERTY_NAME)         >>= sTable;

    // first ask the database itself for the best columns which can be used
    ::std::vector< ::rtl::OUString> aBestRowColumnNames;
    Reference<XResultSet> xBestRes(i_xMeta->getBestRowIdentifier(aCatalog,sSchema,sTable,0,sal_False));
    Reference<XRow> xBestRow(xBestRes,uno::UNO_QUERY);
    while ( xBestRes->next() )
    {
        aBestRowColumnNames.push_back(xBestRow->getString(2));
    }

    Sequence< ::rtl::OUString> aBestColumnNames;
    if ( aBestRowColumnNames.empty() )
    {
        Reference<XNameAccess> xKeyColumns  = getPrimaryKeyColumns_throw(i_aTable);
        if ( xKeyColumns.is() )
            aBestColumnNames = xKeyColumns->getElementNames();
    }
    else
        aBestColumnNames = Sequence< ::rtl::OUString>(&aBestRowColumnNames[0],aBestRowColumnNames.size());

    const Reference<XColumnsSupplier> xTblColSup(i_aTable,UNO_QUERY_THROW);
    const Reference<XNameAccess> xTblColumns = xTblColSup->getColumns();
    const ::rtl::OUString sComposedUpdateTableName = dbtools::composeTableName( i_xMeta, sCatalog, sSchema, sTable, sal_False, ::dbtools::eInDataManipulation );
    ::dbaccess::getColumnPositions(i_xQueryColumns,aBestColumnNames,sComposedUpdateTableName,(*m_pKeyColumnNames),true);
    ::dbaccess::getColumnPositions(i_xQueryColumns,xTblColumns->getElementNames(),sComposedUpdateTableName,(*m_pColumnNames),true);

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
    ::dbaccess::getColumnPositions(i_xQueryColumns,aParameterColumns,sComposedUpdateTableName,(*m_pParameterNames),true);

    SelectColumnsMetaData::const_iterator aPosIter = m_pKeyColumnNames->begin();
    SelectColumnsMetaData::const_iterator aPosEnd = m_pKeyColumnNames->end();
    for(;aPosIter != aPosEnd;++aPosIter)
    {
        if ( xTblColumns->hasByName(aPosIter->second.sRealName) )
        {
            Reference<XPropertySet> xProp(xTblColumns->getByName(aPosIter->second.sRealName),UNO_QUERY);
            sal_Bool bAuto = sal_False;
            if( (xProp->getPropertyValue(PROPERTY_ISAUTOINCREMENT) >>= bAuto) && bAuto)
                m_aAutoColumns.push_back(aPosIter->first);
        }
    }
}
// -----------------------------------------------------------------------------
void OptimisticSet::executeUpdate(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const ::rtl::OUString& i_sSQL,const ::rtl::OUString& i_sTableName)
{
    // now create end execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(i_sSQL));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    sal_Int32 i = 1;
    // first the set values
    SelectColumnsMetaData::const_iterator aIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aEnd = m_pColumnNames->end();
    sal_uInt16 j = 0;
    for(;aIter != aEnd;++aIter,++j)
    {
        if ( aIter->second.sTableName == i_sTableName )
        {
            sal_Int32 nPos = aIter->second.nPosition;
            if((_rInsertRow->get())[nPos].isModified())
            {
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
        if ( aIter->second.sTableName == i_sTableName )
        {
            setParameter(i++,xParameter,(_rOrginalRow->get())[aIter->second.nPosition],aIter->second.nType,aIter->second.nScale);
        }
    }
/*
    // now we have to set the index values
    ::std::vector<sal_Int32>::iterator aIdxColIter = aIndexColumnPositions.begin();
    ::std::vector<sal_Int32>::iterator aIdxColEnd = aIndexColumnPositions.end();
    j = 0;
    aIter = m_pColumnNames->begin();
    for(;aIdxColIter != aIdxColEnd;++aIdxColIter,++i,++j,++aIter)
    {
        setParameter(i,xParameter,(_rOrginalRow->get())[*aIdxColIter],(_rOrginalRow->get())[*aIdxColIter].getTypeKind(),aIter->second.nScale);
    }
*/
    const sal_Int32 nRowsUpdated = xPrep->executeUpdate();
     m_bUpdated =  nRowsUpdated > 0;
    if(m_bUpdated)
    {
        const sal_Int32 nBookmark = ::comphelper::getINT32((_rInsertRow->get())[0].getAny());
        m_aKeyIter = m_aKeyMap.find(nBookmark);
        m_aKeyIter->second.second = 2;
        copyRowValue(_rInsertRow,m_aKeyIter->second.first,nBookmark);
    }
}
// -----------------------------------------------------------------------------
void OptimisticSet::fillJoinedColumns_throw(const ::std::vector< TNodePair >& i_aJoinColumns)
{
    ::std::vector< TNodePair >::const_iterator aIter = i_aJoinColumns.begin();
    for(;aIter != i_aJoinColumns.end();++aIter)
    {
        ::rtl::OUString sColumnName,sTableName;
        m_aSqlIterator.getColumnRange(aIter->first,sColumnName,sTableName);
        ::rtl::OUStringBuffer sLeft,sRight;
        sLeft.append(sTableName);
        sLeft.appendAscii(".");
        sLeft.append(sColumnName);
        m_aSqlIterator.getColumnRange(aIter->second,sColumnName,sTableName);
        sRight.append(sTableName);
        sRight.appendAscii(".");
        sRight.append(sColumnName);
        fillJoinedColumns_throw(sLeft.makeStringAndClear(),sRight.makeStringAndClear());
    }
}
// -----------------------------------------------------------------------------
void OptimisticSet::fillJoinedColumns_throw(const ::rtl::OUString& i_sLeftColumn,const ::rtl::OUString& i_sRightColumn)
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
// -----------------------------------------------------------------------------
bool OptimisticSet::isResultSetChanged() const
{
    bool bOld = m_bResultSetChanged;
    m_bResultSetChanged = false;
    return bOld;
}
// -----------------------------------------------------------------------------
void OptimisticSet::reset(const Reference< XResultSet>& _xDriverSet)
{
    OCacheSet::construct(_xDriverSet,::rtl::OUString());
    m_bRowCountFinal = sal_False;
    m_aKeyMap.clear();
    m_aKeyMap.insert(OKeySetMatrix::value_type(0,OKeySetValue(NULL,0)));
    m_aKeyIter = m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
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
        ::rtl::OUString m_sTableName;
        TableNameFunctor(const ::rtl::OUString& i_sTableName)
            : m_sTableName(i_sTableName)
        {
        }

        inline bool operator()(const SelectColumnsMetaData::value_type& _aType)
        {
            return m_sTableName == _aType.second.sTableName;
        }
    };
}
// -----------------------------------------------------------------------------
bool OptimisticSet::updateColumnValues(const ORowSetValueVector::Vector& io_aCachedRow,ORowSetValueVector::Vector& io_aRow,const ::std::vector<sal_Int32>& i_aChangedColumns)
{
    bool bRet = false;
    ::std::vector<sal_Int32>::const_iterator aColIdxIter = i_aChangedColumns.begin();
    for(;aColIdxIter != i_aChangedColumns.end();++aColIdxIter)
    {
        SelectColumnsMetaData::const_iterator aFind = ::std::find_if(m_pKeyColumnNames->begin(),m_pKeyColumnNames->end(),PositionFunctor(*aColIdxIter));
        if ( aFind != m_pKeyColumnNames->end() )
        {
            const ::rtl::OUString sTableName = aFind->second.sTableName;
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
                for(sal_Int32 i = 1;aIter != aEnd;++aIter)
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
// -----------------------------------------------------------------------------
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
            const ::rtl::OUString sTableName = aFind->second.sTableName;
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
                for(sal_Int32 i = 1;aIter2 != aEnd2;++aIter2)
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
// -----------------------------------------------------------------------------
void OptimisticSet::fillMissingValues(ORowSetValueVector::Vector& io_aRow) const
{
    TSQLStatements aSql;
    TSQLStatements aKeyConditions;
    ::std::map< ::rtl::OUString,bool > aResultSetChanged;
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    ::rtl::OUString sIsNull(RTL_CONSTASCII_USTRINGPARAM(" IS NULL"));
    ::rtl::OUString sParam(RTL_CONSTASCII_USTRINGPARAM(" = ?"));
    // here we build the condition part for the update statement
    SelectColumnsMetaData::const_iterator aColIter = m_pColumnNames->begin();
    SelectColumnsMetaData::const_iterator aColEnd = m_pColumnNames->end();
    for(;aColIter != aColEnd;++aColIter)
    {
        const ::rtl::OUString sQuotedColumnName = ::dbtools::quoteName( aQuote,aColIter->second.sRealName);
        if ( m_aJoinedKeyColumns.find(aColIter->second.nPosition) != m_aJoinedKeyColumns.end() )
        {
            lcl_fillKeyCondition(aColIter->second.sTableName,sQuotedColumnName,io_aRow[aColIter->second.nPosition],aKeyConditions);
        }
        ::rtl::OUStringBuffer& rPart = aSql[aColIter->second.sTableName];
        if ( rPart.getLength() )
            rPart.appendAscii(", ");
        rPart.append(sQuotedColumnName);
    }
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    TSQLStatements::iterator aSqlIter = aSql.begin();
    TSQLStatements::iterator aSqlEnd  = aSql.end();
    for(;aSqlIter != aSqlEnd ; ++aSqlIter)
    {
        if ( aSqlIter->second.getLength() )
        {
            ::rtl::OUStringBuffer& rCondition = aKeyConditions[aSqlIter->first];
            if ( rCondition.getLength() )
            {
                ::rtl::OUString sCatalog,sSchema,sTable;
                ::dbtools::qualifiedNameComponents(xMetaData,aSqlIter->first,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
                ::rtl::OUString sComposedTableName = ::dbtools::composeTableNameForSelect( m_xConnection, sCatalog, sSchema, sTable );
                ::rtl::OUStringBuffer sQuery;
                sQuery.appendAscii("SELECT ");
                sQuery.append(aSqlIter->second);
                sQuery.appendAscii(" FROM ");
                sQuery.append(sComposedTableName);
                sQuery.appendAscii(" WHERE ");
                sQuery.append(rCondition.makeStringAndClear());

                try
                {
                    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(sQuery.makeStringAndClear()));
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
                                io_aRow[aColIter->second.nPosition].fill(i++,aColIter->second.nType,aColIter->second.bNullable,xRow);
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
// -----------------------------------------------------------------------------

