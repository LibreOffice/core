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

#include "BookmarkSet.hxx"
#include "CRowSetColumn.hxx"
#include "CRowSetDataColumn.hxx"
#include "KeySet.hxx"
#include "OptimisticSet.hxx"
#include "RowSetBase.hxx"
#include "RowSetCache.hxx"
#include "StaticSet.hxx"
#include "WrappedResultSet.hxx"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include "dbastrings.hrc"

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>

#include <comphelper/extract.hxx>
#include <comphelper/property.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlnode.hxx>
#include <connectivity/sqlparse.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>

#include <algorithm>

using namespace dbaccess;
using namespace dbtools;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;

#define CHECK_MATRIX_POS(M) OSL_ENSURE(((M) >= static_cast<ORowSetMatrix::difference_type>(0)) && ((M) < static_cast<sal_Int32>(m_pMatrix->size())),"Position is invalid!")










ORowSetCache::ORowSetCache(const Reference< XResultSet >& _xRs,
                           const Reference< XSingleSelectQueryAnalyzer >& _xAnalyzer,
                           const Reference<XComponentContext>& _rContext,
                           const OUString& _rUpdateTableName,
                           sal_Bool&    _bModified,
                           sal_Bool&    _bNew,
                           const ORowSetValueVector& _aParameterValueForCache,
                           const OUString& i_sRowSetFilter,
                           sal_Int32 i_nMaxRows)
    :m_xSet(_xRs)
    ,m_xMetaData(Reference< XResultSetMetaDataSupplier >(_xRs,UNO_QUERY)->getMetaData())
    ,m_aContext( _rContext )
    ,m_pCacheSet(NULL)
    ,m_pMatrix(NULL)
    ,m_pInsertMatrix(NULL)
    ,m_nLastColumnIndex(0)
    ,m_nFetchSize(0)
    ,m_nRowCount(0)
    ,m_nPrivileges( Privilege::SELECT )
    ,m_nPosition(0)
    ,m_nStartPos(0)
    ,m_nEndPos(0)
    ,m_bRowCountFinal(sal_False)
    ,m_bBeforeFirst(sal_True)
    ,m_bAfterLast( sal_False )
    ,m_bUpdated(sal_False)
    ,m_bModified(_bModified)
    ,m_bNew(_bNew)
{

    
    Reference< XPropertySet> xProp(_xRs,UNO_QUERY);
    Reference< XPropertySetInfo > xPropInfo = xProp->getPropertySetInfo();
    sal_Bool bBookmarkable = sal_False;
    try
    {
        Reference< XResultSetUpdate> xUp(_xRs,UNO_QUERY_THROW);
        bBookmarkable = xPropInfo->hasPropertyByName(PROPERTY_ISBOOKMARKABLE) &&
                                any2bool(xProp->getPropertyValue(PROPERTY_ISBOOKMARKABLE)) && Reference< XRowLocate >(_xRs, UNO_QUERY).is();
        if ( bBookmarkable )
        {
            xUp->moveToInsertRow();
            xUp->cancelRowUpdates();
            _xRs->beforeFirst();
            m_nPrivileges = Privilege::SELECT|Privilege::DELETE|Privilege::INSERT|Privilege::UPDATE;
            m_pCacheSet = new WrappedResultSet(i_nMaxRows);
            m_xCacheSet = m_pCacheSet;
            m_pCacheSet->construct(_xRs,i_sRowSetFilter);
            return;
        }
    }
    catch(const Exception& ex)
    {
        (void)ex;
    }
    try
    {
        if ( xPropInfo->hasPropertyByName(PROPERTY_RESULTSETTYPE) &&
                            ::comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETTYPE)) != ResultSetType::FORWARD_ONLY)
            _xRs->beforeFirst();
    }
    catch(const SQLException& e)
    {
        (void)e;
    }

    
    sal_Bool bAllKeysFound = sal_False;
    sal_Int32 nTablesCount = 0;

    sal_Bool bNeedKeySet = !bBookmarkable || (xPropInfo->hasPropertyByName(PROPERTY_RESULTSETCONCURRENCY) &&
                            ::comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETCONCURRENCY)) == ResultSetConcurrency::READ_ONLY);

    Reference< XIndexAccess> xUpdateTableKeys;
    OUString aUpdateTableName = _rUpdateTableName;
    Reference< XConnection> xConnection;
    
    Reference< XStatement> xStmt(_xRs->getStatement(),UNO_QUERY);
    if(xStmt.is())
        xConnection = xStmt->getConnection();
    else
    {
        Reference< XPreparedStatement> xPrepStmt(_xRs->getStatement(),UNO_QUERY);
        xConnection = xPrepStmt->getConnection();
    }
    OSL_ENSURE(xConnection.is(),"No connection!");
    if(_xAnalyzer.is())
    {
        try
        {
            Reference<XTablesSupplier> xTabSup(_xAnalyzer,UNO_QUERY);
            OSL_ENSURE(xTabSup.is(),"ORowSet::execute composer isn't a tablesupplier!");
            Reference<XNameAccess> xTables = xTabSup->getTables();
            Sequence< OUString> aTableNames = xTables->getElementNames();
            if ( aTableNames.getLength() > 1 && _rUpdateTableName.isEmpty() && bNeedKeySet )
            {
                m_nPrivileges = Privilege::SELECT|Privilege::DELETE|Privilege::INSERT|Privilege::UPDATE;
                OptimisticSet* pCursor = new OptimisticSet(m_aContext,xConnection,_xAnalyzer,_aParameterValueForCache,i_nMaxRows,m_nRowCount);
                m_pCacheSet = pCursor;
                m_xCacheSet = m_pCacheSet;
                try
                {
                    m_pCacheSet->construct(_xRs,i_sRowSetFilter);
                    if ( pCursor->isReadOnly() )
                        m_nPrivileges = Privilege::SELECT;
                    m_aKeyColumns = pCursor->getJoinedKeyColumns();
                    return;
                }
                catch(const Exception&)
                {
                }
                m_pCacheSet = NULL;
                m_xCacheSet.clear();
            }
            else
            {
                if(!_rUpdateTableName.isEmpty() && xTables->hasByName(_rUpdateTableName))
                    xTables->getByName(_rUpdateTableName) >>= m_aUpdateTable;
                else if(xTables->getElementNames().getLength())
                {
                    aUpdateTableName = xTables->getElementNames()[0];
                    xTables->getByName(aUpdateTableName) >>= m_aUpdateTable;
                }
                Reference<XIndexAccess> xIndexAccess(xTables,UNO_QUERY);
                if(xIndexAccess.is())
                    nTablesCount = xIndexAccess->getCount();
                else
                    nTablesCount = xTables->getElementNames().getLength();

                if(m_aUpdateTable.is() && nTablesCount < 3) 
                {
                    Reference<XPropertySet> xSet(m_aUpdateTable,UNO_QUERY);
                    const Reference<XNameAccess> xPrimaryKeyColumns = dbtools::getPrimaryKeyColumns_throw(xSet);
                    if ( xPrimaryKeyColumns.is() )
                    {
                        Reference<XColumnsSupplier> xColSup(_xAnalyzer,UNO_QUERY);
                        if ( xColSup.is() )
                        {
                            Reference<XNameAccess> xSelColumns = xColSup->getColumns();
                            Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
                            SelectColumnsMetaData aColumnNames(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers() ? true : false);
                            ::dbaccess::getColumnPositions(xSelColumns,xPrimaryKeyColumns->getElementNames(),aUpdateTableName,aColumnNames);
                            bAllKeysFound = !aColumnNames.empty() && sal_Int32(aColumnNames.size()) == xPrimaryKeyColumns->getElementNames().getLength();
                        }
                    }
                }
            }
        }
        catch(Exception&)
        {
        }
    }

    
    if(!bNeedKeySet)
    {
        try
        {
            m_pCacheSet = new OBookmarkSet(i_nMaxRows);
            m_xCacheSet = m_pCacheSet;
            m_pCacheSet->construct(_xRs,i_sRowSetFilter);

            
            m_nPrivileges = Privilege::SELECT;
            if(Reference<XResultSetUpdate>(_xRs,UNO_QUERY).is())  
            {
                Reference<XPropertySet> xTable(m_aUpdateTable,UNO_QUERY);
                if(xTable.is() && xTable->getPropertySetInfo()->hasPropertyByName(PROPERTY_PRIVILEGES))
                {
                    m_nPrivileges = 0;
                    xTable->getPropertyValue(PROPERTY_PRIVILEGES) >>= m_nPrivileges;
                    if(!m_nPrivileges)
                        m_nPrivileges = Privilege::SELECT;
                }
            }
        }
        catch(const SQLException&)
        {
            bNeedKeySet = sal_True;
        }

    }
    if(bNeedKeySet)
    {
        
        bAllKeysFound = bAllKeysFound && (nTablesCount == 1 || checkJoin(xConnection,_xAnalyzer,aUpdateTableName));

        if(!bAllKeysFound )
        {
            if ( bBookmarkable )
            {
                
                _xRs->beforeFirst();
                m_nPrivileges = Privilege::SELECT;
                m_pCacheSet = new WrappedResultSet(i_nMaxRows);
                m_xCacheSet = m_pCacheSet;
                m_pCacheSet->construct(_xRs,i_sRowSetFilter);
                return;
            }
            m_pCacheSet = new OStaticSet(i_nMaxRows);
            m_xCacheSet = m_pCacheSet;
            m_pCacheSet->construct(_xRs,i_sRowSetFilter);
            m_nPrivileges = Privilege::SELECT;
        }
        else
        {
            Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
            SelectColumnsMetaData aColumnNames(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers() ? true : false);
            Reference<XColumnsSupplier> xColSup(_xAnalyzer,UNO_QUERY);
            Reference<XNameAccess> xSelColumns  = xColSup->getColumns();
            Reference<XNameAccess> xColumns     = m_aUpdateTable->getColumns();
            ::dbaccess::getColumnPositions(xSelColumns,xColumns->getElementNames(),aUpdateTableName,aColumnNames);

            
            m_nPrivileges = Privilege::SELECT;
            sal_Bool bNoInsert = sal_False;

            Sequence< OUString> aNames(xColumns->getElementNames());
            const OUString* pIter    = aNames.getConstArray();
            const OUString* pEnd     = pIter + aNames.getLength();
            for(;pIter != pEnd;++pIter)
            {
                Reference<XPropertySet> xColumn(xColumns->getByName(*pIter),UNO_QUERY);
                OSL_ENSURE(xColumn.is(),"Column in table is null!");
                if(xColumn.is())
                {
                    sal_Int32 nNullable = 0;
                    xColumn->getPropertyValue(PROPERTY_ISNULLABLE) >>= nNullable;
                    if(nNullable == ColumnValue::NO_NULLS && aColumnNames.find(*pIter) == aColumnNames.end())
                    { 
                        bNoInsert = sal_True;
                        break; 
                    }
                }
            }

            OKeySet* pKeySet = new OKeySet(m_aUpdateTable,xUpdateTableKeys,aUpdateTableName ,_xAnalyzer,_aParameterValueForCache,i_nMaxRows,m_nRowCount);
            try
            {
                m_pCacheSet = pKeySet;
                m_xCacheSet = m_pCacheSet;
                pKeySet->construct(_xRs,i_sRowSetFilter);

                if(Reference<XResultSetUpdate>(_xRs,UNO_QUERY).is())  
                {
                    Reference<XPropertySet> xTable(m_aUpdateTable,UNO_QUERY);
                    if(xTable.is() && xTable->getPropertySetInfo()->hasPropertyByName(PROPERTY_PRIVILEGES))
                    {
                        m_nPrivileges = 0;
                        xTable->getPropertyValue(PROPERTY_PRIVILEGES) >>= m_nPrivileges;
                        if(!m_nPrivileges)
                            m_nPrivileges = Privilege::SELECT;
                    }
                }
                if(bNoInsert)
                    m_nPrivileges |= ~Privilege::INSERT; 
            }
            catch(const SQLException&)
            {
                
                if ( m_pCacheSet )
                    m_pCacheSet = NULL;
                m_xCacheSet = NULL;
                m_pCacheSet = new OStaticSet(i_nMaxRows);
                m_xCacheSet = m_pCacheSet;
                m_pCacheSet->construct(_xRs,i_sRowSetFilter);
                m_nPrivileges = Privilege::SELECT;
            }
        }

    }
    
    if(!bAllKeysFound && xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_RESULTSETCONCURRENCY) &&
        ::comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETCONCURRENCY)) == ResultSetConcurrency::READ_ONLY)
        m_nPrivileges = Privilege::SELECT;
}

ORowSetCache::~ORowSetCache()
{
    m_pCacheSet = NULL;
    m_xCacheSet = NULL;
    if(m_pMatrix)
    {
        m_pMatrix->clear();
        delete m_pMatrix;
    }

    if(m_pInsertMatrix)
    {
        m_pInsertMatrix->clear();
        delete m_pInsertMatrix;
    }
    m_xSet          = WeakReference< XResultSet>();
    m_xMetaData     = NULL;
    m_aUpdateTable  = NULL;

}

void ORowSetCache::setFetchSize(sal_Int32 _nSize)
{
    if(_nSize == m_nFetchSize)
        return;

    m_nFetchSize = _nSize;
    if(!m_pMatrix)
    {
        m_pMatrix = new ORowSetMatrix(_nSize);
        m_aMatrixIter = m_pMatrix->end();
        m_aMatrixEnd = m_pMatrix->end();

        m_pInsertMatrix = new ORowSetMatrix(1); 
        m_aInsertRow    = m_pInsertMatrix->end();
    }
    else
    {
        
        ::std::vector<sal_Int32> aPositions;
        ::std::map<sal_Int32,sal_Bool> aCacheIterToChange;
        
        ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
        ORowSetCacheMap::iterator aCacheEnd = m_aCacheIterators.end();
        for(;aCacheIter != aCacheEnd;++aCacheIter)
        {
            aCacheIterToChange[aCacheIter->first] = sal_False;
            if ( !aCacheIter->second.pRowSet->isInsertRow()
                /*&& aCacheIter->second.aIterator != m_pMatrix->end()*/ && !m_bModified )
            {
                ptrdiff_t nDist = (aCacheIter->second.aIterator - m_pMatrix->begin());
                aPositions.push_back(nDist);
                aCacheIterToChange[aCacheIter->first] = sal_True;
            }
        }
        sal_Int32 nKeyPos = (m_aMatrixIter - m_pMatrix->begin());
        m_pMatrix->resize(_nSize);

        if ( nKeyPos < _nSize )
            m_aMatrixIter = m_pMatrix->begin() + nKeyPos;
        else
            m_aMatrixIter = m_pMatrix->end();
        m_aMatrixEnd = m_pMatrix->end();

        
        ::std::vector<sal_Int32>::const_iterator aIter = aPositions.begin();
        ::std::map<sal_Int32,sal_Bool>::const_iterator aPosChangeIter = aCacheIterToChange.begin();
        for(    aCacheIter = m_aCacheIterators.begin();
                aPosChangeIter != aCacheIterToChange.end();
                ++aPosChangeIter,++aCacheIter)
        {
            if ( aPosChangeIter->second )
            {
                CHECK_MATRIX_POS(*aIter);
                if ( *aIter < _nSize )
                    aCacheIter->second.aIterator = m_pMatrix->begin() + *aIter++;
                else
                    aCacheIter->second.aIterator = m_pMatrix->end();
            }
        }
    }
    if(!m_nPosition)
    {
        sal_Int32 nNewSt = 0;
        fillMatrix(nNewSt,_nSize);
        OSL_ENSURE(nNewSt == 0, "fillMatrix set new start to unexpected value");
        m_nStartPos = 0;
        m_nEndPos = _nSize;
    }
    else if (m_nStartPos < m_nPosition && m_nPosition <= m_nEndPos)
    {
        sal_Int32 nNewSt = -1;
        _nSize += m_nStartPos;
        fillMatrix(nNewSt, _nSize);
        if (nNewSt >= 0)
        {
            m_nStartPos = nNewSt;
            m_nEndPos =  _nSize;
            m_aMatrixIter = calcPosition();
        }
        else
        {
            m_nEndPos = m_nStartPos + m_nFetchSize;
        }
    }
    else
    {
        OSL_FAIL("m_nPosition not between m_nStartPos and m_nEndpos");
        
        moveWindow();
        m_aMatrixIter = calcPosition();
    }
}


Reference< XResultSetMetaData > ORowSetCache::getMetaData(  )
{
    return m_xMetaData;
}

static Any lcl_getBookmark(ORowSetValue& i_aValue,OCacheSet* i_pCacheSet)
{
    switch ( i_aValue.getTypeKind() )
    {
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
            return makeAny((sal_Int32)i_aValue);
        default:
            if ( i_pCacheSet && i_aValue.isNull())
                i_aValue = i_pCacheSet->getBookmark();
            return i_aValue.getAny();
    }
}


Any ORowSetCache::getBookmark(  )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    if ( m_aMatrixIter >= m_pMatrix->end() || m_aMatrixIter < m_pMatrix->begin() || !(*m_aMatrixIter).is())
    {
        return Any(); 
    }

    return lcl_getBookmark(((*m_aMatrixIter)->get())[0],m_pCacheSet);
}

sal_Bool ORowSetCache::moveToBookmark( const Any& bookmark )
{
    if ( m_pCacheSet->moveToBookmark(bookmark) )
    {
        m_bBeforeFirst = sal_False;
        m_nPosition = m_pCacheSet->getRow();

        checkPositionFlags();

        if(!m_bAfterLast)
        {
            moveWindow();
            checkPositionFlags();
            if ( !m_bAfterLast )
            {
                m_aMatrixIter = calcPosition();
                OSL_ENSURE(m_aMatrixIter->is(),"Iterator after moveToBookmark not valid");
            }
            else
                m_aMatrixIter = m_pMatrix->end();
        }
        else
            m_aMatrixIter = m_pMatrix->end();
    }
    else
        return sal_False;

    return m_aMatrixIter != m_pMatrix->end() && (*m_aMatrixIter).is();
}

sal_Bool ORowSetCache::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows )
{
    sal_Bool bRet( moveToBookmark( bookmark ) );
    if ( bRet )
    {
        m_nPosition = m_pCacheSet->getRow() + rows;
        absolute(m_nPosition);

        bRet = m_aMatrixIter != m_pMatrix->end() && (*m_aMatrixIter).is();
    }

    return bRet;
}

sal_Int32 ORowSetCache::compareBookmarks( const Any& _first, const Any& _second )
{
    return (!_first.hasValue() || !_second.hasValue()) ? CompareBookmark::NOT_COMPARABLE : m_pCacheSet->compareBookmarks(_first,_second);
}

sal_Bool ORowSetCache::hasOrderedBookmarks(  )
{
    return m_pCacheSet->hasOrderedBookmarks();
}

sal_Int32 ORowSetCache::hashBookmark( const Any& bookmark )
{
    return m_pCacheSet->hashBookmark(bookmark);
}


void ORowSetCache::updateNull(sal_Int32 columnIndex,ORowSetValueVector::Vector& io_aRow
                              ,::std::vector<sal_Int32>& o_ChangedColumns
                              )
{
    checkUpdateConditions(columnIndex);

    ORowSetValueVector::Vector& rInsert = ((*m_aInsertRow)->get());
    if ( !rInsert[columnIndex].isNull() )
    {
        rInsert[columnIndex].setBound(true);
        rInsert[columnIndex].setNull();
        rInsert[columnIndex].setModified();
        io_aRow[columnIndex].setNull();

        m_pCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}

void ORowSetCache::updateValue(sal_Int32 columnIndex,const ORowSetValue& x
                               ,ORowSetValueVector::Vector& io_aRow
                               ,::std::vector<sal_Int32>& o_ChangedColumns
                               )
{
    checkUpdateConditions(columnIndex);

    ORowSetValueVector::Vector& rInsert = ((*m_aInsertRow)->get());
    if ( rInsert[columnIndex] != x )
    {
        rInsert[columnIndex].setBound(true);
        rInsert[columnIndex] = x;
        rInsert[columnIndex].setModified();
        io_aRow[columnIndex] = rInsert[columnIndex];

        m_pCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}

void ORowSetCache::updateCharacterStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x
                                         , sal_Int32 length,ORowSetValueVector::Vector& io_aRow
                                         ,::std::vector<sal_Int32>& o_ChangedColumns
                                         )
{
    checkUpdateConditions(columnIndex);

    Sequence<sal_Int8> aSeq;
    if(x.is())
        x->readBytes(aSeq,length);

    ORowSetValueVector::Vector& rInsert = ((*m_aInsertRow)->get());
    rInsert[columnIndex].setBound(true);
    rInsert[columnIndex] = aSeq;
    rInsert[columnIndex].setModified();
    io_aRow[columnIndex] = makeAny(x);

    m_pCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
    impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
}

void ORowSetCache::updateObject( sal_Int32 columnIndex, const Any& x
                                ,ORowSetValueVector::Vector& io_aRow
                                ,::std::vector<sal_Int32>& o_ChangedColumns
                                )
{
    checkUpdateConditions(columnIndex);

    ORowSetValueVector::Vector& rInsert = ((*m_aInsertRow)->get());
    ORowSetValue aTemp;
    aTemp.fill(x);
    if ( rInsert[columnIndex] != aTemp )
    {
        rInsert[columnIndex].setBound(true);
        rInsert[columnIndex] = aTemp;
        rInsert[columnIndex].setModified();
        io_aRow[columnIndex] = rInsert[columnIndex];

        m_pCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}

void ORowSetCache::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 /*scale*/
                                       ,ORowSetValueVector::Vector& io_aRow
                                       ,::std::vector<sal_Int32>& o_ChangedColumns
                                       )
{
    checkUpdateConditions(columnIndex);

    ORowSetValueVector::Vector& rInsert = ((*m_aInsertRow)->get());
    ORowSetValue aTemp;
    aTemp.fill(x);
    if ( rInsert[columnIndex] != aTemp )
    {
        rInsert[columnIndex].setBound(true);
        rInsert[columnIndex] = aTemp;
        rInsert[columnIndex].setModified();
        io_aRow[columnIndex] = rInsert[columnIndex];

        m_pCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}


sal_Bool ORowSetCache::next(  )
{
    if(!isAfterLast())
    {
        m_bBeforeFirst = sal_False;
        ++m_nPosition;

        
        checkPositionFlags();
        if(!m_bAfterLast)
        {
            moveWindow();

            OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
            m_aMatrixIter = calcPosition();
            checkPositionFlags();
        }
    }

    return !m_bAfterLast;
}

sal_Bool ORowSetCache::isBeforeFirst(  )
{
    return m_bBeforeFirst;
}

sal_Bool ORowSetCache::isAfterLast(  )
{
    return m_bAfterLast;
}

sal_Bool ORowSetCache::isFirst(  )
{
    return m_nPosition == 1; 
}

sal_Bool ORowSetCache::isLast(  )
{
    return m_nPosition == m_nRowCount;
}

sal_Bool ORowSetCache::beforeFirst(  )
{
    if(!m_bBeforeFirst)
    {
        m_bAfterLast    = sal_False;
        m_nPosition     = 0;
        m_bBeforeFirst  = sal_True;
        m_pCacheSet->beforeFirst();
        moveWindow();
        m_aMatrixIter = m_pMatrix->end();
    }
    return sal_True;
}

sal_Bool ORowSetCache::afterLast(  )
{
    if(!m_bAfterLast)
    {
        m_bBeforeFirst = sal_False;
        m_bAfterLast = sal_True;

        if(!m_bRowCountFinal)
        {
            m_pCacheSet->last_checked(sal_False);
            m_bRowCountFinal = sal_True;
            m_nRowCount = m_pCacheSet->getRow();
        }
        m_pCacheSet->afterLast();

        m_nPosition = 0;
        m_aMatrixIter = m_pMatrix->end();
    }
    return sal_True;
}

sal_Bool ORowSetCache::fillMatrix(sal_Int32& _nNewStartPos, sal_Int32 &_nNewEndPos)
{
    OSL_ENSURE(_nNewStartPos != _nNewEndPos,"ORowSetCache::fillMatrix: StartPos and EndPos can not be equal!");
    
    
    
    OSL_ENSURE( _nNewStartPos >= -1, "ORowSetCache::fillMatrix: invalid _nNewStartPos" );

    ORowSetMatrix::iterator aIter;
    sal_Int32 i;
    sal_Bool bCheck;
    sal_Int32 requestedStartPos;
    if ( _nNewStartPos == -1 )
    {
        aIter = m_pMatrix->begin() + (m_nEndPos - m_nStartPos);
        i = m_nEndPos + 1;
        requestedStartPos = m_nStartPos;
    }
    else
    {
        aIter = m_pMatrix->begin();
        i = _nNewStartPos + 1;
        requestedStartPos = _nNewStartPos;
    }
    bCheck = m_pCacheSet->absolute(i);


    for(; i <= _nNewEndPos; ++i,++aIter)
    {
        if(bCheck)
        {
            if(!aIter->is())
                *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
            m_pCacheSet->fillValueRow(*aIter,i);
        }
        else
        {   

            if(!m_bRowCountFinal)
            {
                if(m_pCacheSet->previous_checked(sal_False)) 
                    m_nRowCount = m_pCacheSet->getRow(); 
                if(!m_nRowCount)
                    m_nRowCount = i-1; 
                m_bRowCountFinal = sal_True;
            }
            const ORowSetMatrix::iterator aEnd = aIter;
            ORowSetMatrix::iterator aRealEnd = m_pMatrix->end();
            sal_Int32 nPos;
            if (m_nRowCount >= m_nFetchSize)
            {
                nPos = m_nRowCount - m_nFetchSize;
            }
            else
            {
                nPos = 0;
            }
            _nNewStartPos = nPos;
            _nNewEndPos = m_nRowCount;
            ++nPos;
            bCheck = m_pCacheSet->absolute(nPos);

            for(;bCheck && nPos <= requestedStartPos && aIter != aRealEnd; ++aIter, ++nPos)
            {
                if(!aIter->is())
                    *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                m_pCacheSet->fillValueRow(*aIter, nPos);
                bCheck = m_pCacheSet->next();
            }
            if(aIter != aEnd)
                ::std::rotate(m_pMatrix->begin(),aEnd,aIter);
            break;
        }
        bCheck = m_pCacheSet->next();
    }
    
    
    if(!m_bRowCountFinal)
    {
        if(!m_pCacheSet->next())
        {
            if(m_pCacheSet->previous_checked(sal_False)) 
                m_nRowCount = m_pCacheSet->getRow(); 
            m_bRowCountFinal = sal_True;
        }
        else
           m_nRowCount = std::max(i,m_nRowCount);

    }
    return bCheck;
}




sal_Bool ORowSetCache::moveWindow()
{
    OSL_ENSURE(m_nStartPos >= 0,"ORowSetCache::moveWindow: m_nStartPos is less than 0!");
    OSL_ENSURE(m_nEndPos >= m_nStartPos,"ORowSetCache::moveWindow: m_nStartPos not smaller than m_nEndPos");
    OSL_ENSURE(m_nEndPos-m_nStartPos <= m_nFetchSize,"ORowSetCache::moveWindow: m_nStartPos and m_nEndPos too far apart");

    if ( m_nStartPos < m_nPosition && m_nPosition <= m_nEndPos )
    {
        
        OSL_ENSURE((m_nPosition - m_nStartPos) <= (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
        
        m_aMatrixIter = calcPosition();
        OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(), "New m_aMatrixIter is at end(), but should not.");
        if(!m_aMatrixIter->is())
        {
            sal_Bool bOk( m_pCacheSet->absolute( m_nPosition ) );
            if ( bOk )
            {
                *m_aMatrixIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                m_pCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
                
                
                if ( !m_bRowCountFinal )
                {
                    bOk = m_pCacheSet->absolute_checked( m_nPosition + 1,sal_False );
                    if ( bOk )
                        m_nRowCount = std::max(sal_Int32(m_nPosition+1),m_nRowCount);
                }
            }
            if(!bOk && !m_bRowCountFinal)
            {
                
                m_nRowCount = m_pCacheSet->previous_checked(sal_False) ? m_pCacheSet->getRow() : 0;
                m_bRowCountFinal = sal_True;
            }
        }
        return sal_True;
    }

    sal_Bool bRet = sal_True;

    sal_Int32 nDiff = (m_nFetchSize - 1) / 2;
    sal_Int32 nNewStartPos  = (m_nPosition - nDiff) - 1; //m_nPosition is 1-based, but m_nStartPos is 0-based
    sal_Int32 nNewEndPos    = nNewStartPos + m_nFetchSize;

    if ( nNewStartPos < 0 )
    {
        
        
        nNewEndPos -= nNewStartPos;
        nNewStartPos = 0;
    }

    if ( nNewStartPos < m_nStartPos )
    {   
        if ( nNewEndPos > m_nStartPos )
        {   
            
            
            
            

            sal_Bool bCheck;
            bCheck = m_pCacheSet->absolute(nNewStartPos + 1);

            
            m_nEndPos = std::min(nNewEndPos, m_nEndPos);
            const sal_Int32 nOverlapSize = m_nEndPos - m_nStartPos;
            const sal_Int32 nStartPosOffset = m_nStartPos - nNewStartPos; 
            m_nStartPos = nNewStartPos;
            OSL_ENSURE( static_cast<ORowSetMatrix::size_type>(nOverlapSize) <= m_pMatrix->size(), "new window end is after end of cache matrix!" );
            
            
            ORowSetMatrix::iterator aEnd (m_pMatrix->begin() + nOverlapSize);
            
            ORowSetMatrix::iterator aNewEnd (aEnd + nStartPosOffset);
            
            
            
            
            
            
            
            
            

            if ( bCheck )
            {
                {
                    ORowSetMatrix::iterator aIter(aEnd);
                    sal_Int32 nPos = m_nStartPos + 1;
                    bCheck = fill(aIter, aNewEnd, nPos, bCheck);
                }

                ::std::rotate(m_pMatrix->begin(), aEnd, aNewEnd);
                
                
                ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
                const ORowSetCacheMap::const_iterator aCacheEnd  = m_aCacheIterators.end();
                for(;aCacheIter != aCacheEnd;++aCacheIter)
                {
                    if ( !aCacheIter->second.pRowSet->isInsertRow()
                        && aCacheIter->second.aIterator != m_pMatrix->end() && !m_bModified )
                    {
                        const ptrdiff_t nDist = (aCacheIter->second.aIterator - m_pMatrix->begin());
                        if ( nDist >= nOverlapSize )
                        {
                            
                            aCacheIter->second.aIterator = m_pMatrix->end();
                        }
                        else
                        {
                            
                            CHECK_MATRIX_POS( (nDist + nStartPosOffset) );
                            aCacheIter->second.aIterator += nStartPosOffset;
                            OSL_ENSURE(aCacheIter->second.aIterator >= m_pMatrix->begin()
                                    && aCacheIter->second.aIterator < m_pMatrix->end(),"Iterator out of area!");
                        }
                    }
                }
            }
            else
            { 
                OSL_FAIL("What the hell is happen here!");
                return sal_False;
            }
        }
        else
        {
            bRet = reFillMatrix(nNewStartPos,nNewEndPos);
        }
    }

    OSL_ENSURE(nNewStartPos >= m_nStartPos, "ORowSetCache::moveWindow internal error: new start pos before current start pos");
    if ( m_nEndPos < nNewEndPos )
    {   
        if( nNewStartPos < m_nEndPos )
        {   
            const sal_Int32 nRowsInCache = m_nEndPos - m_nStartPos;
            if ( nRowsInCache < m_nFetchSize )
            {
                
                CHECK_MATRIX_POS(nRowsInCache);
                sal_Int32 nPos = m_nEndPos + 1;
                sal_Bool bCheck = m_pCacheSet->absolute(nPos);
                ORowSetMatrix::iterator aIter = m_pMatrix->begin() + nRowsInCache;
                const sal_Int32 nRowsToFetch = std::min(nNewEndPos-m_nEndPos, m_nFetchSize-nRowsInCache);
                const ORowSetMatrix::const_iterator aEnd = aIter + nRowsToFetch;
                bCheck = fill(aIter, aEnd, nPos, bCheck);
                m_nEndPos = nPos - 1;
                OSL_ENSURE( (!bCheck && m_nEndPos <= nNewEndPos ) ||
                            ( bCheck && m_nEndPos == nNewEndPos ),
                             "ORowSetCache::moveWindow opportunistic fetch-after-current-end went badly");
            }

            
            
            
            ORowSetMatrix::iterator aIter = m_pMatrix->begin();
            const sal_Int32 nNewStartPosInMatrix = nNewStartPos - m_nStartPos;
            CHECK_MATRIX_POS( nNewStartPosInMatrix );
            
            const ORowSetMatrix::const_iterator aEnd  = m_pMatrix->begin() + nNewStartPosInMatrix;
            
            ORowSetMatrix::iterator aDataEnd  = m_pMatrix->begin() + (m_nEndPos - m_nStartPos);

            sal_Int32 nPos = m_nEndPos + 1;
            sal_Bool bCheck = m_pCacheSet->absolute(nPos);
            bCheck = fill(aIter, aEnd, nPos, bCheck); 
            //aIter and nPos are now the position *after* last filled in one!

            
            if(bCheck)
            {
                OSL_ENSURE(aIter == aEnd, "fill() said went till end, but did not.");
                
                ::std::rotate(m_pMatrix->begin(), aIter, aDataEnd);
                
                rotateCacheIterator( nNewStartPosInMatrix );
                m_nStartPos = nNewStartPos;
                m_nEndPos = nNewEndPos;
                
                
                
                sal_Bool bOk = sal_True;
                if(!m_bRowCountFinal)
                    bOk = m_pCacheSet->next();
                if(!bOk)
                {
                    m_pCacheSet->previous_checked(sal_False); 
                    m_nRowCount      = nPos; 
                    OSL_ENSURE(nPos == m_pCacheSet->getRow(),"nPos is not valid!");
                    m_bRowCountFinal = sal_True;
                }
                else if(!m_bRowCountFinal)
                    m_nRowCount = std::max(nPos+1, m_nRowCount); //+1 because we successfully moved to row after nPos
                else
                    OSL_ENSURE(m_nRowCount >= nPos, "Final m_nRowCount is smaller than row I moved to!");
            }
            else
            {   
                
                const ORowSetMatrix::iterator::difference_type nFetchedRows  = aIter - m_pMatrix->begin();
                
                
                

                nPos -= 1;
                m_nStartPos += nFetchedRows;
                m_nEndPos = nPos;
                ::std::rotate(m_pMatrix->begin(), aIter, aDataEnd);
                
                rotateCacheIterator( nFetchedRows );

                if ( !m_bRowCountFinal )
                {
                    m_pCacheSet->previous_checked(sal_False);                   
                    m_nRowCount      = std::max(m_nRowCount, nPos);    
                    OSL_ENSURE(nPos == m_pCacheSet->getRow(),"nPos isn't valid!");
                    m_bRowCountFinal = sal_True;
                }

            }
            
            if(!m_pMatrix->begin()->is())
            {
                aIter = m_pMatrix->begin();

                nPos    = m_nStartPos + 1;
                bCheck  = m_pCacheSet->absolute_checked(nPos, sal_True);
                for(; !aIter->is() && bCheck;++aIter, ++nPos)
                {
                    OSL_ENSURE(aIter != m_pMatrix->end(),"Invalid iterator");

                    *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                    m_pCacheSet->fillValueRow(*aIter, nPos);

                    bCheck = m_pCacheSet->next();
                }
            }
        }
        else 
            bRet = reFillMatrix(nNewStartPos,nNewEndPos);
    }

    if(!m_bRowCountFinal)
       m_nRowCount = std::max(m_nPosition,m_nRowCount);
    OSL_ENSURE(m_nStartPos >= 0,"ORowSetCache::moveWindow: m_nStartPos is less than 0!");
    OSL_ENSURE(m_nEndPos > m_nStartPos,"ORowSetCache::moveWindow: m_nStartPos not smaller than m_nEndPos");
    OSL_ENSURE(m_nEndPos-m_nStartPos <= m_nFetchSize,"ORowSetCache::moveWindow: m_nStartPos and m_nEndPos too far apart");

    return bRet;
}

sal_Bool ORowSetCache::first(  )
{
    
    
    
    
    sal_Bool bRet = m_pCacheSet->first();
    if(bRet)
    {
        m_bBeforeFirst  = m_bAfterLast = sal_False;
        m_nPosition     = 1;
        moveWindow();
        m_aMatrixIter   = m_pMatrix->begin();
    }
    else
    {
        m_bRowCountFinal = m_bBeforeFirst = m_bAfterLast = sal_True;
        m_nRowCount = m_nPosition = 0;

        OSL_ENSURE(m_bBeforeFirst || m_bNew,"ORowSetCache::first return false and BeforeFirst isn't true");
        m_aMatrixIter = m_pMatrix->end();
    }
    return bRet;
}

sal_Bool ORowSetCache::last(  )
{
    sal_Bool bRet = m_pCacheSet->last();
    if(bRet)
    {
        m_bBeforeFirst = m_bAfterLast = sal_False;
        if(!m_bRowCountFinal)
        {
            m_bRowCountFinal = sal_True;
            m_nRowCount = m_pCacheSet->getRow(); 
        }
        m_nPosition = m_pCacheSet->getRow();
        moveWindow();
        
        m_pCacheSet->last();
        OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
        m_aMatrixIter = calcPosition();
    }
    else
    {
        m_bRowCountFinal = m_bBeforeFirst = m_bAfterLast = sal_True;
        m_nRowCount = m_nPosition = 0;
        OSL_ENSURE(m_bBeforeFirst,"ORowSetCache::last return false and BeforeFirst isn't true");
        m_aMatrixIter = m_pMatrix->end();
    }
#if OSL_DEBUG_LEVEL > 1
    if(bRet)
    {
        OSL_ENSURE((*m_aMatrixIter).is(),"ORowSetCache::last: Row not valid!");
    }
#endif

    return bRet;
}

sal_Int32 ORowSetCache::getRow(  )
{
    return (isBeforeFirst() || isAfterLast()) ? 0 : m_nPosition;
}

sal_Bool ORowSetCache::absolute( sal_Int32 row )
{
    if(!row )
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_ABS_ZERO),NULL,SQLSTATE_GENERAL,1000,Any() );

    if(row < 0)
    {
        
        
        if(m_bRowCountFinal || last())
        {
            m_nPosition = m_nRowCount + row + 1; 
            if(m_nPosition < 1)
            {
                m_bBeforeFirst = sal_True;
                m_bAfterLast = sal_False;
                m_aMatrixIter = m_pMatrix->end();
            }
            else
            {
                m_bBeforeFirst  = sal_False;
                m_bAfterLast    = m_nPosition > m_nRowCount;
                moveWindow();
                OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
                m_aMatrixIter = calcPosition();
            }
        }
        else
            m_aMatrixIter = m_pMatrix->end();
    }
    else
    {
        m_nPosition = row;
        
        m_bBeforeFirst  = sal_False;
        checkPositionFlags();

        if(!m_bAfterLast)
        {
            moveWindow();
            checkPositionFlags();
            if(!m_bAfterLast)
                m_aMatrixIter = calcPosition();
            else
                m_aMatrixIter = m_pMatrix->end();
        }
        else
            m_aMatrixIter = m_pMatrix->end();
    }

    return !(m_bAfterLast || m_bBeforeFirst);
}

sal_Bool ORowSetCache::relative( sal_Int32 rows )
{
    sal_Bool bErg = sal_True;
    if(rows)
    {
        sal_Int32 nNewPosition = m_nPosition + rows;

        if ( m_bBeforeFirst && rows > 0 )
            nNewPosition = rows;
        else if ( m_bRowCountFinal && m_bAfterLast && rows < 0 )
            nNewPosition = m_nRowCount + 1 + rows;
        else
            if ( m_bBeforeFirst || ( m_bRowCountFinal && m_bAfterLast ) )
                throw SQLException( DBACORE_RESSTRING( RID_STR_NO_RELATIVE ), NULL, SQLSTATE_GENERAL, 1000, Any() );
        if ( nNewPosition )
        {
            bErg = absolute( nNewPosition );
            bErg = bErg && !isAfterLast() && !isBeforeFirst();
        }
        else
        {
            m_bBeforeFirst = sal_True;
            bErg = sal_False;
        }
    }
    return bErg;
}

sal_Bool ORowSetCache::previous(  )
{
    sal_Bool bRet = sal_False;
    if(!isBeforeFirst())
    {
        if(m_bAfterLast)   
            bRet = last();
        else
        {
            m_bAfterLast = sal_False;
            --m_nPosition;
            moveWindow();
            OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");

            checkPositionFlags();

            if(!m_nPosition)
            {
                m_bBeforeFirst = sal_True;
                m_aMatrixIter = m_pMatrix->end();
            }
            else
            {
                m_aMatrixIter = calcPosition();
                bRet = (*m_aMatrixIter).is();
            }
        }
    }
    return bRet;
}

void ORowSetCache::refreshRow(  )
{
    if(isAfterLast())
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_REFESH_AFTERLAST),NULL,SQLSTATE_GENERAL,1000,Any() );
    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"refreshRow() called for invalid row!");
    m_pCacheSet->refreshRow();
    m_pCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
    if ( m_bNew )
    {
        cancelRowModification();
    }
}

sal_Bool ORowSetCache::rowUpdated(  )
{
    return m_pCacheSet->rowUpdated();
}

sal_Bool ORowSetCache::rowInserted(  )
{
    return m_pCacheSet->rowInserted();
}


sal_Bool ORowSetCache::insertRow(::std::vector< Any >& o_aBookmarks)
{
    if ( !m_bNew || !m_aInsertRow->is() )
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_MOVETOINSERTROW_CALLED),NULL,SQLSTATE_GENERAL,1000,Any() );

    m_pCacheSet->insertRow(*m_aInsertRow,m_aUpdateTable);

    sal_Bool bRet( rowInserted() );
    if ( bRet )
    {
        ++m_nRowCount;
        Any aBookmark = ((*m_aInsertRow)->get())[0].makeAny();
        m_bAfterLast = m_bBeforeFirst = sal_False;
        if(aBookmark.hasValue())
        {
            moveToBookmark(aBookmark);
            
            ORowSetValueVector::Vector& rCurrentRow = ((*m_aMatrixIter))->get();
            ORowSetMatrix::iterator aIter = m_pMatrix->begin();
            for(;aIter != m_pMatrix->end();++aIter)
            {
                if ( m_aMatrixIter != aIter && aIter->is() && m_pCacheSet->columnValuesUpdated((*aIter)->get(),rCurrentRow) )
                {
                    o_aBookmarks.push_back(lcl_getBookmark((*aIter)->get()[0],m_pCacheSet));
                }
            }
        }
        else
        {
            OSL_FAIL("There must be a bookmark after the row was inserted!");
        }
    }
    return bRet;
}

void ORowSetCache::resetInsertRow(sal_Bool _bClearInsertRow)
{
    if ( _bClearInsertRow )
        clearInsertRow();
    m_bNew      = sal_False;
    m_bModified = sal_False;
}

void ORowSetCache::cancelRowModification()
{
    
    ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
    ORowSetCacheMap::iterator aCacheEnd = m_aCacheIterators.end();
    for(;aCacheIter != aCacheEnd;++aCacheIter)
    {
        if ( aCacheIter->second.pRowSet->isInsertRow() && aCacheIter->second.aIterator == m_aInsertRow )
            aCacheIter->second.aIterator = m_pMatrix->end();
    }
    resetInsertRow(sal_False);
}

void ORowSetCache::updateRow( ORowSetMatrix::iterator& _rUpdateRow,::std::vector< Any >& o_aBookmarks )
{
    if(isAfterLast() || isBeforeFirst())
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_UPDATEROW),NULL,SQLSTATE_GENERAL,1000,Any() );

    Any aBookmark = ((*_rUpdateRow)->get())[0].makeAny();
    OSL_ENSURE(aBookmark.hasValue(),"Bookmark must have a value!");
    
    
    moveToBookmark(aBookmark);
    m_pCacheSet->updateRow(*_rUpdateRow,*m_aMatrixIter,m_aUpdateTable);
    
    (*m_aMatrixIter) = NULL;

    if ( moveToBookmark(aBookmark) )
    {
        
        ORowSetValueVector::Vector& rCurrentRow = ((*m_aMatrixIter))->get();
        ORowSetMatrix::iterator aIter = m_pMatrix->begin();
        for(;aIter != m_pMatrix->end();++aIter)
        {
            if ( m_aMatrixIter != aIter && aIter->is() && m_pCacheSet->columnValuesUpdated((*aIter)->get(),rCurrentRow) )
            {
                o_aBookmarks.push_back(lcl_getBookmark((*aIter)->get()[0],m_pCacheSet));
            }
        }
    }

    m_bModified = sal_False;
}

bool ORowSetCache::deleteRow(  )
{
    if(isAfterLast() || isBeforeFirst())
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_DELETEROW),NULL,SQLSTATE_GENERAL,1000,Any() );

    m_pCacheSet->deleteRow(*m_aMatrixIter,m_aUpdateTable);
    if ( !m_pCacheSet->rowDeleted() )
        return false;

    --m_nRowCount;
    OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
    ORowSetMatrix::iterator aPos = calcPosition();
    (*aPos)   = NULL;

    ORowSetMatrix::iterator aEnd = m_pMatrix->end();
    for(++aPos;aPos != aEnd && aPos->is();++aPos)
    {
        *(aPos-1) = *aPos;
        (*aPos)   = NULL;
    }
    m_aMatrixIter = m_pMatrix->end();

    --m_nPosition;
    return true;
}

void ORowSetCache::cancelRowUpdates(  )
{
    m_bNew = m_bModified = sal_False;
    if(!m_nPosition)
    {
        OSL_FAIL("cancelRowUpdates:Invalid positions pos == 0");
        ::dbtools::throwFunctionSequenceException(NULL);
    }

    if(m_pCacheSet->absolute(m_nPosition))
        m_pCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
    else
    {
        OSL_FAIL("cancelRowUpdates couldn't position right with absolute");
        ::dbtools::throwFunctionSequenceException(NULL);
    }
}

void ORowSetCache::moveToInsertRow(  )
{
    m_bNew      = sal_True;
    m_bUpdated  = m_bAfterLast = sal_False;

    m_aInsertRow = m_pInsertMatrix->begin();
    if(!m_aInsertRow->is())
        *m_aInsertRow = new ORowSetValueVector(m_xMetaData->getColumnCount());

    
    ORowSetValueVector::Vector::iterator aIter = (*m_aInsertRow)->get().begin()+1;
    ORowSetValueVector::Vector::iterator aEnd = (*m_aInsertRow)->get().end();
    for(sal_Int32 i = 1;aIter != aEnd;++aIter,++i)
    {
        aIter->setBound(false);
        aIter->setModified(false);
        aIter->setNull();
        aIter->setTypeKind(m_xMetaData->getColumnType(i));
    }
}

ORowSetCacheIterator ORowSetCache::createIterator(ORowSetBase* _pRowSet)
{
    ORowSetCacheIterator_Helper aHelper;
    aHelper.aIterator = m_pMatrix->end();
    aHelper.pRowSet = _pRowSet;
    return ORowSetCacheIterator(m_aCacheIterators.insert(m_aCacheIterators.begin(),ORowSetCacheMap::value_type(m_aCacheIterators.size()+1,aHelper)),this,_pRowSet);
}

void ORowSetCache::deleteIterator(const ORowSetBase* _pRowSet)
{
    ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
    for(;aCacheIter != m_aCacheIterators.end();)
    {
        if ( aCacheIter->second.pRowSet == _pRowSet )
        {
            m_aCacheIterators.erase(aCacheIter);
            aCacheIter = m_aCacheIterators.begin();
        }
        else
            ++aCacheIter;
    }
}

void ORowSetCache::rotateCacheIterator(ORowSetMatrix::difference_type _nDist)
{
    if(_nDist)
    {
        
        ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
        ORowSetCacheMap::iterator aCacheEnd  = m_aCacheIterators.end();
        for(;aCacheIter != aCacheEnd;++aCacheIter)
        {
            if ( !aCacheIter->second.pRowSet->isInsertRow()
                && aCacheIter->second.aIterator != m_pMatrix->end() && !m_bModified )
            {
                ptrdiff_t nDist = (aCacheIter->second.aIterator - m_pMatrix->begin());
                if(nDist < _nDist)
                {
                    aCacheIter->second.aIterator = m_pMatrix->end();
                }
                else
                {
                    OSL_ENSURE((aCacheIter->second.aIterator - m_pMatrix->begin()) >= _nDist,"Invalid Dist value!");
                    aCacheIter->second.aIterator -= _nDist;
                    OSL_ENSURE(aCacheIter->second.aIterator >= m_pMatrix->begin()
                            && aCacheIter->second.aIterator < m_pMatrix->end(),"Iterator out of area!");
                }
            }
        }
    }
}

void ORowSetCache::setUpdateIterator(const ORowSetMatrix::iterator& _rOriginalRow)
{
    m_aInsertRow = m_pInsertMatrix->begin();
    if(!m_aInsertRow->is())
        *m_aInsertRow = new ORowSetValueVector(m_xMetaData->getColumnCount());

    (*(*m_aInsertRow)) = (*(*_rOriginalRow));
    
    ORowSetValueVector::Vector::iterator aIter = (*m_aInsertRow)->get().begin();
    ORowSetValueVector::Vector::iterator aEnd = (*m_aInsertRow)->get().end();
    for(;aIter != aEnd;++aIter)
        aIter->setModified(false);
}

void ORowSetCache::checkPositionFlags()
{
    if(m_bRowCountFinal)
    {
        m_bAfterLast    = m_nPosition > m_nRowCount;
        if(m_bAfterLast)
            m_nPosition = 0;//m_nRowCount;
    }
}

void ORowSetCache::checkUpdateConditions(sal_Int32 columnIndex)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->get().size())
        throwFunctionSequenceException(m_xSet.get());
}

sal_Bool ORowSetCache::checkInnerJoin(const ::connectivity::OSQLParseNode *pNode,const Reference< XConnection>& _xConnection,const OUString& _sUpdateTableName)
{
    sal_Bool bOk = sal_False;
    if (pNode->count() == 3 &&  
        SQL_ISPUNCTUATION(pNode->getChild(0),"(") &&
        SQL_ISPUNCTUATION(pNode->getChild(2),")"))
    {
        bOk = checkInnerJoin(pNode->getChild(1),_xConnection,_sUpdateTableName);
    }
    else if ((SQL_ISRULE(pNode,search_condition) || SQL_ISRULE(pNode,boolean_term)) && 
                pNode->count() == 3)
    {
        
        if ( SQL_ISTOKEN(pNode->getChild(1),AND) )
            bOk = checkInnerJoin(pNode->getChild(0),_xConnection,_sUpdateTableName)
                && checkInnerJoin(pNode->getChild(2),_xConnection,_sUpdateTableName);
    }
    else if (SQL_ISRULE(pNode,comparison_predicate))
    {
        
        OSL_ENSURE(pNode->count() == 3,"checkInnerJoin: Fehler im Parse Tree");
        if (!(SQL_ISRULE(pNode->getChild(0),column_ref) &&
                SQL_ISRULE(pNode->getChild(2),column_ref) &&
                pNode->getChild(1)->getNodeType() == SQL_NODE_EQUAL))
        {
            bOk = sal_False;
        }
        OUString sColumnName,sTableRange;
        OSQLParseTreeIterator::getColumnRange( pNode->getChild(0), _xConnection, sColumnName, sTableRange );
        bOk = sTableRange == _sUpdateTableName;
        if ( !bOk )
        {
            OSQLParseTreeIterator::getColumnRange( pNode->getChild(2), _xConnection, sColumnName, sTableRange );
            bOk =  sTableRange == _sUpdateTableName;
        }
    }
    return bOk;
}

sal_Bool ORowSetCache::checkJoin(const Reference< XConnection>& _xConnection,
                                 const Reference< XSingleSelectQueryAnalyzer >& _xAnalyzer,
                                 const OUString& _sUpdateTableName )
{
    sal_Bool bOk = sal_False;
    OUString sSql = _xAnalyzer->getQuery();
    OUString sErrorMsg;
    ::connectivity::OSQLParser aSqlParser( m_aContext );
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ::connectivity::OSQLParseNode> pSqlParseNode( aSqlParser.parseTree(sErrorMsg,sSql));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if ( pSqlParseNode.get() && SQL_ISRULE(pSqlParseNode, select_statement) )
    {
        OSQLParseNode* pTableRefCommalist = pSqlParseNode->getByRule(::connectivity::OSQLParseNode::table_ref_commalist);
        OSL_ENSURE(pTableRefCommalist,"NO tables why!?");
        if(pTableRefCommalist && pTableRefCommalist->count() == 1)
        {
            
            OSQLParseNode* pJoin = pTableRefCommalist->getByRule(::connectivity::OSQLParseNode::qualified_join);
            if(pJoin)
            { 
                OSQLParseNode* pJoinType    = pJoin->getChild(1);
                OSQLParseNode* pOuterType   = NULL;
                if(SQL_ISRULE(pJoinType,join_type) && pJoinType->count() == 2)
                    pOuterType = pJoinType->getChild(0);
                else if(SQL_ISRULE(pJoinType,outer_join_type))
                    pOuterType = pJoinType;

                sal_Bool bCheck     = sal_False;
                sal_Bool bLeftSide  = sal_False;
                if(pOuterType)
                { 
                    bLeftSide = SQL_ISTOKEN(pOuterType->getChild(0),LEFT);
                    bCheck = bLeftSide || SQL_ISTOKEN(pOuterType->getChild(0),RIGHT);
                }

                if(bCheck)
                { 
                    const OSQLParseNode* pTableRef = pJoin->getByRule(::connectivity::OSQLParseNode::qualified_join);
                    if(bLeftSide)
                        pTableRef = pJoin->getChild(0);
                    else
                        pTableRef = pJoin->getChild(3);
                    OSL_ENSURE(SQL_ISRULE(pTableRef,table_ref),"Must be a tableref here!");

                    OUString sTableRange = OSQLParseNode::getTableRange(pTableRef);
                    if(sTableRange.isEmpty())
                        pTableRef->getChild(0)->parseNodeToStr( sTableRange, _xConnection, NULL, false, false );
                    bOk =  sTableRange == _sUpdateTableName;
                }
            }
        }
        else
        {
            OSQLParseNode* pWhereOpt = pSqlParseNode->getChild(3)->getChild(1);
            if ( pWhereOpt && !pWhereOpt->isLeaf() )
                bOk = checkInnerJoin(pWhereOpt->getChild(1),_xConnection,_sUpdateTableName);
        }
    }
    return bOk;
}

void ORowSetCache::clearInsertRow()
{
    
    if ( m_aInsertRow != m_pInsertMatrix->end() && m_aInsertRow->is() )
    {
        ORowSetValueVector::Vector::iterator aIter = (*m_aInsertRow)->get().begin()+1;
        ORowSetValueVector::Vector::iterator aEnd = (*m_aInsertRow)->get().end();
        for(;aIter != aEnd;++aIter)
        {
            aIter->setBound(false);
            aIter->setModified(false);
            aIter->setNull();
        }
    }
}

ORowSetMatrix::iterator ORowSetCache::calcPosition() const
{
    sal_Int32 nValue = (m_nPosition - m_nStartPos) - 1;
    CHECK_MATRIX_POS(nValue);
    return ( nValue < 0 || nValue >= static_cast<sal_Int32>(m_pMatrix->size()) ) ? m_pMatrix->end() : (m_pMatrix->begin() + nValue);
}

TORowSetOldRowHelperRef ORowSetCache::registerOldRow()
{
    TORowSetOldRowHelperRef pRef = new ORowSetOldRowHelper(ORowSetRow());
    m_aOldRows.push_back(pRef);
    return pRef;
}

void ORowSetCache::deregisterOldRow(const TORowSetOldRowHelperRef& _rRow)
{
    TOldRowSetRows::iterator aOldRowEnd = m_aOldRows.end();
    for (TOldRowSetRows::iterator aOldRowIter = m_aOldRows.begin(); aOldRowIter != aOldRowEnd; ++aOldRowIter)
    {
        if ( aOldRowIter->get() == _rRow.get() )
        {
            m_aOldRows.erase(aOldRowIter);
            break;
        }

    }
}

sal_Bool ORowSetCache::reFillMatrix(sal_Int32 _nNewStartPos,sal_Int32 _nNewEndPos)
{
    const TOldRowSetRows::const_iterator aOldRowEnd = m_aOldRows.end();
    for (TOldRowSetRows::iterator aOldRowIter = m_aOldRows.begin(); aOldRowIter != aOldRowEnd; ++aOldRowIter)
    {
        if ( aOldRowIter->is() && (*aOldRowIter)->getRow().is() )
            (*aOldRowIter)->setRow(new ORowSetValueVector( *((*aOldRowIter)->getRow()) ) );
    }
    sal_Int32 nNewSt = _nNewStartPos;
    sal_Bool bRet = fillMatrix(nNewSt,_nNewEndPos);
    m_nStartPos = nNewSt;
    m_nEndPos = _nNewEndPos;
    rotateCacheIterator(static_cast<ORowSetMatrix::difference_type>(m_nFetchSize+1)); 
    return bRet;
}

sal_Bool ORowSetCache::fill(ORowSetMatrix::iterator& _aIter,const ORowSetMatrix::const_iterator& _aEnd,sal_Int32& _nPos,sal_Bool _bCheck)
{
    const sal_Int32 nColumnCount = m_xMetaData->getColumnCount();
    for(; _bCheck && _aIter != _aEnd; _aIter++, _nPos++)
    {
        if ( !_aIter->is() )
            *_aIter = new ORowSetValueVector(nColumnCount);
        else
        {
            const TOldRowSetRows::const_iterator aOldRowEnd = m_aOldRows.end();
            for (TOldRowSetRows::iterator aOldRowIter = m_aOldRows.begin(); aOldRowIter != aOldRowEnd; ++aOldRowIter)
            {
                if ( (*aOldRowIter)->getRow() == *_aIter )
                    *_aIter = new ORowSetValueVector(nColumnCount);
            }
        }
        m_pCacheSet->fillValueRow(*_aIter, _nPos);
        _bCheck = m_pCacheSet->next();
    }
    return _bCheck;
}

bool ORowSetCache::isResultSetChanged() const
{
    return m_pCacheSet->isResultSetChanged();
}

void ORowSetCache::reset(const Reference< XResultSet>& _xDriverSet)
{
    m_xSet = _xDriverSet;
    m_xMetaData.set(Reference< XResultSetMetaDataSupplier >(_xDriverSet,UNO_QUERY)->getMetaData());
    m_pCacheSet->reset(_xDriverSet);

    m_bRowCountFinal = sal_False;
    m_nRowCount = 0;
    reFillMatrix(m_nStartPos,m_nEndPos);
}

void ORowSetCache::impl_updateRowFromCache_throw(ORowSetValueVector::Vector& io_aRow
                                           ,::std::vector<sal_Int32>& o_ChangedColumns)
{
    if ( o_ChangedColumns.size() > 1 )
    {
        ORowSetMatrix::iterator aIter = m_pMatrix->begin();
        for(;aIter != m_pMatrix->end();++aIter)
        {
            if ( aIter->is() && m_pCacheSet->updateColumnValues((*aIter)->get(),io_aRow,o_ChangedColumns))
            {
                break;
            }
        }

        if ( aIter == m_pMatrix->end() )
        {
            m_pCacheSet->fillMissingValues(io_aRow);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
