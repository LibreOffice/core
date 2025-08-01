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
#include "BookmarkSet.hxx"
#include "KeySet.hxx"
#include "OptimisticSet.hxx"
#include "RowSetBase.hxx"
#include "RowSetCache.hxx"
#include "StaticSet.hxx"
#include "WrappedResultSet.hxx"
#include <core_resource.hxx>
#include <strings.hrc>
#include <strings.hxx>

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>

#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlnode.hxx>
#include <connectivity/sqlparse.hxx>
#include <sqlbison.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/safeint.hxx>
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
using namespace ::cppu;

// This class calls m_pCacheSet->FOO_checked(..., sal_False)
// (where FOO is absolute, last, previous)
// when it does not immediately care about the values in the row's columns.
// As a corollary, m_pCacheSet may be left in an inconsistent state,
// and all ->fillFOO calls (and ->getFOO) may fail or give wrong results,
// until m_pCacheSet is moved (or refreshed) again.
// So always make sure m_pCacheSet is moved or refreshed before accessing column values.


ORowSetCache::ORowSetCache(const Reference< XResultSet >& _xRs,
                           const Reference< XSingleSelectQueryAnalyzer >& _xAnalyzer,
                           const Reference<XComponentContext>& _rContext,
                           const OUString& _rUpdateTableName,
                           bool&    _bModified,
                           bool&    _bNew,
                           const ORowSetValueVector& _aParameterValueForCache,
                           const OUString& i_sRowSetFilter,
                           sal_Int32 i_nMaxRows)
    :m_xSet(_xRs)
    ,m_xMetaData(Reference< XResultSetMetaDataSupplier >(_xRs,UNO_QUERY_THROW)->getMetaData())
    ,m_aContext( _rContext )
    ,m_nFetchSize(0)
    ,m_nRowCount(0)
    ,m_nPrivileges( Privilege::SELECT )
    ,m_nPosition(0)
    ,m_nStartPos(0)
    ,m_nEndPos(0)
    ,m_bRowCountFinal(false)
    ,m_bBeforeFirst(true)
    ,m_bAfterLast( false )
    ,m_bModified(_bModified)
    ,m_bNew(_bNew)
{

    // first try if the result can be used to do inserts and updates
    Reference< XPropertySet> xProp(_xRs,UNO_QUERY);
    Reference< XPropertySetInfo > xPropInfo = xProp->getPropertySetInfo();
    bool bBookmarkable = false;
    try
    {
        Reference< XResultSetUpdate> xUp(_xRs,UNO_QUERY_THROW);
        bBookmarkable = xPropInfo->hasPropertyByName(PROPERTY_ISBOOKMARKABLE) &&
                                any2bool(xProp->getPropertyValue(PROPERTY_ISBOOKMARKABLE)) && Reference< XRowLocate >(_xRs, UNO_QUERY).is();
        if ( bBookmarkable )
        {
            xUp->moveToInsertRow();
            xUp->moveToCurrentRow();
            xUp->cancelRowUpdates();
            _xRs->beforeFirst();
            m_nPrivileges = Privilege::SELECT|Privilege::DELETE|Privilege::INSERT|Privilege::UPDATE;
            m_xCacheSet = new WrappedResultSet(i_nMaxRows);
            m_xCacheSet->construct(_xRs,i_sRowSetFilter);
            return;
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess.core");
    }
    try
    {
        if ( xPropInfo->hasPropertyByName(PROPERTY_RESULTSETTYPE) &&
                            ::comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETTYPE)) != ResultSetType::FORWARD_ONLY)
            _xRs->beforeFirst();
    }
    catch(const SQLException&)
    {
        TOOLS_WARN_EXCEPTION("dbaccess.core", "ORowSetCache");
    }

    // check if all keys of the updateable table are fetched
    bool bAllKeysFound = false;
    sal_Int32 nTablesCount = 0;

    bool bNeedKeySet = !bBookmarkable || (xPropInfo->hasPropertyByName(PROPERTY_RESULTSETCONCURRENCY) &&
                            ::comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETCONCURRENCY)) == ResultSetConcurrency::READ_ONLY);

    OUString aUpdateTableName = _rUpdateTableName;
    Reference< XConnection> xConnection;
    // first we need a connection
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
            {// here we have a join or union and nobody told us which table to update, so we update them all
                m_nPrivileges = Privilege::SELECT|Privilege::DELETE|Privilege::INSERT|Privilege::UPDATE;
                rtl::Reference<OptimisticSet> pCursor = new OptimisticSet(m_aContext,xConnection,_xAnalyzer,_aParameterValueForCache,i_nMaxRows,m_nRowCount);
                m_xCacheSet = pCursor;
                try
                {
                    m_xCacheSet->construct(_xRs,i_sRowSetFilter);
                    if ( pCursor->isReadOnly() )
                        m_nPrivileges = Privilege::SELECT;
                    m_aKeyColumns = pCursor->getJoinedKeyColumns();
                    return;
                }
                catch (const Exception&)
                {
                    TOOLS_WARN_EXCEPTION("dbaccess.core", "ORowSetCache");
                }
                m_xCacheSet.clear();
            }
            else
            {
                if(!_rUpdateTableName.isEmpty() && xTables->hasByName(_rUpdateTableName))
                    xTables->getByName(_rUpdateTableName) >>= m_aUpdateTable;
                else if(xTables->getElementNames().hasElements())
                {
                    aUpdateTableName = xTables->getElementNames()[0];
                    xTables->getByName(aUpdateTableName) >>= m_aUpdateTable;
                }
                Reference<XIndexAccess> xIndexAccess(xTables,UNO_QUERY);
                if(xIndexAccess.is())
                    nTablesCount = xIndexAccess->getCount();
                else
                    nTablesCount = xTables->getElementNames().getLength();

                if(m_aUpdateTable.is() && nTablesCount < 3) // for we can't handle more than 2 tables in our keyset
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
                            SelectColumnsMetaData aColumnNames(comphelper::UStringMixLess(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers()));
                            ::dbaccess::getColumnPositions(xSelColumns,xPrimaryKeyColumns->getElementNames(),aUpdateTableName,aColumnNames);
                            bAllKeysFound = !aColumnNames.empty() && aColumnNames.size() == o3tl::make_unsigned(xPrimaryKeyColumns->getElementNames().getLength());
                        }
                    }
                }
            }
        }
        catch (Exception const&)
        {
            TOOLS_WARN_EXCEPTION("dbaccess.core", "ORowSetCache");
        }
    }

    // first check if resultset is bookmarkable
    if(!bNeedKeySet)
    {
        try
        {
            m_xCacheSet = new OBookmarkSet(i_nMaxRows);
            m_xCacheSet->construct(_xRs,i_sRowSetFilter);

            // check privileges
            m_nPrivileges = Privilege::SELECT;
            if(Reference<XResultSetUpdate>(_xRs,UNO_QUERY).is())  // this interface is optional so we have to check it
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
        catch (const SQLException&)
        {
            TOOLS_WARN_EXCEPTION("dbaccess.core", "ORowSetCache");
            bNeedKeySet = true;
        }

    }
    if(bNeedKeySet)
    {
        // need to check if we could handle this select clause
        bAllKeysFound = bAllKeysFound && (nTablesCount == 1 || checkJoin(xConnection,_xAnalyzer,aUpdateTableName));

        if(!bAllKeysFound )
        {
            if ( bBookmarkable )
            {
                // here I know that we have a read only bookmarkable cursor
                _xRs->beforeFirst();
                m_nPrivileges = Privilege::SELECT;
                m_xCacheSet = new WrappedResultSet(i_nMaxRows);
                m_xCacheSet->construct(_xRs,i_sRowSetFilter);
                return;
            }
            m_xCacheSet = new OStaticSet(i_nMaxRows);
            m_xCacheSet->construct(_xRs,i_sRowSetFilter);
            m_nPrivileges = Privilege::SELECT;
        }
        else
        {
            Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
            SelectColumnsMetaData aColumnNames(comphelper::UStringMixLess(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers()));
            Reference<XColumnsSupplier> xColSup(_xAnalyzer,UNO_QUERY);
            Reference<XNameAccess> xSelColumns  = xColSup->getColumns();
            Reference<XNameAccess> xColumns     = m_aUpdateTable->getColumns();
            ::dbaccess::getColumnPositions(xSelColumns,xColumns->getElementNames(),aUpdateTableName,aColumnNames);

            // check privileges
            m_nPrivileges = Privilege::SELECT;
            bool bNoInsert = false;

            for (auto& columnName : xColumns->getElementNames())
            {
                Reference<XPropertySet> xColumn(xColumns->getByName(columnName), UNO_QUERY);
                OSL_ENSURE(xColumn.is(),"Column in table is null!");
                if(xColumn.is())
                {
                    sal_Int32 nNullable = 0;
                    xColumn->getPropertyValue(PROPERTY_ISNULLABLE) >>= nNullable;
                    if(nNullable == ColumnValue::NO_NULLS && aColumnNames.find(columnName) == aColumnNames.end())
                    { // we found a column where null is not allowed so we can't insert new values
                        bNoInsert = true;
                        break; // one column is enough
                    }
                }
            }

            rtl::Reference<OKeySet> pKeySet = new OKeySet(m_aUpdateTable, aUpdateTableName ,_xAnalyzer,_aParameterValueForCache,i_nMaxRows,m_nRowCount);
            try
            {
                m_xCacheSet = pKeySet;
                pKeySet->construct(_xRs,i_sRowSetFilter);

                if(Reference<XResultSetUpdate>(_xRs,UNO_QUERY).is())  // this interface is optional so we have to check it
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
                    m_nPrivileges |= ~Privilege::INSERT; // remove the insert privilege
            }
            catch (const SQLException&)
            {
                TOOLS_WARN_EXCEPTION("dbaccess.core", "ORowSetCache");
                // we couldn't create a keyset here so we have to create a static cache
                m_xCacheSet = new OStaticSet(i_nMaxRows);
                m_xCacheSet->construct(_xRs,i_sRowSetFilter);
                m_nPrivileges = Privilege::SELECT;
            }
        }

    }
    // last check
    if(!bAllKeysFound && xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_RESULTSETCONCURRENCY) &&
        ::comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETCONCURRENCY)) == ResultSetConcurrency::READ_ONLY)
        m_nPrivileges = Privilege::SELECT;
}

ORowSetCache::~ORowSetCache()
{
    m_xCacheSet.clear();
    if(m_pMatrix)
    {
        m_pMatrix->clear();
        m_pMatrix.reset();
    }

    if(m_pInsertMatrix)
    {
        m_pInsertMatrix->clear();
        m_pInsertMatrix.reset();
    }
    m_xSet          = WeakReference< XResultSet>();
    m_xMetaData     = nullptr;
    m_aUpdateTable  = nullptr;
}

void ORowSetCache::setFetchSize(sal_Int32 _nSize)
{
    if(_nSize == m_nFetchSize)
        return;

    m_nFetchSize = _nSize;
    if(!m_pMatrix)
    {
        m_pMatrix.reset( new ORowSetMatrix(_nSize) );
        m_aMatrixIter = m_pMatrix->end();
        m_aMatrixEnd = m_pMatrix->end();

        m_pInsertMatrix.reset( new ORowSetMatrix(1) ); // a little bit overkill but ??? :-)
        m_aInsertRow    = m_pInsertMatrix->end();
    }
    else
    {
        // now correct the iterator in our iterator vector
        std::vector<sal_Int32> aPositions;
        std::map<sal_Int32,bool> aCacheIterToChange;
        // first get the positions where they stand now
        for(const auto& [rIndex, rHelper] : m_aCacheIterators)
        {
            aCacheIterToChange[rIndex] = false;
            if ( !rHelper.pRowSet->isInsertRow()
                /*&& rHelper.aIterator != m_pMatrix->end()*/ && !m_bModified )
            {
                ptrdiff_t nDist = rHelper.aIterator - m_pMatrix->begin();
                aPositions.push_back(nDist);
                aCacheIterToChange[rIndex] = true;
            }
        }
        sal_Int32 nKeyPos = m_aMatrixIter - m_pMatrix->begin();
        m_pMatrix->resize(_nSize);

        if ( nKeyPos < _nSize )
            m_aMatrixIter = m_pMatrix->begin() + nKeyPos;
        else
            m_aMatrixIter = m_pMatrix->end();
        m_aMatrixEnd = m_pMatrix->end();

        // now adjust their positions because a resize invalidates all iterators
        std::vector<sal_Int32>::const_iterator aIter = aPositions.begin();
        ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
        for(const auto& rPosChange : aCacheIterToChange)
        {
            if ( rPosChange.second )
            {
                OSL_ENSURE((*aIter >= static_cast<ORowSetMatrix::difference_type>(0)) && (*aIter < static_cast<sal_Int32>(m_pMatrix->size())),"Position is invalid!");
                if ( *aIter < _nSize )
                    aCacheIter->second.aIterator = m_pMatrix->begin() + *aIter++;
                else
                    aCacheIter->second.aIterator = m_pMatrix->end();
            }
            ++aCacheIter;
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
        // try to repair
        moveWindow();
        m_aMatrixIter = calcPosition();
    }
}

// XResultSetMetaDataSupplier

static Any lcl_getBookmark(ORowSetValue& i_aValue,OCacheSet* i_pCacheSet)
{
    switch ( i_aValue.getTypeKind() )
    {
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
            return Any(i_aValue.getInt32());
        default:
            if ( i_pCacheSet && i_aValue.isNull())
                i_aValue = i_pCacheSet->getBookmark();
            return i_aValue.getAny();
    }
}

// css::sdbcx::XRowLocate
Any ORowSetCache::getBookmark(  )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    if ( m_aMatrixIter >= m_pMatrix->end() || m_aMatrixIter < m_pMatrix->begin() || !(*m_aMatrixIter).is())
    {
        return Any(); // this is allowed here because the rowset knows what it is doing
    }

    return lcl_getBookmark((**m_aMatrixIter)[0],m_xCacheSet.get());
}

bool ORowSetCache::moveToBookmark( const Any& bookmark )
{
    if ( m_xCacheSet->moveToBookmark(bookmark) )
    {
        m_bBeforeFirst = false;
        m_nPosition = m_xCacheSet->getRow();

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
        return false;

    return m_aMatrixIter != m_pMatrix->end() && (*m_aMatrixIter).is();
}

bool ORowSetCache::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows )
{
    bool bRet( moveToBookmark( bookmark ) );
    if ( bRet )
    {
        m_nPosition = m_xCacheSet->getRow() + rows;
        absolute(m_nPosition);

        bRet = m_aMatrixIter != m_pMatrix->end() && (*m_aMatrixIter).is();
    }

    return bRet;
}

sal_Int32 ORowSetCache::compareBookmarks( const Any& _first, const Any& _second )
{
    return (!_first.hasValue() || !_second.hasValue()) ? CompareBookmark::NOT_COMPARABLE : m_xCacheSet->compareBookmarks(_first,_second);
}

bool ORowSetCache::hasOrderedBookmarks(  )
{
    return m_xCacheSet->hasOrderedBookmarks();
}

sal_Int32 ORowSetCache::hashBookmark( const Any& bookmark )
{
    return m_xCacheSet->hashBookmark(bookmark);
}

// XRowUpdate
void ORowSetCache::updateNull(sal_Int32 columnIndex,ORowSetValueVector::Vector& io_aRow
                              ,std::vector<sal_Int32>& o_ChangedColumns
                              )
{
    checkUpdateConditions(columnIndex);

    ORowSetValueVector::Vector& rInsert = **m_aInsertRow;
    if ( !rInsert[columnIndex].isNull() )
    {
        rInsert[columnIndex].setBound(true);
        rInsert[columnIndex].setNull();
        rInsert[columnIndex].setModified(true);
        io_aRow[columnIndex].setNull();

        m_xCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}

void ORowSetCache::updateValue(sal_Int32 columnIndex,const ORowSetValue& x
                               ,ORowSetValueVector::Vector& io_aRow
                               ,std::vector<sal_Int32>& o_ChangedColumns
                               )
{
    checkUpdateConditions(columnIndex);

    ORowSetValueVector::Vector& rInsert = **m_aInsertRow;
    if ( rInsert[columnIndex] != x )
    {
        rInsert[columnIndex].setBound(true);
        rInsert[columnIndex] = x;
        rInsert[columnIndex].setModified(true);
        io_aRow[columnIndex] = rInsert[columnIndex];

        m_xCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}

void ORowSetCache::updateCharacterStream( sal_Int32 columnIndex, const Reference< css::io::XInputStream >& x
                                         , sal_Int32 length,ORowSetValueVector::Vector& io_aRow
                                         ,std::vector<sal_Int32>& o_ChangedColumns
                                         )
{
    checkUpdateConditions(columnIndex);

    Sequence<sal_Int8> aSeq;
    if(x.is())
        x->readBytes(aSeq,length);

    ORowSetValueVector::Vector& rInsert = **m_aInsertRow;
    rInsert[columnIndex].setBound(true);
    rInsert[columnIndex] = aSeq;
    rInsert[columnIndex].setModified(true);
    io_aRow[columnIndex] = Any(x);

    m_xCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
    impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
}

void ORowSetCache::updateObject( sal_Int32 columnIndex, const Any& x
                                ,ORowSetValueVector::Vector& io_aRow
                                ,std::vector<sal_Int32>& o_ChangedColumns
                                )
{
    checkUpdateConditions(columnIndex);

    ORowSetValueVector::Vector& rInsert = **m_aInsertRow;
    ORowSetValue aTemp;
    aTemp.fill(x);
    if ( rInsert[columnIndex] != aTemp )
    {
        rInsert[columnIndex].setBound(true);
        rInsert[columnIndex] = std::move(aTemp);
        rInsert[columnIndex].setModified(true);
        io_aRow[columnIndex] = rInsert[columnIndex];

        m_xCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}

void ORowSetCache::updateNumericObject( sal_Int32 columnIndex, const Any& x
                                       ,ORowSetValueVector::Vector& io_aRow
                                       ,std::vector<sal_Int32>& o_ChangedColumns
                                       )
{
    checkUpdateConditions(columnIndex);

    ORowSetValueVector::Vector& rInsert = **m_aInsertRow;
    ORowSetValue aTemp;
    aTemp.fill(x);
    if ( rInsert[columnIndex] != aTemp )
    {
        rInsert[columnIndex].setBound(true);
        rInsert[columnIndex] = std::move(aTemp);
        rInsert[columnIndex].setModified(true);
        io_aRow[columnIndex] = rInsert[columnIndex];

        m_xCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}

// XResultSet
bool ORowSetCache::next(  )
{
    if(!isAfterLast())
    {
        m_bBeforeFirst = false;
        ++m_nPosition;

        // after we increment the position we have to check if we are already after the last row
        checkPositionFlags();
        if(!m_bAfterLast)
        {
            moveWindow();

            OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < static_cast<sal_Int32>(m_pMatrix->size()),"Position is behind end()!");
            m_aMatrixIter = calcPosition();
            checkPositionFlags();
        }
    }

    return !m_bAfterLast;
}


bool ORowSetCache::isFirst(  ) const
{
    return m_nPosition == 1; // ask resultset for
}

bool ORowSetCache::isLast(  ) const
{
    return m_nPosition == m_nRowCount;
}

void ORowSetCache::beforeFirst(  )
{
    if(!m_bBeforeFirst)
    {
        m_bAfterLast    = false;
        m_nPosition     = 0;
        m_bBeforeFirst  = true;
        m_xCacheSet->beforeFirst();
        moveWindow();
        m_aMatrixIter = m_pMatrix->end();
    }
}

void ORowSetCache::afterLast(  )
{
    if(m_bAfterLast)
        return;

    m_bBeforeFirst = false;
    m_bAfterLast = true;

    if(!m_bRowCountFinal)
    {
        m_xCacheSet->last();
        m_bRowCountFinal = true;
        m_nRowCount = m_xCacheSet->getRow();// + 1 removed
    }
    m_xCacheSet->afterLast();

    m_nPosition = 0;
    m_aMatrixIter = m_pMatrix->end();
}

bool ORowSetCache::fillMatrix(sal_Int32& _nNewStartPos, sal_Int32 &_nNewEndPos)
{
    OSL_ENSURE((_nNewStartPos != _nNewEndPos) || (_nNewStartPos == 0 && _nNewEndPos == 0 && m_nRowCount == 0),
               "ORowSetCache::fillMatrix: StartPos and EndPos can not be equal (unless the recordset is empty)!");
    // If _nNewStartPos >= 0, then fill the whole window with new data
    // Else if _nNewStartPos == -1, then fill only segment [m_nEndPos, _nNewEndPos)
    // Else, undefined (invalid argument)
    OSL_ENSURE( _nNewStartPos >= -1, "ORowSetCache::fillMatrix: invalid _nNewStartPos" );

    ORowSetMatrix::iterator aIter;
    sal_Int32 i;
    bool bCheck;
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
    bCheck = m_xCacheSet->absolute(i);


    for(; i <= _nNewEndPos; ++i,++aIter)
    {
        if(bCheck)
        {
            if(!aIter->is())
                *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
            m_xCacheSet->fillValueRow(*aIter,i);
        }
        else
        {   // there are no more rows found so we can fetch some before start

            if(!m_bRowCountFinal)
            {
                if(m_xCacheSet->previous()) // because we stand after the last row
                    m_nRowCount = m_xCacheSet->getRow(); // here we have the row count
                if(!m_nRowCount)
                    m_nRowCount = i-1; // it can be that getRow return zero
                m_bRowCountFinal = true;
            }
            const ORowSetMatrix::iterator aEnd = aIter;
            ORowSetMatrix::const_iterator aRealEnd = m_pMatrix->end();
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
            bCheck = m_xCacheSet->absolute(nPos);

            for(;bCheck && nPos <= requestedStartPos && aIter != aRealEnd; ++aIter, ++nPos)
            {
                if(!aIter->is())
                    *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                m_xCacheSet->fillValueRow(*aIter, nPos);
                bCheck = m_xCacheSet->next();
            }
            if(aIter != aEnd)
                std::rotate(m_pMatrix->begin(),aEnd,aIter);
            break;
        }
        bCheck = m_xCacheSet->next();
    }
    // we have to read one row forward to ensure that we know when we are on last row
    // but only when we don't know it already
    if(!m_bRowCountFinal)
    {
        if(!m_xCacheSet->next())
        {
            if(m_xCacheSet->previous()) // because we stand after the last row
                m_nRowCount = m_xCacheSet->getRow(); // here we have the row count
            m_bRowCountFinal = true;
        }
        else
           m_nRowCount = std::max(i,m_nRowCount);

    }
    return bCheck;
}

// If m_nPosition is out of the current window,
// move it and update m_nStartPos and m_nEndPos
// Caller is responsible for updating m_aMatrixIter
void ORowSetCache::moveWindow()
{
    OSL_ENSURE(m_nStartPos >= 0,"ORowSetCache::moveWindow: m_nStartPos is less than 0!");
    OSL_ENSURE(m_nEndPos >= m_nStartPos,"ORowSetCache::moveWindow: m_nStartPos not smaller than m_nEndPos");
    OSL_ENSURE(m_nEndPos-m_nStartPos <= m_nFetchSize,"ORowSetCache::moveWindow: m_nStartPos and m_nEndPos too far apart");

    if ( m_nStartPos < m_nPosition && m_nPosition <= m_nEndPos )
    {
        // just move inside the window
        OSL_ENSURE((m_nPosition - m_nStartPos) <= static_cast<sal_Int32>(m_pMatrix->size()),"Position is behind end()!");
        // make double plus sure that we have fetched that row
        m_aMatrixIter = calcPosition();
        OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(), "New m_aMatrixIter is at end(), but should not.");
        if(!m_aMatrixIter->is())
        {
            bool bOk( m_xCacheSet->absolute( m_nPosition ) );
            if ( bOk )
            {
                *m_aMatrixIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                m_xCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
                // we have to read one row forward to ensure that we know when we are on last row
                // but only when we don't know it already
                if ( !m_bRowCountFinal )
                {
                    bOk = m_xCacheSet->absolute( m_nPosition + 1 );
                    if ( bOk )
                        m_nRowCount = std::max(sal_Int32(m_nPosition+1),m_nRowCount);
                }
            }
            if(!bOk && !m_bRowCountFinal)
            {
                // because we stand after the last row
                m_nRowCount = m_xCacheSet->previous() ? m_xCacheSet->getRow() : 0;
                m_bRowCountFinal = true;
            }
        }
        return;
    }

    sal_Int32 nDiff = (m_nFetchSize - 1) / 2;
    sal_Int32 nNewStartPos  = (m_nPosition - nDiff) - 1; //m_nPosition is 1-based, but m_nStartPos is 0-based
    sal_Int32 nNewEndPos    = nNewStartPos + m_nFetchSize;

    if ( nNewStartPos < 0 )
    {
        // The computed new window crashes through the floor (begins before first row);
        // nNew*Pos has to be shifted by -nNewStartPos
        nNewEndPos -= nNewStartPos;
        nNewStartPos = 0;
    }

    if ( nNewStartPos < m_nStartPos )
    {   // need to fill data *before* m_nStartPos
        if ( nNewEndPos > m_nStartPos )
        {   // The two regions are overlapping.
            // We'll first rotate the contents of m_pMatrix so that the overlap area
            // is positioned right; in the old window it is at the beginning,
            // it has to go to the end.
            // then we fill in the rows between new and old start pos.

            bool bCheck;
            bCheck = m_xCacheSet->absolute(nNewStartPos + 1);

            // m_nEndPos < nNewEndPos when window not filled (e.g. there are fewer rows in total than window size)
            m_nEndPos = std::min(nNewEndPos, m_nEndPos);
            const sal_Int32 nOverlapSize = m_nEndPos - m_nStartPos;
            const sal_Int32 nStartPosOffset = m_nStartPos - nNewStartPos; // by how much m_nStartPos moves
            m_nStartPos = nNewStartPos;
            OSL_ENSURE( o3tl::make_unsigned(nOverlapSize) <= m_pMatrix->size(), "new window end is after end of cache matrix!" );
            // the first position in m_pMatrix whose data we don't keep;
            // content will be moved to m_pMatrix.begin()
            ORowSetMatrix::iterator aEnd (m_pMatrix->begin() + nOverlapSize);
            // the first unused position after we are done; it == m_pMatrix.end() if and only if the window is full
            ORowSetMatrix::iterator aNewEnd (aEnd + nStartPosOffset);
            // *m_pMatrix now looks like:
            //   [0; nOverlapSize) i.e. [begin(); aEnd): data kept
            //   [nOverlapSize; nOverlapSize + nStartPosOffset) i.e. [aEnd, aNewEnd): new data of positions < old m_nStartPos
            //   [nOverlapSize + nStartPosOffset; size()) i.e. [aNewEnd, end()): unused
            // Note that nOverlapSize + nStartPosOffset == m_nEndPos - m_nStartPos (new values)
            // When we are finished:
            //   [0; nStartPosOffset) i.e. [begin(); aEnd): new data of positions < old m_nStartPos
            //   [nStartPosOffset; nOverlapSize + nStartPosOffset) i.e. [aEnd, aNewEnd): kept
            //   [nOverlapSize + nStartPosOffset; size()) i.e. [aNewEnd, end()): unused

            if ( bCheck )
            {
                {
                    ORowSetMatrix::iterator aIter(aEnd);
                    sal_Int32 nPos = m_nStartPos + 1;
                    fill(aIter, aNewEnd, nPos, bCheck);
                }

                std::rotate(m_pMatrix->begin(), aEnd, aNewEnd);
                if (!m_bModified)
                {
                    // now correct the iterator in our iterator vector
                    //  rotateCacheIterator(aEnd-m_pMatrix->begin()); //can't be used because they decrement and here we need to increment
                    for(auto& rCacheIter : m_aCacheIterators)
                    {
                        if ( !rCacheIter.second.pRowSet->isInsertRow()
                            && rCacheIter.second.aIterator != m_pMatrix->end() )
                        {
                            const ptrdiff_t nDist = rCacheIter.second.aIterator - m_pMatrix->begin();
                            if ( nDist >= nOverlapSize )
                            {
                                // That's from outside the overlap area; invalidate iterator.
                                rCacheIter.second.aIterator = m_pMatrix->end();
                            }
                            else
                            {
                                // Inside overlap area: move to correct position
                                OSL_ENSURE(((nDist + nStartPosOffset) >= static_cast<ORowSetMatrix::difference_type>(0)) &&
                                    ((nDist + nStartPosOffset) < static_cast<sal_Int32>(m_pMatrix->size())),"Position is invalid!");
                                rCacheIter.second.aIterator += nStartPosOffset;
                                OSL_ENSURE(rCacheIter.second.aIterator >= m_pMatrix->begin()
                                    && rCacheIter.second.aIterator < m_pMatrix->end(),"Iterator out of area!");
                            }
                        }
                    }
                }
            }
            else
            { // normally this should never happen
                OSL_FAIL("What the hell is happen here!");
                return;
            }
        }
        else
        {// no rows can be reused so fill again
            reFillMatrix(nNewStartPos,nNewEndPos);
        }
    }

    OSL_ENSURE(nNewStartPos >= m_nStartPos, "ORowSetCache::moveWindow internal error: new start pos before current start pos");
    if ( m_nEndPos < nNewEndPos )
    {   // need to fill data *after* m_nEndPos
        if( nNewStartPos < m_nEndPos )
        {   // The two regions are overlapping.
            const sal_Int32 nRowsInCache = m_nEndPos - m_nStartPos;
            if ( nRowsInCache < m_nFetchSize )
            {
                // There is some unused space in *m_pMatrix; fill it
                OSL_ENSURE((nRowsInCache >= static_cast<ORowSetMatrix::difference_type>(0)) && (o3tl::make_unsigned(nRowsInCache) < m_pMatrix->size()),"Position is invalid!");
                sal_Int32 nPos = m_nEndPos + 1;
                bool bCheck = m_xCacheSet->absolute(nPos);
                ORowSetMatrix::iterator aIter = m_pMatrix->begin() + nRowsInCache;
                const sal_Int32 nRowsToFetch = std::min(nNewEndPos-m_nEndPos, m_nFetchSize-nRowsInCache);
                const ORowSetMatrix::const_iterator aEnd = aIter + nRowsToFetch;
                bCheck = fill(aIter, aEnd, nPos, bCheck);
                m_nEndPos = nPos - 1;
                OSL_ENSURE( (!bCheck && m_nEndPos <= nNewEndPos ) ||
                            ( bCheck && m_nEndPos == nNewEndPos ),
                             "ORowSetCache::moveWindow opportunistic fetch-after-current-end went badly");
            }

            // A priori, the rows from begin() [inclusive] to (begin() + nNewStartPos - m_nStartPos) [exclusive]
            // have to be refilled with new to-be-fetched rows.
            // The rows behind this can be reused
            ORowSetMatrix::iterator aIter = m_pMatrix->begin();
            const sal_Int32 nNewStartPosInMatrix = nNewStartPos - m_nStartPos;
            OSL_ENSURE((nNewStartPosInMatrix >= static_cast<ORowSetMatrix::difference_type>(0)) && (o3tl::make_unsigned(nNewStartPosInMatrix) < m_pMatrix->size()),"Position is invalid!");
            // first position we reuse
            const ORowSetMatrix::const_iterator aEnd  = m_pMatrix->begin() + nNewStartPosInMatrix;
            // End of used portion of the matrix. Is < m_pMatrix->end() if less data than window size
            ORowSetMatrix::iterator aDataEnd  = m_pMatrix->begin() + (m_nEndPos - m_nStartPos);

            sal_Int32 nPos = m_nEndPos + 1;
            bool bCheck = m_xCacheSet->absolute(nPos);
            bCheck = fill(aIter, aEnd, nPos, bCheck); // refill the region we don't need anymore
            //aIter and nPos are now the position *after* last filled in one!

            // bind end to front
            if(bCheck)
            {
                OSL_ENSURE(aIter == aEnd, "fill() said went till end, but did not.");
                // rotate the end to the front
                std::rotate(m_pMatrix->begin(), aIter, aDataEnd);
                // now correct the iterator in our iterator vector
                rotateCacheIterator( nNewStartPosInMatrix );
                m_nStartPos = nNewStartPos;
                m_nEndPos = nNewEndPos;
                // now I can say how many rows we have
                // we have to read one row forward to ensure that we know when we are on last row
                // but only when we don't know it already
                bool bOk = true;
                if(!m_bRowCountFinal)
                    bOk = m_xCacheSet->next();
                if(!bOk)
                {
                    m_xCacheSet->previous(); // because we stand after the last row
                    m_nRowCount      = nPos; // here we have the row count
                    OSL_ENSURE(nPos == m_xCacheSet->getRow(),"nPos is not valid!");
                    m_bRowCountFinal = true;
                }
                else if(!m_bRowCountFinal)
                    m_nRowCount = std::max(nPos+1, m_nRowCount); //+1 because we successfully moved to row after nPos
                else
                    OSL_ENSURE(m_nRowCount >= nPos, "Final m_nRowCount is smaller than row I moved to!");
            }
            else
            {   // the end was reached before or at end() so we can set the start before or at nNewStartPos
                // and possibly keep more of m_pMatrix than planned.
                const ORowSetMatrix::const_iterator::difference_type nFetchedRows  = aIter - m_pMatrix->begin();
                // *m_pMatrix now looks like:
                // [0; nFetchedRows) i.e. [begin(); aIter): newly fetched data for positions m_nEndPos to m_nEndPos+nFetchedRows
                // [nFetchedRows; ???) i.e. [aIter; aDataEnd]: data to be kept for positions m_nStartPos+nFetchedRows to ???

                nPos -= 1;
                m_nStartPos += nFetchedRows;
                m_nEndPos = nPos;
                std::rotate(m_pMatrix->begin(), aIter, aDataEnd);
                // now correct the iterator in our iterator vector
                rotateCacheIterator( nFetchedRows );

                if ( !m_bRowCountFinal )
                {
                    m_xCacheSet->previous();                   // because we stand after the last row
                    m_nRowCount      = std::max(m_nRowCount, nPos);    // here we have the row count
                    OSL_ENSURE(nPos == m_xCacheSet->getRow(),"nPos isn't valid!");
                    m_bRowCountFinal = true;
                }

            }
            // here we need only to check if the beginning row is valid. If not we have to fetch it.
            if(!m_pMatrix->begin()->is())
            {
                aIter = m_pMatrix->begin();

                nPos    = m_nStartPos + 1;
                bCheck  = m_xCacheSet->absolute(nPos);
                for(; !aIter->is() && bCheck;++aIter, ++nPos)
                {
                    OSL_ENSURE(aIter != m_pMatrix->end(),"Invalid iterator");

                    *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                    m_xCacheSet->fillValueRow(*aIter, nPos);

                    bCheck = m_xCacheSet->next();
                }
            }
        }
        else // no rows can be reused so fill again
            reFillMatrix(nNewStartPos,nNewEndPos);
    }

    if(!m_bRowCountFinal)
       m_nRowCount = std::max(m_nPosition,m_nRowCount);
    OSL_ENSURE(m_nStartPos >= 0,"ORowSetCache::moveWindow: m_nStartPos is less than 0!");
    OSL_ENSURE(m_nEndPos > m_nStartPos,"ORowSetCache::moveWindow: m_nStartPos not smaller than m_nEndPos");
    OSL_ENSURE(m_nEndPos-m_nStartPos <= m_nFetchSize,"ORowSetCache::moveWindow: m_nStartPos and m_nEndPos too far apart");
}

bool ORowSetCache::first(  )
{
    // First move to the first row.
    // Then check if the cache window is at the beginning.
    // If not, then position the window and fill it with data.
    // We move the window smartly, i.e. we clear only the rows that are out of range
    bool bRet = m_xCacheSet->first();
    if(bRet)
    {
        m_bBeforeFirst  = m_bAfterLast = false;
        m_nPosition     = 1;
        moveWindow();
        m_aMatrixIter   = m_pMatrix->begin();
    }
    else
    {
        m_bRowCountFinal = m_bBeforeFirst = m_bAfterLast = true;
        m_nRowCount = m_nPosition = 0;

        OSL_ENSURE(m_bBeforeFirst || m_bNew,"ORowSetCache::first return false and BeforeFirst isn't true");
        m_aMatrixIter = m_pMatrix->end();
    }
    return bRet;
}

bool ORowSetCache::last(  )
{
    bool bRet = m_xCacheSet->last();
    if(bRet)
    {
        m_bBeforeFirst = m_bAfterLast = false;
        if(!m_bRowCountFinal)
        {
            m_bRowCountFinal = true;
            m_nRowCount = m_xCacheSet->getRow(); // not  + 1
        }
        m_nPosition = m_xCacheSet->getRow();
        moveWindow();
        // we have to repositioning because moveWindow can modify the cache
        m_xCacheSet->last();
        OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < static_cast<sal_Int32>(m_pMatrix->size()),"Position is behind end()!");
        m_aMatrixIter = calcPosition();
    }
    else
    {
        m_bRowCountFinal = m_bBeforeFirst = m_bAfterLast = true;
        m_nRowCount = m_nPosition = 0;
        OSL_ENSURE(m_bBeforeFirst,"ORowSetCache::last return false and BeforeFirst isn't true");
        m_aMatrixIter = m_pMatrix->end();
    }
#if OSL_DEBUG_LEVEL > 0
    if(bRet)
    {
        assert((*m_aMatrixIter).is() && "ORowSetCache::last: Row not valid!");
    }
#endif

    return bRet;
}

sal_Int32 ORowSetCache::getRow(  ) const
{
    return (isBeforeFirst() || isAfterLast()) ? 0 : m_nPosition;
}

bool ORowSetCache::absolute( sal_Int32 row )
{
    if(!row )
        throw SQLException(DBA_RES(RID_STR_NO_ABS_ZERO),nullptr,SQLSTATE_GENERAL,1000,Any() );

    if(row < 0)
    {
        // here we have to scroll from the last row to backward so we have to go to last row and
        // and to the previous
        if(m_bRowCountFinal || last())
        {
            m_nPosition = m_nRowCount + row + 1; // + row because row is negative and +1 because row==-1 means last row
            if(m_nPosition < 1)
            {
                m_bBeforeFirst = true;
                m_bAfterLast = false;
                m_aMatrixIter = m_pMatrix->end();
            }
            else
            {
                m_bBeforeFirst  = false;
                m_bAfterLast    = m_nPosition > m_nRowCount;
                moveWindow();
                OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < static_cast<sal_Int32>(m_pMatrix->size()),"Position is behind end()!");
                m_aMatrixIter = calcPosition();
            }
        }
        else
            m_aMatrixIter = m_pMatrix->end();
    }
    else
    {
        m_nPosition = row;
        // the position flags
        m_bBeforeFirst  = false;
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

bool ORowSetCache::relative( sal_Int32 rows )
{
    bool bErg = true;
    if(rows)
    {
        sal_Int32 nNewPosition = m_nPosition + rows;

        if ( m_bBeforeFirst && rows > 0 )
            nNewPosition = rows;
        else if ( m_bRowCountFinal && m_bAfterLast && rows < 0 )
            nNewPosition = m_nRowCount + 1 + rows;
        else
            if ( m_bBeforeFirst || ( m_bRowCountFinal && m_bAfterLast ) )
                throw SQLException( DBA_RES( RID_STR_NO_RELATIVE ), nullptr, SQLSTATE_GENERAL, 1000, Any() );
        if ( nNewPosition )
        {
            bErg = absolute( nNewPosition );
            bErg = bErg && !isAfterLast() && !isBeforeFirst();
        }
        else
        {
            m_bBeforeFirst = true;
            bErg = false;
        }
    }
    return bErg;
}

bool ORowSetCache::previous(  )
{
    bool bRet = false;
    if(!isBeforeFirst())
    {
        if(m_bAfterLast)   // we stand after the last row so one before is the last row
            bRet = last();
        else
        {
            m_bAfterLast = false;
            --m_nPosition;
            moveWindow();
            OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < static_cast<sal_Int32>(m_pMatrix->size()),"Position is behind end()!");

            checkPositionFlags();

            if(!m_nPosition)
            {
                m_bBeforeFirst = true;
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
        throw SQLException(DBA_RES(RID_STR_NO_REFRESH_AFTERLAST),nullptr,SQLSTATE_GENERAL,1000,Any() );
    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"refreshRow() called for invalid row!");
    m_xCacheSet->refreshRow();
    m_xCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
    if ( m_bNew )
    {
        cancelRowModification();
    }
}

bool ORowSetCache::rowUpdated(  )
{
    return m_xCacheSet->rowUpdated();
}

bool ORowSetCache::rowInserted(  )
{
    return m_xCacheSet->rowInserted();
}

// XResultSetUpdate
bool ORowSetCache::insertRow(std::vector< Any >& o_aBookmarks)
{
    if ( !m_bNew || !m_aInsertRow->is() )
        throw SQLException(DBA_RES(RID_STR_NO_MOVETOINSERTROW_CALLED),nullptr,SQLSTATE_GENERAL,1000,Any() );

    m_xCacheSet->insertRow(*m_aInsertRow,m_aUpdateTable);

    bool bRet( rowInserted() );
    if ( bRet )
    {
        ++m_nRowCount;
        Any aBookmark = (**m_aInsertRow)[0].makeAny();
        m_bAfterLast = m_bBeforeFirst = false;
        if(aBookmark.hasValue())
        {
            moveToBookmark(aBookmark);
            // update the cached values
            ORowSetValueVector::Vector& rCurrentRow = **m_aMatrixIter;
            ORowSetMatrix::const_iterator aIter = m_pMatrix->begin();
            for(;aIter != m_pMatrix->end();++aIter)
            {
                if ( m_aMatrixIter != aIter && aIter->is() && m_xCacheSet->columnValuesUpdated(**aIter,rCurrentRow) )
                {
                    o_aBookmarks.push_back(lcl_getBookmark((**aIter)[0], m_xCacheSet.get()));
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

void ORowSetCache::resetInsertRow(bool _bClearInsertRow)
{
    if ( _bClearInsertRow )
        clearInsertRow();
    m_bNew      = false;
    m_bModified = false;
}

void ORowSetCache::cancelRowModification()
{
    // clear the insertrow references   -> implies that the current row of the rowset changes as well
    for(auto& rCacheIter : m_aCacheIterators)
    {
        if ( rCacheIter.second.pRowSet->isInsertRow() && rCacheIter.second.aIterator == m_aInsertRow )
            rCacheIter.second.aIterator = m_pMatrix->end();
    }
    resetInsertRow(false);
}

void ORowSetCache::updateRow( ORowSetMatrix::iterator const & _rUpdateRow, std::vector< Any >& o_aBookmarks )
{
    if(isAfterLast() || isBeforeFirst())
        throw SQLException(DBA_RES(RID_STR_NO_UPDATEROW),nullptr,SQLSTATE_GENERAL,1000,Any() );

    Any aBookmark = (**_rUpdateRow)[0].makeAny();
    OSL_ENSURE(aBookmark.hasValue(),"Bookmark must have a value!");
    // here we don't have to reposition our CacheSet, when we try to update a row,
    // the row was already fetched
    moveToBookmark(aBookmark);
    m_xCacheSet->updateRow(*_rUpdateRow,*m_aMatrixIter,m_aUpdateTable);
    // refetch the whole row
    (*m_aMatrixIter) = nullptr;

    if ( moveToBookmark(aBookmark) )
    {
        // update the cached values
        ORowSetValueVector::Vector& rCurrentRow = **m_aMatrixIter;
        ORowSetMatrix::const_iterator aIter = m_pMatrix->begin();
        for(;aIter != m_pMatrix->end();++aIter)
        {
            if ( m_aMatrixIter != aIter && aIter->is() && m_xCacheSet->columnValuesUpdated(**aIter,rCurrentRow) )
            {
                o_aBookmarks.push_back(lcl_getBookmark((**aIter)[0], m_xCacheSet.get()));
            }
        }
    }

    m_bModified = false;
}

bool ORowSetCache::deleteRow(  )
{
    if(isAfterLast() || isBeforeFirst())
        throw SQLException(DBA_RES(RID_STR_NO_DELETEROW),nullptr,SQLSTATE_GENERAL,1000,Any() );

    m_xCacheSet->deleteRow(*m_aMatrixIter,m_aUpdateTable);
    if ( !m_xCacheSet->rowDeleted() )
        return false;

    --m_nRowCount;
    OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < static_cast<sal_Int32>(m_pMatrix->size()),"Position is behind end()!");
    ORowSetMatrix::iterator aPos = calcPosition();
    (*aPos)   = nullptr;

    ORowSetMatrix::const_iterator aEnd = m_pMatrix->end();
    for(++aPos;aPos != aEnd && aPos->is();++aPos)
    {
        *(aPos-1) = *aPos;
        (*aPos)   = nullptr;
    }
    m_aMatrixIter = m_pMatrix->end();

    --m_nPosition;
    return true;
}

void ORowSetCache::cancelRowUpdates(  )
{
    m_bNew = m_bModified = false;
    if(!m_nPosition)
    {
        OSL_FAIL("cancelRowUpdates:Invalid positions pos == 0");
        ::dbtools::throwFunctionSequenceException(nullptr);
    }

    if(m_xCacheSet->absolute(m_nPosition))
        m_xCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
    else
    {
        OSL_FAIL("cancelRowUpdates couldn't position right with absolute");
        ::dbtools::throwFunctionSequenceException(nullptr);
    }
}

void ORowSetCache::moveToInsertRow(  )
{
    m_bNew      = true;
    m_bAfterLast = false;

    m_aInsertRow = m_pInsertMatrix->begin();
    if(!m_aInsertRow->is())
        *m_aInsertRow = new ORowSetValueVector(m_xMetaData->getColumnCount());

    // we don't unbound the bookmark column
    ORowSetValueVector::Vector::iterator aIter = (*m_aInsertRow)->begin()+1;
    ORowSetValueVector::Vector::const_iterator aEnd = (*m_aInsertRow)->end();
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
    ORowSetCacheMap::const_iterator aCacheIter = m_aCacheIterators.begin();
    for(;aCacheIter != m_aCacheIterators.end();)
    {
        if ( aCacheIter->second.pRowSet == _pRowSet )
        {
            aCacheIter = m_aCacheIterators.erase(aCacheIter);
        }
        else
            ++aCacheIter;
    }
}

void ORowSetCache::rotateCacheIterator(ORowSetMatrix::difference_type _nDist)
{
    if (m_bModified)
        return;

    if(!_nDist)
        return;

    // now correct the iterator in our iterator vector
    for(auto& rCacheIter : m_aCacheIterators)
    {
        if ( !rCacheIter.second.pRowSet->isInsertRow()
            && rCacheIter.second.aIterator != m_pMatrix->end())
        {
            ptrdiff_t nDist = rCacheIter.second.aIterator - m_pMatrix->begin();
            if(nDist < _nDist)
            {
                rCacheIter.second.aIterator = m_pMatrix->end();
            }
            else
            {
                OSL_ENSURE((rCacheIter.second.aIterator - m_pMatrix->begin()) >= _nDist,"Invalid Dist value!");
                rCacheIter.second.aIterator -= _nDist;
                OSL_ENSURE(rCacheIter.second.aIterator >= m_pMatrix->begin()
                        && rCacheIter.second.aIterator < m_pMatrix->end(),"Iterator out of area!");
            }
        }
    }
}

void ORowSetCache::rotateAllCacheIterators()
{
    if (m_bModified)
        return;

    // now correct the iterator in our iterator vector
    for (auto& rCacheIter : m_aCacheIterators)
    {
        if (!rCacheIter.second.pRowSet->isInsertRow())
        {
            rCacheIter.second.aIterator = m_pMatrix->end();
        }
    }
}

void ORowSetCache::setUpdateIterator(const ORowSetMatrix::iterator& _rOriginalRow)
{
    m_aInsertRow = m_pInsertMatrix->begin();
    if(!m_aInsertRow->is())
        *m_aInsertRow = new ORowSetValueVector(m_xMetaData->getColumnCount());

    (*(*m_aInsertRow)) = *(*_rOriginalRow);
    // we don't unbound the bookmark column
    for(auto& rItem : **m_aInsertRow)
        rItem.setModified(false);
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
    if(m_bAfterLast || columnIndex >= static_cast<sal_Int32>((*m_aInsertRow)->size()))
        throwFunctionSequenceException(m_xSet.get());
}

bool ORowSetCache::checkInnerJoin(const ::connectivity::OSQLParseNode *pNode,const Reference< XConnection>& _xConnection,const OUString& _sUpdateTableName)
{
    bool bOk = false;
    if (pNode->count() == 3 &&  // expression in parentheses
        SQL_ISPUNCTUATION(pNode->getChild(0),"(") &&
        SQL_ISPUNCTUATION(pNode->getChild(2),")"))
    {
        bOk = checkInnerJoin(pNode->getChild(1),_xConnection,_sUpdateTableName);
    }
    else if ((SQL_ISRULE(pNode,search_condition) || SQL_ISRULE(pNode,boolean_term)) && // AND/OR link
                pNode->count() == 3)
    {
        // only allow an AND link
        if ( SQL_ISTOKEN(pNode->getChild(1),AND) )
            bOk = checkInnerJoin(pNode->getChild(0),_xConnection,_sUpdateTableName)
                && checkInnerJoin(pNode->getChild(2),_xConnection,_sUpdateTableName);
    }
    else if (SQL_ISRULE(pNode,comparison_predicate))
    {
        // only the comparison of columns is allowed
        OSL_ENSURE(pNode->count() == 3,"checkInnerJoin: Error in Parse Tree");
        if (!(SQL_ISRULE(pNode->getChild(0),column_ref) &&
                SQL_ISRULE(pNode->getChild(2),column_ref) &&
                pNode->getChild(1)->getNodeType() == SQLNodeType::Equal))
        {
            bOk = false;
        }
        else
        {
            OUString sColumnName,sTableRange;
            OSQLParseTreeIterator::getColumnRange( pNode->getChild(0), _xConnection, sColumnName, sTableRange );
            bOk = sTableRange == _sUpdateTableName;
            if ( !bOk )
            {
                OSQLParseTreeIterator::getColumnRange( pNode->getChild(2), _xConnection, sColumnName, sTableRange );
                bOk =  sTableRange == _sUpdateTableName;
            }
        }
    }
    return bOk;
}

bool ORowSetCache::checkJoin(const Reference< XConnection>& _xConnection,
                             const Reference< XSingleSelectQueryAnalyzer >& _xAnalyzer,
                             const OUString& _sUpdateTableName )
{
    bool bOk = false;
    OUString sSql = _xAnalyzer->getQuery();
    OUString sErrorMsg;
    ::connectivity::OSQLParser aSqlParser( m_aContext );
    std::unique_ptr< ::connectivity::OSQLParseNode> pSqlParseNode( aSqlParser.parseTree(sErrorMsg,sSql));
    if ( pSqlParseNode && SQL_ISRULE(pSqlParseNode, select_statement) )
    {
        OSQLParseNode* pTableRefCommalist = pSqlParseNode->getByRule(::connectivity::OSQLParseNode::table_ref_commalist);
        OSL_ENSURE(pTableRefCommalist,"NO tables why!?");
        if(pTableRefCommalist && pTableRefCommalist->count() == 1)
        {
            // we found only one element so it must some kind of join here
            OSQLParseNode* pJoin = pTableRefCommalist->getByRule(::connectivity::OSQLParseNode::qualified_join);
            if(pJoin)
            { // we are only interested in qualified joins like RIGHT or LEFT
                OSQLParseNode* pJoinType    = pJoin->getChild(1);
                OSQLParseNode* pOuterType   = nullptr;
                if(SQL_ISRULE(pJoinType,join_type) && pJoinType->count() == 2)
                    pOuterType = pJoinType->getChild(0);
                else if(SQL_ISRULE(pJoinType,outer_join_type))
                    pOuterType = pJoinType;

                bool bCheck     = false;
                bool bLeftSide  = false;
                if(pOuterType)
                { // found outer join
                    bLeftSide = SQL_ISTOKEN(pOuterType->getChild(0),LEFT);
                    bCheck = bLeftSide || SQL_ISTOKEN(pOuterType->getChild(0),RIGHT);
                }

                if(bCheck)
                { // here we know that we have to check on which side our table resides
                    const OSQLParseNode* pTableRef;
                    if(bLeftSide)
                        pTableRef = pJoin->getChild(0);
                    else
                        pTableRef = pJoin->getChild(3);
                    OSL_ENSURE(SQL_ISRULE(pTableRef,table_ref),"Must be a tableref here!");

                    OUString sTableRange = OSQLParseNode::getTableRange(pTableRef);
                    if(sTableRange.isEmpty())
                        pTableRef->getChild(0)->parseNodeToStr( sTableRange, _xConnection, nullptr, false, false );
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
    // we don't unbound the bookmark column
    if ( m_aInsertRow != m_pInsertMatrix->end() && m_aInsertRow->is() )
    {
        ORowSetValueVector::Vector::iterator aIter = (*m_aInsertRow)->begin()+1;
        ORowSetValueVector::Vector::const_iterator aEnd = (*m_aInsertRow)->end();
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
    OSL_ENSURE((nValue >= static_cast<ORowSetMatrix::difference_type>(0)) && (o3tl::make_unsigned(nValue) < m_pMatrix->size()),"Position is invalid!");
    return ( nValue < 0 || o3tl::make_unsigned(nValue) >= m_pMatrix->size() ) ? m_pMatrix->end() : (m_pMatrix->begin() + nValue);
}

TORowSetOldRowHelperRef ORowSetCache::registerOldRow()
{
    TORowSetOldRowHelperRef pRef = new ORowSetOldRowHelper(ORowSetRow());
    m_aOldRows.push_back(pRef);
    return pRef;
}

void ORowSetCache::deregisterOldRow(const TORowSetOldRowHelperRef& _rRow)
{
    TOldRowSetRows::iterator aOldRowIter = std::find_if(m_aOldRows.begin(), m_aOldRows.end(),
        [&_rRow](const TORowSetOldRowHelperRef& rxOldRow) { return rxOldRow.get() == _rRow.get(); });
    if (aOldRowIter != m_aOldRows.end())
        m_aOldRows.erase(aOldRowIter);
}

bool ORowSetCache::reFillMatrix(sal_Int32 _nNewStartPos, sal_Int32 _nNewEndPos)
{
    for (const auto& rxOldRow : m_aOldRows)
    {
        if ( rxOldRow.is() && rxOldRow->getRow().is() )
            rxOldRow->setRow(new ORowSetValueVector( *(rxOldRow->getRow()) ) );
    }
    sal_Int32 nNewSt = _nNewStartPos;
    bool bRet = fillMatrix(nNewSt,_nNewEndPos);
    m_nStartPos = nNewSt;
    m_nEndPos = _nNewEndPos;
    rotateAllCacheIterators(); // invalidate every iterator
    return bRet;
}

bool ORowSetCache::fill(ORowSetMatrix::iterator& _aIter, const ORowSetMatrix::const_iterator& _aEnd, sal_Int32& _nPos, bool _bCheck)
{
    const sal_Int32 nColumnCount = m_xMetaData->getColumnCount();
    for (; _bCheck && _aIter != _aEnd; ++_aIter, ++_nPos)
    {
        if ( !_aIter->is() )
            *_aIter = new ORowSetValueVector(nColumnCount);
        else
        {
            for (const auto& rxOldRow : m_aOldRows)
            {
                if ( rxOldRow->getRow() == *_aIter )
                    *_aIter = new ORowSetValueVector(nColumnCount);
            }
        }
        m_xCacheSet->fillValueRow(*_aIter, _nPos);
        _bCheck = m_xCacheSet->next();
    }
    return _bCheck;
}

bool ORowSetCache::isResultSetChanged() const
{
    return m_xCacheSet->isResultSetChanged();
}

void ORowSetCache::reset(const Reference< XResultSet>& _xDriverSet)
{
    m_xSet = _xDriverSet;
    m_xMetaData.set(Reference< XResultSetMetaDataSupplier >(_xDriverSet,UNO_QUERY_THROW)->getMetaData());
    m_xCacheSet->reset(_xDriverSet);

    m_bRowCountFinal = false;
    m_nRowCount = 0;
    reFillMatrix(m_nStartPos,m_nEndPos);
}

void ORowSetCache::impl_updateRowFromCache_throw(ORowSetValueVector::Vector& io_aRow
                                           ,std::vector<sal_Int32> const & o_ChangedColumns)
{
    if ( o_ChangedColumns.size() > 1 )
    {
        for (auto const& elem : *m_pMatrix)
        {
            if ( elem.is() && m_xCacheSet->updateColumnValues(*elem,io_aRow,o_ChangedColumns))
            {
                return;
            }
        }
        m_xCacheSet->fillMissingValues(io_aRow);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
