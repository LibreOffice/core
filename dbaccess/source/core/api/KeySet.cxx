/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: KeySet.cxx,v $
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
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
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
                 const ORowSetValueVector& _aParameterValueForCache)
            :m_aParameterValueForCache(_aParameterValueForCache)
            ,m_pKeyColumnNames(NULL)
            ,m_pColumnNames(NULL)
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
    delete m_pKeyColumnNames;
    delete m_pColumnNames;
    delete m_pForeignColumnNames;

    DBG_DTOR(OKeySet,NULL);
}
// -----------------------------------------------------------------------------
void OKeySet::construct(const Reference< XResultSet>& _xDriverSet)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::construct" );
    OCacheSet::construct(_xDriverSet);

    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    bool bCase = (xMeta.is() && xMeta->storesMixedCaseQuotedIdentifiers()) ? true : false;
    m_pKeyColumnNames = new SelectColumnsMetaData(bCase);
    m_pColumnNames = new SelectColumnsMetaData(bCase);
    m_pForeignColumnNames = new SelectColumnsMetaData(bCase);

    Reference<XNameAccess> xKeyColumns  = getKeyColumns();
    Reference<XColumnsSupplier> xSup(m_xComposer,UNO_QUERY);
    Reference<XNameAccess> xSourceColumns = m_xTable->getColumns();

    ::dbaccess::getColumnPositions(xSup->getColumns(),xKeyColumns,m_sUpdateTableName,(*m_pKeyColumnNames));
    ::dbaccess::getColumnPositions(xSup->getColumns(),xSourceColumns,m_sUpdateTableName,(*m_pColumnNames));

    SelectColumnsMetaData::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
    SelectColumnsMetaData::const_iterator aPosEnd = (*m_pKeyColumnNames).end();
    for(;aPosIter != aPosEnd;++aPosIter)
    {
        if(xSourceColumns->hasByName(aPosIter->first))
        {
            Reference<XPropertySet> xProp(xSourceColumns->getByName(aPosIter->first),UNO_QUERY);
            sal_Bool bAuto = sal_Bool();
            if( (xProp->getPropertyValue(PROPERTY_ISAUTOINCREMENT) >>= bAuto) && bAuto)
                m_aAutoColumns.push_back(aPosIter->first);
        }
    }

    // the first row is empty because it's now easier for us to distinguish when we are beforefirst or first
    // without extra varaible to be set
    m_aKeyMap.insert(OKeySetMatrix::value_type(0,OKeySetValue(NULL,0)));

    m_aKeyIter = m_aKeyMap.begin();

    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    ::rtl::OUString aQuote  = getIdentifierQuoteString();

    ::rtl::OUStringBuffer aFilter;
    ::rtl::OUString sCatalog,sSchema,sTable;

    Reference<XPropertySet> xTableProp(m_xTable,UNO_QUERY);
    xTableProp->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
    xTableProp->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;
    xTableProp->getPropertyValue(PROPERTY_NAME)         >>= sTable;

    m_aSelectComposedTableName = getComposedTableName(sCatalog,sSchema,sTable);

    ::rtl::OUString sComposedName;
    sCatalog = sSchema = sTable = ::rtl::OUString();
    ::dbtools::qualifiedNameComponents(xMetaData,m_sUpdateTableName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
    sComposedName = ::dbtools::composeTableName( xMetaData, sCatalog, sSchema, sTable, sal_True, ::dbtools::eInDataManipulation );

    static ::rtl::OUString s_sDot(RTL_CONSTASCII_USTRINGPARAM("."));
    static ::rtl::OUString s_sParam(RTL_CONSTASCII_USTRINGPARAM(" = ?"));
    // create the where clause
    aPosEnd = (*m_pKeyColumnNames).end();
    for(aPosIter = (*m_pKeyColumnNames).begin();aPosIter != aPosEnd;)
    {
        aFilter.append(sComposedName);
        aFilter.append(s_sDot);
        aFilter.append(::dbtools::quoteName( aQuote,aPosIter->first));
        aFilter.append(s_sParam);
        ++aPosIter;
        if(aPosIter != aPosEnd)
            aFilter.append(aAnd);
    }

    Reference< XMultiServiceFactory >  xFactory(m_xConnection, UNO_QUERY_THROW);
    Reference<XSingleSelectQueryComposer> xAnalyzer(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
    xAnalyzer->setQuery(m_xComposer->getQuery());
    Reference<XTablesSupplier> xTabSup(xAnalyzer,uno::UNO_QUERY);
    Reference<XNameAccess> xSelectTables(xTabSup->getTables(),uno::UNO_QUERY);
    const Sequence< ::rtl::OUString> aSeq = xSelectTables->getElementNames();
    if ( aSeq.getLength() > 1 ) // special handling for join
    {
        const ::rtl::OUString* pIter = aSeq.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if ( *pIter != m_sUpdateTableName )
            {
                connectivity::OSQLTable xSelColSup(xSelectTables->getByName(*pIter),uno::UNO_QUERY);
                Reference<XPropertySet> xProp(xSelColSup,uno::UNO_QUERY);
                ::rtl::OUString sSelectTableName = ::dbtools::composeTableName( xMetaData, xProp, ::dbtools::eInDataManipulation, false, false, false );
                Reference<XNameAccess > xSelectColumns = xSup->getColumns();

                ::dbaccess::getColumnPositions(xSelectColumns,xSelColSup->getColumns(),sSelectTableName,(*m_pForeignColumnNames));

                uno::Sequence< ::rtl::OUString> aSelectColumnNames = xSelectColumns->getElementNames();
                const ::rtl::OUString* pSelectColumnName = aSelectColumnNames.getConstArray();
                const ::rtl::OUString* pSelectColumnEnd   = pSelectColumnName + aSelectColumnNames.getLength();
                for( ; pSelectColumnName != pSelectColumnEnd ; ++pSelectColumnName)
                {
                    // look for columns not in the source columns to use them as filter as well
                    if ( !xSourceColumns->hasByName(*pSelectColumnName) )
                    {
                        aFilter.append(s_sDot);
                        aFilter.append(::dbtools::quoteName( aQuote,*pSelectColumnName));
                        aFilter.append(s_sParam);
                        if ( (pSelectColumnName+1) != pSelectColumnEnd )
                            aFilter.append(aAnd);
                    }
                }
                break;
            }
        }
    } // if ( aSeq.getLength() > 1 ) // special handling for join
    const ::rtl::OUString sOldFilter = xAnalyzer->getFilter();
    if ( sOldFilter.getLength() )
    {
        aFilter = sOldFilter + aAnd + aFilter;
    }
    xAnalyzer->setFilter(aFilter.makeStringAndClear());
    m_xStatement = m_xConnection->prepareStatement(xAnalyzer->getQueryWithSubstitution());
    ::comphelper::disposeComponent(xAnalyzer);
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
        aCondition.append(::dbtools::quoteName( aQuote,aIter->first));
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
        if(xKeyColumns.is() && xKeyColumns->hasByName(aIter->first))
        {
            sKeyCondition.append(::dbtools::quoteName( aQuote,aIter->first));
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
                    sIndexCondition.append(::dbtools::quoteName( aQuote,aIter->first));
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
            aSql.append(::dbtools::quoteName( aQuote,aIter->first));
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
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql.makeStringAndClear()));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    i = 1;
    // first the set values
    aIter = m_pColumnNames->begin();
    sal_uInt16 j = 0;
    for(;aIter != aEnd;++aIter,++j)
    {
        sal_Int32 nPos = aIter->second.nPosition;
        if((_rInsertRow->get())[nPos].isModified())
        {
            (_rInsertRow->get())[nPos].setSigned((_rOrginalRow->get())[nPos].isSigned());
            setParameter(i++,xParameter,(_rInsertRow->get())[nPos],aIter->second.nType,aIter->second.nScale);
        }
    }
    // and then the values of the where condition
    aIter = (*m_pKeyColumnNames).begin();
    aEnd = (*m_pKeyColumnNames).end();
    j = 0;
    for(;aIter != aEnd;++aIter,++i,++j)
    {
        setParameter(i,xParameter,(_rOrginalRow->get())[aIter->second.nPosition],aIter->second.nType,aIter->second.nScale);
    }

    // now we have to set the index values
    ::std::vector<sal_Int32>::iterator aIdxColIter = aIndexColumnPositions.begin();
    ::std::vector<sal_Int32>::iterator aIdxColEnd = aIndexColumnPositions.end();
    j = 0;
    for(;aIdxColIter != aIdxColEnd;++aIdxColIter,++i,++j)
    {
        setParameter(i,xParameter,(_rOrginalRow->get())[*aIdxColIter],(_rOrginalRow->get())[*aIdxColIter].getTypeKind(),aIter->second.nScale);
    }

     m_bUpdated = xPrep->executeUpdate() > 0;


    if(m_bUpdated)
    {
        m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32((_rInsertRow->get())[0].getAny()));
        OSL_ENSURE(m_aKeyIter != m_aKeyMap.end(),"New inserted row not found!");
        m_aKeyIter->second.second = 2;
        copyRowValue(_rInsertRow,m_aKeyIter->second.first);
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
    sal_Bool bModified = sal_False;
    for(;aIter != aEnd;++aIter,++j)
    {
        if((_rInsertRow->get())[aIter->second.nPosition].isModified())
        {
            aSql.append(::dbtools::quoteName( aQuote,aIter->first));
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
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql.makeStringAndClear()));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    aIter = m_pColumnNames->begin();
    for(sal_Int32 i = 1;aIter != aEnd;++aIter)
    {
        const sal_Int32 nPos = aIter->second.nPosition;
        if((_rInsertRow->get())[nPos].isModified())
        {
            if((_rInsertRow->get())[nPos].isNull())
                xParameter->setNull(i++,(_rInsertRow->get())[nPos].getTypeKind());
            else
            {
                (_rInsertRow->get())[nPos].setSigned(m_aSignedFlags[nPos-1]);
                setParameter(i++,xParameter,(_rInsertRow->get())[nPos],aIter->second.nType,aIter->second.nScale);
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
    if ( m_bInserted )
    {
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >((*m_pKeyColumnNames).size());
        copyRowValue(_rInsertRow,aKeyRow);

        OKeySetMatrix::iterator aKeyIter = m_aKeyMap.end();
        --aKeyIter;
        m_aKeyIter = m_aKeyMap.insert(OKeySetMatrix::value_type(aKeyIter->first + 1,OKeySetValue(aKeyRow,1))).first;
        // now we set the bookmark for this row
        (_rInsertRow->get())[0] = makeAny((sal_Int32)m_aKeyIter->first);
    }
}
// -----------------------------------------------------------------------------
void OKeySet::copyRowValue(const ORowSetRow& _rInsertRow,ORowSetRow& _rKeyRow)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::copyRowValue" );
    connectivity::ORowVector< ORowSetValue >::Vector::iterator aIter = _rKeyRow->get().begin();
    SelectColumnsMetaData::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
    SelectColumnsMetaData::const_iterator aPosEnd = (*m_pKeyColumnNames).end();
    for(;aPosIter != aPosEnd;++aPosIter,++aIter)
    {
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
        if(xKeyColumns.is() && xKeyColumns->hasByName(aIter->first))
        {
            aSql.append(::dbtools::quoteName( aQuote,aIter->first));
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
                    sIndexCondition.append(::dbtools::quoteName( aQuote,aIter->first));
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
    for(;aIdxColIter != aIdxColEnd;++aIdxColIter,++i)
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
        Reference<XKeysSupplier> xKeySup(m_xTable,UNO_QUERY);
        if(xKeySup.is())
            xKeys = xKeySup->getKeys();
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
    m_xSet = NULL;
    ::comphelper::disposeComponent(m_xRow);
}
// -----------------------------------------------------------------------------
void SAL_CALL OKeySet::afterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::afterLast" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();
    m_aKeyIter = m_aKeyMap.end();
    m_xSet = NULL;
    ::comphelper::disposeComponent(m_xRow);
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

    m_xSet = NULL;
    ::comphelper::disposeComponent(m_xRow);
    // we just areassign the base members
    Reference< XParameters > xParameter(m_xStatement,UNO_QUERY);
    OSL_ENSURE(xParameter.is(),"No Parameter interface!");
    xParameter->clearParameters();
    sal_Int32 nPos=1;
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aParaIter = m_aParameterValueForCache.get().begin();
    connectivity::ORowVector< ORowSetValue >::Vector::const_iterator aParaEnd = m_aParameterValueForCache.get().end();
    for(++aParaIter;aParaIter != aParaEnd;++aParaIter,++nPos)
    {
        ::dbtools::setObjectWithInfo( xParameter, nPos, aParaIter->makeAny(), aParaIter->getTypeKind() );
    }
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
    sal_Bool bOK = m_xSet->next(); (void)bOK;
    OSL_ENSURE(bOK,"No rows!");
    m_xRow.set(m_xSet,UNO_QUERY);
    OSL_ENSURE(m_xRow.is(),"No row form statement!");
}
// -----------------------------------------------------------------------------
sal_Bool OKeySet::fetchRow()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::fetchRow" );
    // fetch the next row and append on the keyset
    sal_Bool bRet = sal_False;
    if ( !m_bRowCountFinal )
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
        m_aKeyIter = m_aKeyMap.insert(OKeySetMatrix::value_type(m_aKeyMap.rbegin()->first+1,OKeySetValue(aKeyRow,0))).first;
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
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second == 2;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OKeySet::rowInserted" );
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second == 1;
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
                            const Reference<XNameAccess>& _rxColumns,
                            const ::rtl::OUString& _rsUpdateTableName,
                            SelectColumnsMetaData& _rColumnNames)
    {
        // get the real name of the columns
        Sequence< ::rtl::OUString> aSelNames(_rxQueryColumns->getElementNames());
        const ::rtl::OUString* pSelBegin    = aSelNames.getConstArray();
        const ::rtl::OUString* pSelEnd      = pSelBegin + aSelNames.getLength();

        Sequence< ::rtl::OUString> aColumnNames(_rxColumns->getElementNames());
        const ::rtl::OUString* pColumnIter  = aColumnNames.getConstArray();
        const ::rtl::OUString* pColumnEnd   = pColumnIter + aColumnNames.getLength();

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
            pColumnIter = aColumnNames.getConstArray();
        }
    }
}
