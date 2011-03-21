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
#include "precompiled_dbaccess.hxx"


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

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
/** === end UNO includes === **/

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

DBG_NAME(ORowSetCache)
// -------------------------------------------------------------------------
ORowSetCache::ORowSetCache(const Reference< XResultSet >& _xRs,
                           const Reference< XSingleSelectQueryAnalyzer >& _xAnalyzer,
                           const ::comphelper::ComponentContext& _rContext,
                           const ::rtl::OUString& _rUpdateTableName,
                           sal_Bool&    _bModified,
                           sal_Bool&    _bNew,
                           const ORowSetValueVector& _aParameterValueForCache,
                           const ::rtl::OUString& i_sRowSetFilter,
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
    DBG_CTOR(ORowSetCache,NULL);

    // first try if the result can be used to do inserts and updates
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

    // check if all keys of the updateable table are fetched
    sal_Bool bAllKeysFound = sal_False;
    sal_Int32 nTablesCount = 0;

    sal_Bool bNeedKeySet = !bBookmarkable || (xPropInfo->hasPropertyByName(PROPERTY_RESULTSETCONCURRENCY) &&
                            ::comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETCONCURRENCY)) == ResultSetConcurrency::READ_ONLY);

    Reference< XIndexAccess> xUpdateTableKeys;
    ::rtl::OUString aUpdateTableName = _rUpdateTableName;
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
            Sequence< ::rtl::OUString> aTableNames = xTables->getElementNames();
            if ( aTableNames.getLength() > 1 && !_rUpdateTableName.getLength() && bNeedKeySet )
            {// here we have a join or union and nobody told us which table to update, so we update them all
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
                    // DBG_UNHANDLED_EXCEPTION();
                }
                m_pCacheSet = NULL;
                m_xCacheSet.clear();
            }
            else
            {
                if(_rUpdateTableName.getLength() && xTables->hasByName(_rUpdateTableName))
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

    // first check if resultset is bookmarkable
    if(!bNeedKeySet)
    {
        try
        {
            m_pCacheSet = new OBookmarkSet(i_nMaxRows);
            m_xCacheSet = m_pCacheSet;
            m_pCacheSet->construct(_xRs,i_sRowSetFilter);

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
        catch(const SQLException&)
        {
            bNeedKeySet = sal_True;
        }

    }
    if(bNeedKeySet)
    {
        // need to check if we could handle this select clause
        bAllKeysFound = bAllKeysFound && (nTablesCount == 1 || checkJoin(xConnection,_xAnalyzer,aUpdateTableName));

        // || !(comphelper::hasProperty(PROPERTY_CANUPDATEINSERTEDROWS,xProp) && any2bool(xProp->getPropertyValue(PROPERTY_CANUPDATEINSERTEDROWS)))

        // oj removed because keyset uses only the next// || (xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_RESULTSETTYPE) && comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETTYPE)) == ResultSetType::FORWARD_ONLY)
        if(!bAllKeysFound )
        {
            if ( bBookmarkable )
            {
                // here I know that we have a read only bookmarable cursor
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

            // check privileges
            m_nPrivileges = Privilege::SELECT;
            sal_Bool bNoInsert = sal_False;

            Sequence< ::rtl::OUString> aNames(xColumns->getElementNames());
            const ::rtl::OUString* pIter    = aNames.getConstArray();
            const ::rtl::OUString* pEnd     = pIter + aNames.getLength();
            for(;pIter != pEnd;++pIter)
            {
                Reference<XPropertySet> xColumn(xColumns->getByName(*pIter),UNO_QUERY);
                OSL_ENSURE(xColumn.is(),"Column in table is null!");
                if(xColumn.is())
                {
                    sal_Int32 nNullable = 0;
                    xColumn->getPropertyValue(PROPERTY_ISNULLABLE) >>= nNullable;
                    if(nNullable == ColumnValue::NO_NULLS && aColumnNames.find(*pIter) == aColumnNames.end())
                    { // we found a column where null is not allowed so we can't insert new values
                        bNoInsert = sal_True;
                        break; // one column is enough
                    }
                }
            }

            OKeySet* pKeySet = new OKeySet(m_aUpdateTable,xUpdateTableKeys,aUpdateTableName ,_xAnalyzer,_aParameterValueForCache,i_nMaxRows,m_nRowCount);
            try
            {
                m_pCacheSet = pKeySet;
                m_xCacheSet = m_pCacheSet;
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
            catch(const SQLException&)
            {
                // we couldn't create a keyset here so we have to create a static cache
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
    // last check
    if(!bAllKeysFound && xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_RESULTSETCONCURRENCY) &&
        ::comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETCONCURRENCY)) == ResultSetConcurrency::READ_ONLY)
        m_nPrivileges = Privilege::SELECT;
}

// -------------------------------------------------------------------------
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

    DBG_DTOR(ORowSetCache,NULL);
}

// -------------------------------------------------------------------------
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

        m_pInsertMatrix = new ORowSetMatrix(1); // a little bit overkill but ??? :-)
        m_aInsertRow    = m_pInsertMatrix->end();
    }
    else
    {
        // now correct the iterator in our iterator vector
        ::std::vector<sal_Int32> aPositions;
        ::std::map<sal_Int32,sal_Bool> aCacheIterToChange;
        // first get the positions where they stand now
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

        // now adjust their positions because a resize invalid all iterators
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
        sal_Int32 nNewSt = 1;
        fillMatrix(nNewSt,_nSize+1);
        m_nStartPos = 0;
        m_nEndPos = _nSize;
    }
    else if (m_nStartPos < m_nPosition && m_nPosition < m_nEndPos)
    {
        sal_Int32 nNewSt = -1;
        fillMatrix(nNewSt,_nSize+1);
        m_nStartPos = 0;
        m_nEndPos = _nSize;
    }
}
// -------------------------------------------------------------------------

// XResultSetMetaDataSupplier
Reference< XResultSetMetaData > ORowSetCache::getMetaData(  )
{
    return m_xMetaData;
}
// -------------------------------------------------------------------------
Any lcl_getBookmark(ORowSetValue& i_aValue,OCacheSet* i_pCacheSet)
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
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XRowLocate
Any ORowSetCache::getBookmark(  )
{

    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    if ( m_aMatrixIter >= m_pMatrix->end() || m_aMatrixIter < m_pMatrix->begin() || !(*m_aMatrixIter).isValid())
    {
        return Any(); // this is allowed here because the rowset knowns what it is doing
    }

    return lcl_getBookmark(((*m_aMatrixIter)->get())[0],m_pCacheSet);
}
// -------------------------------------------------------------------------
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
                OSL_ENSURE(m_aMatrixIter->isValid(),"Iterator after moveToBookmark not valid");
            }
            else
                m_aMatrixIter = m_pMatrix->end();
        }
        else
            m_aMatrixIter = m_pMatrix->end();
    }
    else
        return sal_False;

    return m_aMatrixIter != m_pMatrix->end() && (*m_aMatrixIter).isValid();
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows )
{
    sal_Bool bRet( moveToBookmark( bookmark ) );
    if ( bRet )
    {
        m_nPosition = m_pCacheSet->getRow() + rows;
        absolute(m_nPosition);
        //  for(sal_Int32 i=0;i<rows && m_aMatrixIter != m_pMatrix->end();++i,++m_aMatrixIter) ;

        bRet = m_aMatrixIter != m_pMatrix->end() && (*m_aMatrixIter).isValid();
    }

    return bRet;
}
// -------------------------------------------------------------------------
sal_Int32 ORowSetCache::compareBookmarks( const Any& _first, const Any& _second )
{
    return (!_first.hasValue() || !_second.hasValue()) ? CompareBookmark::NOT_COMPARABLE : m_pCacheSet->compareBookmarks(_first,_second);
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::hasOrderedBookmarks(  )
{
    return m_pCacheSet->hasOrderedBookmarks();
}
// -------------------------------------------------------------------------
sal_Int32 ORowSetCache::hashBookmark( const Any& bookmark )
{
    return m_pCacheSet->hashBookmark(bookmark);
}
// XRowUpdate
// -----------------------------------------------------------------------------
void ORowSetCache::updateNull(sal_Int32 columnIndex,ORowSetValueVector::Vector& io_aRow
                              ,::std::vector<sal_Int32>& o_ChangedColumns
                              )
{
    checkUpdateConditions(columnIndex);

    ORowSetValueVector::Vector& rInsert = ((*m_aInsertRow)->get());
    if ( !rInsert[columnIndex].isNull() )
    {
        rInsert[columnIndex].setBound(sal_True);
        rInsert[columnIndex].setNull();
        rInsert[columnIndex].setModified();
        io_aRow[columnIndex].setNull();

        m_pCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}
// -----------------------------------------------------------------------------
void ORowSetCache::updateValue(sal_Int32 columnIndex,const ORowSetValue& x
                               ,ORowSetValueVector::Vector& io_aRow
                               ,::std::vector<sal_Int32>& o_ChangedColumns
                               )
{
    checkUpdateConditions(columnIndex);

    ORowSetValueVector::Vector& rInsert = ((*m_aInsertRow)->get());
    if ( rInsert[columnIndex] != x )
    {
        rInsert[columnIndex].setBound(sal_True);
        rInsert[columnIndex] = x;
        rInsert[columnIndex].setModified();
        io_aRow[columnIndex] = rInsert[columnIndex];

        m_pCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}
// -------------------------------------------------------------------------
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
    rInsert[columnIndex].setBound(sal_True);
    rInsert[columnIndex] = aSeq;
    rInsert[columnIndex].setModified();
    io_aRow[columnIndex] = makeAny(x);

    m_pCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
    impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
}
// -------------------------------------------------------------------------
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
        rInsert[columnIndex].setBound(sal_True);
        rInsert[columnIndex] = aTemp;
        rInsert[columnIndex].setModified();
        io_aRow[columnIndex] = rInsert[columnIndex];

        m_pCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}
// -------------------------------------------------------------------------
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
        rInsert[columnIndex].setBound(sal_True);
        rInsert[columnIndex] = aTemp;
        rInsert[columnIndex].setModified();
        io_aRow[columnIndex] = rInsert[columnIndex];

        m_pCacheSet->mergeColumnValues(columnIndex,rInsert,io_aRow,o_ChangedColumns);
        impl_updateRowFromCache_throw(io_aRow,o_ChangedColumns);
    }
}
// -------------------------------------------------------------------------
// XResultSet
sal_Bool ORowSetCache::next(  )
{
    if(!isAfterLast())
    {
        m_bBeforeFirst = sal_False;
        ++m_nPosition;

        // after we increment the position we have to check if we are already after the last row
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
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::isBeforeFirst(  )
{
    return m_bBeforeFirst;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::isAfterLast(  )
{
    return m_bAfterLast;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::isFirst(  )
{
    return m_nPosition == 1; // ask resultset for
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::isLast(  )
{
    return m_nPosition == m_nRowCount;
}
// -------------------------------------------------------------------------
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
// -------------------------------------------------------------------------
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
            m_nRowCount = m_pCacheSet->getRow();// + 1 removed
        }
        m_pCacheSet->afterLast();

        m_nPosition = 0;
        m_aMatrixIter = m_pMatrix->end();
    }
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::fillMatrix(sal_Int32& _nNewStartPos,sal_Int32 _nNewEndPos)
{
    OSL_ENSURE(_nNewStartPos != _nNewEndPos,"ORowSetCache::fillMatrix: StartPos and EndPos can not be equal!");
    // fill the whole window with new data
    ORowSetMatrix::iterator aIter;
    sal_Int32 i;
    sal_Bool bCheck;
    if ( _nNewStartPos == -1 )
    {
        aIter = m_pMatrix->begin() + m_nEndPos;
        i = m_nEndPos+1;
    }
    else
    {
        aIter = m_pMatrix->begin();
        i = _nNewStartPos;
    }
    bCheck = m_pCacheSet->absolute(i); // -1 no need to


    for(;i<_nNewEndPos;++i,++aIter)
    {
        if(bCheck)
        {
            if(!aIter->isValid())
                *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
            m_pCacheSet->fillValueRow(*aIter,i);
            if(!m_bRowCountFinal)
                ++m_nRowCount;
        }
        else
        {   // there are no more rows found so we can fetch some before start

            if(!m_bRowCountFinal)
            {
                if(m_pCacheSet->previous_checked(sal_False)) // because we stand after the last row
                    m_nRowCount = m_pCacheSet->getRow(); // here we have the row count
                if(!m_nRowCount)
                    m_nRowCount = i-1; // it can be that getRow return zero
                m_bRowCountFinal = sal_True;
            }
            if(m_nRowCount > m_nFetchSize)
            {
                ORowSetMatrix::iterator aEnd = aIter;
                ORowSetMatrix::iterator aRealEnd = m_pMatrix->end();
                sal_Int32 nPos = m_nRowCount - m_nFetchSize + 1;
                _nNewStartPos = nPos;
                bCheck = m_pCacheSet->absolute(_nNewStartPos);

                for(;bCheck && aIter != aRealEnd;++aIter)
                {
                    if(bCheck)
                    {
                        if(!aIter->isValid())
                            *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                        m_pCacheSet->fillValueRow(*aIter,nPos++);
                    }
                    bCheck = m_pCacheSet->next();
                }
                if(aIter != aEnd)
                    ::std::rotate(m_pMatrix->begin(),aEnd,aRealEnd);
            }
            break;
        }
        if ( i < (_nNewEndPos-1) )
            bCheck = m_pCacheSet->next();
    }
    //  m_nStartPos = _nNewStartPos;
    // we have to read one row forward to ensure that we know when we are on last row
    // but only when we don't know it already
    /*
    if(!m_bRowCountFinal)
    {
        if(!m_pCacheSet->next())
        {
            if(m_pCacheSet->previous_checked(sal_False)) // because we stand after the last row
                m_nRowCount = m_pCacheSet->getRow(); // here we have the row count
            m_bRowCountFinal = sal_True;
        }
        else
           m_nRowCount = std::max(i,m_nRowCount);

    }
    */
    return bCheck;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::moveWindow()
{

    sal_Bool bRet = sal_True;

    sal_Int32 nDiff = (sal_Int32)(m_nFetchSize*0.5 -0.5);
    sal_Int32 nNewStartPos  = (m_nPosition - nDiff);
    //  sal_Int32 nNewEndPos    = (m_nPosition+m_nFetchSize*0.5);
    sal_Int32 nNewEndPos    = nNewStartPos + m_nFetchSize;

    if ( m_nPosition <= m_nStartPos )
    {   // the window is behind the new start pos
        if(!m_nStartPos)
            return sal_False;
        // the new position should be the nPos - nFetchSize/2
        if ( nNewEndPos > m_nStartPos )
        {   // but the two regions are overlapping
            // fill the rows behind the new end

            ORowSetMatrix::iterator aEnd; // the iterator we need for rotate
            ORowSetMatrix::iterator aIter; // the iterator we fill with new values

            sal_Bool bCheck = sal_True;
            if ( nNewStartPos < 1 )
            {
                bCheck = m_pCacheSet->first();
                OSL_ENSURE((nNewEndPos - m_nStartPos - nNewStartPos) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
                aEnd = m_pMatrix->begin() + (nNewEndPos - m_nStartPos - nNewStartPos);
                aIter = aEnd;
                m_nStartPos = 0;
            }
            else
            {
                OSL_ENSURE((nNewEndPos - m_nStartPos -1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
                aEnd = m_pMatrix->begin() + ((nNewEndPos - m_nStartPos)-1);
                aIter = m_pMatrix->begin() + ((nNewEndPos - m_nStartPos)-1);
                bCheck = m_pCacheSet->absolute(nNewStartPos);
                m_nStartPos = nNewStartPos -1;
            }

            if ( bCheck )
            {
                sal_Int32 nPos = m_nStartPos;
                bCheck = fill(aIter,m_pMatrix->end(),nPos,bCheck);

                ::std::rotate(m_pMatrix->begin(),aEnd,m_pMatrix->end());
                // now correct the iterator in our iterator vector
                //  rotateCacheIterator(aEnd-m_pMatrix->begin()); //can't be used because they decrement and here we need to increment
                ptrdiff_t nNewDist = aEnd - m_pMatrix->begin();
                ptrdiff_t nOffSet = m_pMatrix->end() - aEnd;
                ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
                ORowSetCacheMap::iterator aCacheEnd  = m_aCacheIterators.end();
                for(;aCacheIter != aCacheEnd;++aCacheIter)
                {
                    if ( !aCacheIter->second.pRowSet->isInsertRow()
                        && aCacheIter->second.aIterator != m_pMatrix->end() && !m_bModified )
                    {
                        ptrdiff_t nDist = (aCacheIter->second.aIterator - m_pMatrix->begin());
                        if ( nDist >= nNewDist )
                        {
                            aCacheIter->second.aIterator = m_pMatrix->end();
                        }
                        else
                        {
#if OSL_DEBUG_LEVEL > 0
                            ORowSetMatrix::iterator aOldPos;
                            aOldPos = aCacheIter->second.aIterator;
#endif
                            CHECK_MATRIX_POS( ((aOldPos - m_pMatrix->begin()) + nOffSet) );
                            aCacheIter->second.aIterator += nOffSet;
#if OSL_DEBUG_LEVEL > 0
                            ORowSetMatrix::iterator aCurrentPos;
                            aCurrentPos = aCacheIter->second.aIterator;
#endif
                            OSL_ENSURE(aCacheIter->second.aIterator >= m_pMatrix->begin()
                                    && aCacheIter->second.aIterator < m_pMatrix->end(),"Iterator out of area!");
                        }
                    }
                }
            }
            else
            { // normaly this should never happen
                OSL_ENSURE(0,"What the hell is happen here!");
                return sal_False;
            }
        }
        else
        {// no rows can be reused so fill again
            if(nNewStartPos < 1) // special case
            {
                m_nStartPos = 0;

                rotateCacheIterator(static_cast<sal_Int16>(m_nFetchSize+1)); // static_cast<sal_Int16>(m_nFetchSize+1)

                m_pCacheSet->beforeFirst();

                sal_Bool bCheck;
                ORowSetMatrix::iterator aIter = m_pMatrix->begin();
                for(sal_Int32 i=0;i<m_nFetchSize;++i,++aIter)
                {
                    bCheck = m_pCacheSet->next();
                    if ( bCheck )
                    {
                        if(!aIter->isValid())
                            *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                        m_pCacheSet->fillValueRow(*aIter,i+1);
                    }
                    else
                        *aIter = NULL;
                }
            }
            else
                bRet = reFillMatrix(nNewStartPos,nNewEndPos);
        }
    }
    else if(m_nPosition > m_nStartPos)
    {   // the new start pos is above the startpos of the window

        if(m_nPosition <= (m_nStartPos+m_nFetchSize))
        {   // position in window
            OSL_ENSURE((m_nPosition - m_nStartPos -1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
            m_aMatrixIter = calcPosition();
            if(!m_aMatrixIter->isValid())
            {
                sal_Bool bOk( m_pCacheSet->absolute( m_nPosition ) );
                if ( bOk )
                {
                    *m_aMatrixIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                    m_pCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
                    // we have to read one row forward to ensure that we know when we are on last row
                    // but only when we don't know it already
                    if ( !m_bRowCountFinal )
                    {
                        bOk = m_pCacheSet->absolute_checked( m_nPosition + 1,sal_False );
                        if ( bOk )
                            m_nRowCount = std::max(sal_Int32(m_nPosition+1),m_nRowCount);
                    }
                }
                if(!bOk && !m_bRowCountFinal)
                {
                    // because we stand after the last row
                    m_nRowCount = m_pCacheSet->previous_checked(sal_False) ? m_pCacheSet->getRow() : 0;//  + 1 removed
                    m_bRowCountFinal = sal_True;
                }
            }
        }
        else if(nNewStartPos < (m_nStartPos+m_nFetchSize))
        {   // position behind window but the region is overlapping
            // the rows from begin() to (begin + nNewStartPos - m_nStartPos) can be refilled with the new rows
            // the rows behind this can be reused
            ORowSetMatrix::iterator aIter = m_pMatrix->begin();
            CHECK_MATRIX_POS(nNewStartPos - m_nStartPos - 1);
            ORowSetMatrix::iterator aEnd  = m_pMatrix->begin() + (nNewStartPos - m_nStartPos - 1);

            sal_Int32 nPos = m_nStartPos + m_nFetchSize + 1;
            sal_Bool bCheck = m_pCacheSet->absolute(nPos);
            bCheck = fill(aIter,aEnd,nPos,bCheck); // refill the region wew don't need anymore

            // we have to read one row forward to enshure that we know when we are on last row
            // but only when we don't know it already
            sal_Bool bOk = sal_True;
            if(bCheck && !m_bRowCountFinal)
                bOk = m_pCacheSet->next();
            // bind end to front
            if(bCheck)
            {   // rotate the end to the front
                ::std::rotate(m_pMatrix->begin(),aIter,m_pMatrix->end());
                // now correct the iterator in our iterator vector
                rotateCacheIterator( (sal_Int16)( aIter - m_pMatrix->begin() ) );
                m_nStartPos = nNewStartPos - 1; // must be -1
                // now I can say how many rows we have
                if(!bOk)
                {
                    m_pCacheSet->previous_checked(sal_False); // because we stand after the last row
                    m_nRowCount      = nPos; // here we have the row count
                    m_bRowCountFinal = sal_True;
                }
                else if(!m_bRowCountFinal)
                    m_nRowCount = std::max(++nPos,m_nRowCount);
            }
            else
            {   // the end was reached before end() so we can set the start before nNewStartPos

                m_nStartPos += (aIter - m_pMatrix->begin());
                //  m_nStartPos = (aIter - m_pMatrix->begin());
                ::std::rotate(m_pMatrix->begin(),aIter,m_pMatrix->end());
                // now correct the iterator in our iterator vector
                rotateCacheIterator( (sal_Int16)( aIter - m_pMatrix->begin() ) );

                if ( !m_bRowCountFinal )
                {
                    m_pCacheSet->previous_checked(sal_False);                   // because we stand after the last row
                    m_nRowCount      = std::max(m_nRowCount,--nPos);    // here we have the row count
                    OSL_ENSURE(nPos == m_pCacheSet->getRow(),"nPos isn't valid!");
                    m_bRowCountFinal = sal_True;
                }
                // TODO check
                //  m_nStartPos = (nNewStartPos+m_nRowCount) - m_nFetchSize ;
                if(m_nStartPos < 0)
                    m_nStartPos = 0;
            }
            // here we need only to check if the begining row is valid. If not we have to fetch it.
            if(!m_pMatrix->begin()->isValid())
            {
                aIter = m_pMatrix->begin();

                nPos    = m_nStartPos;
                bCheck  = m_pCacheSet->absolute_checked(m_nStartPos,sal_False);
                for(; !aIter->isValid() && bCheck;++aIter)
                {
                    OSL_ENSURE(aIter != m_pMatrix->end(),"Invalid iterator");
                    bCheck = m_pCacheSet->next();
                    if ( bCheck ) // resultset stands on right position
                    {
                        *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                        m_pCacheSet->fillValueRow(*aIter,++nPos);
                    }
                }
            }
        }
        else // no rows can be reused so fill again
            bRet = reFillMatrix(nNewStartPos,nNewEndPos);
    }

    if(!m_bRowCountFinal)
       m_nRowCount = std::max(m_nPosition,m_nRowCount);
    OSL_ENSURE(m_nStartPos >= 0,"ORowSetCache::moveWindow: m_nStartPos is less than 0!");

    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::first(  )
{
    // first move to the first row
    // then check if the cache window is at the begining
    // when not postionize the window and fill it with data
    // smart moving of the window -> clear only the rows whom are out of range
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
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::last(  )
{
    sal_Bool bRet = m_pCacheSet->last();
    if(bRet)
    {
        m_bBeforeFirst = m_bAfterLast = sal_False;
        if(!m_bRowCountFinal)
        {
            m_bRowCountFinal = sal_True;
            m_nRowCount = m_nPosition = m_pCacheSet->getRow(); // not  + 1
        }
        m_nPosition = m_pCacheSet->getRow();
        moveWindow();
        // we have to repositioning because moveWindow can modify the cache
        m_pCacheSet->last();
//      if(m_nPosition > m_nFetchSize)
//          m_aMatrixIter = m_pMatrix->end() -1;
//      else
//          m_aMatrixIter = m_pMatrix->begin() + m_nPosition - 1;
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
        OSL_ENSURE((*m_aMatrixIter).isValid(),"ORowSetCache::last: Row not valid!");
    }
#endif

    return bRet;
}
// -------------------------------------------------------------------------
sal_Int32 ORowSetCache::getRow(  )
{
    return (isBeforeFirst() || isAfterLast()) ? 0 : m_nPosition;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::absolute( sal_Int32 row )
{
    if(!row )
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_ABS_ZERO),NULL,SQLSTATE_GENERAL,1000,Any() );

    if(row < 0)
    {
        // here we have to scroll from the last row to backward so we have to go to last row and
        // and two the previous
        if(m_bRowCountFinal || last())
        {
            m_nPosition = m_nRowCount + row + 1; // + row because row is negative and +1 because row==-1 means last row
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
        // the position flags
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
// -------------------------------------------------------------------------
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
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::previous(  )
{
    sal_Bool bRet = sal_False;
    if(!isBeforeFirst())
    {
        if(m_bAfterLast)   // we stand after the last row so one before is the last row
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
                bRet = (*m_aMatrixIter).isValid();
            }
        }
    }
    return bRet;
}
// -------------------------------------------------------------------------
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
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::rowUpdated(  )
{
    return m_pCacheSet->rowUpdated();
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::rowInserted(  )
{
    return m_pCacheSet->rowInserted();
}
// -------------------------------------------------------------------------
// XResultSetUpdate
sal_Bool ORowSetCache::insertRow(::std::vector< Any >& o_aBookmarks)
{
    if ( !m_bNew || !m_aInsertRow->isValid() )
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
            // update the cached values
            ORowSetValueVector::Vector& rCurrentRow = ((*m_aMatrixIter))->get();
            ORowSetMatrix::iterator aIter = m_pMatrix->begin();
            for(;aIter != m_pMatrix->end();++aIter)
            {
                if ( m_aMatrixIter != aIter && aIter->isValid() && m_pCacheSet->columnValuesUpdated((*aIter)->get(),rCurrentRow) )
                {
                    o_aBookmarks.push_back(lcl_getBookmark((*aIter)->get()[0],m_pCacheSet));
                }
            }
        }
        else
        {
            OSL_ENSURE(0,"There must be a bookmark after the row was inserted!");
        }
    }
    return bRet;
}
// -------------------------------------------------------------------------
void ORowSetCache::resetInsertRow(sal_Bool _bClearInsertRow)
{
    if ( _bClearInsertRow )
        clearInsertRow();
    m_bNew      = sal_False;
    m_bModified = sal_False;
}
// -------------------------------------------------------------------------
void ORowSetCache::cancelRowModification()
{
    // clear the insertrow references   -> implies that the current row of the rowset changes as well
    ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
    ORowSetCacheMap::iterator aCacheEnd = m_aCacheIterators.end();
    for(;aCacheIter != aCacheEnd;++aCacheIter)
    {
        if ( aCacheIter->second.pRowSet->isInsertRow() && aCacheIter->second.aIterator == m_aInsertRow )
            aCacheIter->second.aIterator = m_pMatrix->end();
    } // for(;aCacheIter != aCacheEnd;++aCacheIter)
    resetInsertRow(sal_False);
}
// -------------------------------------------------------------------------
void ORowSetCache::updateRow( ORowSetMatrix::iterator& _rUpdateRow,::std::vector< Any >& o_aBookmarks )
{
    if(isAfterLast() || isBeforeFirst())
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_UPDATEROW),NULL,SQLSTATE_GENERAL,1000,Any() );

    Any aBookmark = ((*_rUpdateRow)->get())[0].makeAny();
    OSL_ENSURE(aBookmark.hasValue(),"Bookmark must have a value!");
    // here we don't have to reposition our CacheSet, when we try to update a row,
    // the row was already fetched
    moveToBookmark(aBookmark);
    m_pCacheSet->updateRow(*_rUpdateRow,*m_aMatrixIter,m_aUpdateTable);
    // refetch the whole row
    (*m_aMatrixIter) = NULL;

    if ( moveToBookmark(aBookmark) )
    {
        // update the cached values
        ORowSetValueVector::Vector& rCurrentRow = ((*m_aMatrixIter))->get();
        ORowSetMatrix::iterator aIter = m_pMatrix->begin();
        for(;aIter != m_pMatrix->end();++aIter)
        {
            if ( m_aMatrixIter != aIter && aIter->isValid() && m_pCacheSet->columnValuesUpdated((*aIter)->get(),rCurrentRow) )
            {
                o_aBookmarks.push_back(lcl_getBookmark((*aIter)->get()[0],m_pCacheSet));
            }
        }
    }

    m_bModified = sal_False;
}
// -------------------------------------------------------------------------
bool ORowSetCache::deleteRow(  )
{
    if(isAfterLast() || isBeforeFirst())
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_DELETEROW),NULL,SQLSTATE_GENERAL,1000,Any() );

    //  m_pCacheSet->absolute(m_nPosition);
    m_pCacheSet->deleteRow(*m_aMatrixIter,m_aUpdateTable);
    if ( !m_pCacheSet->rowDeleted() )
        return false;

    --m_nRowCount;
    OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
    ORowSetMatrix::iterator aPos = calcPosition();
    (*aPos)   = NULL;

    ORowSetMatrix::iterator aEnd = m_pMatrix->end();
    for(++aPos;aPos != aEnd && aPos->isValid();++aPos)
    {
        *(aPos-1) = *aPos;
        (*aPos)   = NULL;
    }
    m_aMatrixIter = m_pMatrix->end();

    --m_nPosition;
    return true;
}
// -------------------------------------------------------------------------
void ORowSetCache::cancelRowUpdates(  )
{
    m_bNew = m_bModified = sal_False;
    if(!m_nPosition)
    {
        OSL_ENSURE(0,"cancelRowUpdates:Invalid positions pos == 0");
        ::dbtools::throwFunctionSequenceException(NULL);
    }

    if(m_pCacheSet->absolute(m_nPosition))
        m_pCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
    else
    {
        OSL_ENSURE(0,"cancelRowUpdates couldn't position right with absolute");
        ::dbtools::throwFunctionSequenceException(NULL);
    }
}
// -------------------------------------------------------------------------
void ORowSetCache::moveToInsertRow(  )
{
    m_bNew      = sal_True;
    m_bUpdated  = m_bAfterLast = sal_False;

    m_aInsertRow = m_pInsertMatrix->begin();
    if(!m_aInsertRow->isValid())
        *m_aInsertRow = new ORowSetValueVector(m_xMetaData->getColumnCount());

    // we don't unbound the bookmark column
    ORowSetValueVector::Vector::iterator aIter = (*m_aInsertRow)->get().begin()+1;
    ORowSetValueVector::Vector::iterator aEnd = (*m_aInsertRow)->get().end();
    for(sal_Int32 i = 1;aIter != aEnd;++aIter,++i)
    {
        aIter->setBound(sal_False);
        aIter->setModified(sal_False);
        aIter->setNull();
        aIter->setTypeKind(m_xMetaData->getColumnType(i));
    }
}
// -------------------------------------------------------------------------
ORowSetCacheIterator ORowSetCache::createIterator(ORowSetBase* _pRowSet)
{

    ORowSetCacheIterator_Helper aHelper;
    aHelper.aIterator = m_pMatrix->end();
    aHelper.pRowSet = _pRowSet;
    return ORowSetCacheIterator(m_aCacheIterators.insert(m_aCacheIterators.begin(),ORowSetCacheMap::value_type(m_aCacheIterators.size()+1,aHelper)),this,_pRowSet);
}
// -----------------------------------------------------------------------------
void ORowSetCache::deleteIterator(const ORowSetBase* _pRowSet)
{
    ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
    for(;aCacheIter != m_aCacheIterators.end();)
    {
        if ( aCacheIter->second.pRowSet == _pRowSet )
        {
            m_aCacheIterators.erase(aCacheIter);
            aCacheIter = m_aCacheIterators.begin();
        } // if ( aCacheIter->second.pRowSet == _pRowSet )
        else
            ++aCacheIter;
    }
}
// -----------------------------------------------------------------------------
void ORowSetCache::rotateCacheIterator(ORowSetMatrix::difference_type _nDist)
{
    if(_nDist)
    {
        // now correct the iterator in our iterator vector
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
// -------------------------------------------------------------------------
void ORowSetCache::setUpdateIterator(const ORowSetMatrix::iterator& _rOriginalRow)
{
    m_aInsertRow = m_pInsertMatrix->begin();
    if(!m_aInsertRow->isValid())
        *m_aInsertRow = new ORowSetValueVector(m_xMetaData->getColumnCount());

    (*(*m_aInsertRow)) = (*(*_rOriginalRow));
    // we don't unbound the bookmark column
    ORowSetValueVector::Vector::iterator aIter = (*m_aInsertRow)->get().begin();
    ORowSetValueVector::Vector::iterator aEnd = (*m_aInsertRow)->get().end();
    for(;aIter != aEnd;++aIter)
        aIter->setModified(sal_False);
}
// -----------------------------------------------------------------------------
void ORowSetCache::checkPositionFlags()
{
    if(m_bRowCountFinal)
    {
        m_bAfterLast    = m_nPosition > m_nRowCount;
        if(m_bAfterLast)
            m_nPosition = 0;//m_nRowCount;
    }
}
// -----------------------------------------------------------------------------
void ORowSetCache::checkUpdateConditions(sal_Int32 columnIndex)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->get().size())
        throwFunctionSequenceException(m_xSet.get());
}
//------------------------------------------------------------------------------
sal_Bool ORowSetCache::checkInnerJoin(const ::connectivity::OSQLParseNode *pNode,const Reference< XConnection>& _xConnection,const ::rtl::OUString& _sUpdateTableName)
{
    sal_Bool bOk = sal_False;
    if (pNode->count() == 3 &&  // Ausdruck is geklammert
        SQL_ISPUNCTUATION(pNode->getChild(0),"(") &&
        SQL_ISPUNCTUATION(pNode->getChild(2),")"))
    {
        bOk = checkInnerJoin(pNode->getChild(1),_xConnection,_sUpdateTableName);
    }
    else if ((SQL_ISRULE(pNode,search_condition) || SQL_ISRULE(pNode,boolean_term)) &&          // AND/OR-Verknuepfung:
                pNode->count() == 3)
    {
        // nur AND Verknüpfung zulassen
        if ( SQL_ISTOKEN(pNode->getChild(1),AND) )
            bOk = checkInnerJoin(pNode->getChild(0),_xConnection,_sUpdateTableName)
                && checkInnerJoin(pNode->getChild(2),_xConnection,_sUpdateTableName);
    }
    else if (SQL_ISRULE(pNode,comparison_predicate))
    {
        // only the comparison of columns is allowed
        DBG_ASSERT(pNode->count() == 3,"checkInnerJoin: Fehler im Parse Tree");
        if (!(SQL_ISRULE(pNode->getChild(0),column_ref) &&
                SQL_ISRULE(pNode->getChild(2),column_ref) &&
                pNode->getChild(1)->getNodeType() == SQL_NODE_EQUAL))
        {
            bOk = sal_False;
        }
        ::rtl::OUString sColumnName,sTableRange;
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
// -----------------------------------------------------------------------------
sal_Bool ORowSetCache::checkJoin(const Reference< XConnection>& _xConnection,
                                 const Reference< XSingleSelectQueryAnalyzer >& _xAnalyzer,
                                 const ::rtl::OUString& _sUpdateTableName )
{
    sal_Bool bOk = sal_False;
    ::rtl::OUString sSql = _xAnalyzer->getQuery();
    ::rtl::OUString sErrorMsg;
    ::connectivity::OSQLParser aSqlParser( m_aContext.getLegacyServiceFactory() );
    ::std::auto_ptr< ::connectivity::OSQLParseNode> pSqlParseNode( aSqlParser.parseTree(sErrorMsg,sSql));
    if ( pSqlParseNode.get() && SQL_ISRULE(pSqlParseNode, select_statement) )
    {
        OSQLParseNode* pTableRefCommalist = pSqlParseNode->getByRule(::connectivity::OSQLParseNode::table_ref_commalist);
        OSL_ENSURE(pTableRefCommalist,"NO tables why!?");
        if(pTableRefCommalist && pTableRefCommalist->count() == 1)
        {
            // we found only one element so it must some kind of join here
            OSQLParseNode* pJoin = pTableRefCommalist->getByRule(::connectivity::OSQLParseNode::qualified_join);
            if(pJoin)
            { // we are only intereseted in qualified joins like RIGHT or LEFT
                OSQLParseNode* pJoinType    = pJoin->getChild(1);
                OSQLParseNode* pOuterType   = NULL;
                if(SQL_ISRULE(pJoinType,join_type) && pJoinType->count() == 2)
                    pOuterType = pJoinType->getChild(0);
                else if(SQL_ISRULE(pJoinType,outer_join_type))
                    pOuterType = pJoinType;

                sal_Bool bCheck     = sal_False;
                sal_Bool bLeftSide  = sal_False;
                if(pOuterType)
                { // found outer join
                    bLeftSide = SQL_ISTOKEN(pOuterType->getChild(0),LEFT);
                    bCheck = bLeftSide || SQL_ISTOKEN(pOuterType->getChild(0),RIGHT);
                }

                if(bCheck)
                { // here we know that we have to check on which side our table resides
                    const OSQLParseNode* pTableRef = pJoin->getByRule(::connectivity::OSQLParseNode::qualified_join);
                    if(bLeftSide)
                        pTableRef = pJoin->getChild(0);
                    else
                        pTableRef = pJoin->getChild(3);
                    OSL_ENSURE(SQL_ISRULE(pTableRef,table_ref),"Must be a tableref here!");

                    ::rtl::OUString sTableRange = OSQLParseNode::getTableRange(pTableRef);
                    if(!sTableRange.getLength())
                        pTableRef->getChild(0)->parseNodeToStr( sTableRange, _xConnection, NULL, sal_False, sal_False );
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
// -----------------------------------------------------------------------------
void ORowSetCache::clearInsertRow()
{
    // we don't unbound the bookmark column
    if ( m_aInsertRow != m_pInsertMatrix->end() && m_aInsertRow->isValid() )
    {
        ORowSetValueVector::Vector::iterator aIter = (*m_aInsertRow)->get().begin()+1;
        ORowSetValueVector::Vector::iterator aEnd = (*m_aInsertRow)->get().end();
        for(;aIter != aEnd;++aIter)
        {
            aIter->setBound(sal_False);
            aIter->setModified(sal_False);
            aIter->setNull();
        } // for(;aIter != (*m_aInsertRow)->end();++aIter)
    }
}
// -----------------------------------------------------------------------------
ORowSetMatrix::iterator ORowSetCache::calcPosition() const
{
    sal_Int32 nValue = (m_nPosition - m_nStartPos) - 1;
    CHECK_MATRIX_POS(nValue);
    return ( nValue < 0 || nValue >= static_cast<sal_Int32>(m_pMatrix->size()) ) ? m_pMatrix->end() : (m_pMatrix->begin() + nValue);
}
// -----------------------------------------------------------------------------

TORowSetOldRowHelperRef ORowSetCache::registerOldRow()
{
    TORowSetOldRowHelperRef pRef = new ORowSetOldRowHelper(ORowSetRow());
    m_aOldRows.push_back(pRef);
    return pRef;
}
// -----------------------------------------------------------------------------
void ORowSetCache::deregisterOldRow(const TORowSetOldRowHelperRef& _rRow)
{
    TOldRowSetRows::iterator aOldRowEnd = m_aOldRows.end();
    for (TOldRowSetRows::iterator aOldRowIter = m_aOldRows.begin(); aOldRowIter != aOldRowEnd; ++aOldRowIter)
    {
        if ( aOldRowIter->getBodyPtr() == _rRow.getBodyPtr() )
        {
            m_aOldRows.erase(aOldRowIter);
            break;
        }

    }
}
// -----------------------------------------------------------------------------
sal_Bool ORowSetCache::reFillMatrix(sal_Int32 _nNewStartPos,sal_Int32 _nNewEndPos)
{
    TOldRowSetRows::iterator aOldRowEnd = m_aOldRows.end();
    for (TOldRowSetRows::iterator aOldRowIter = m_aOldRows.begin(); aOldRowIter != aOldRowEnd; ++aOldRowIter)
    {
        if ( aOldRowIter->isValid() && aOldRowIter->getBody().getRow().isValid() )
            aOldRowIter->getBody().setRow(new ORowSetValueVector(aOldRowIter->getBody().getRow().getBody()) );
    }
    sal_Int32 nNewSt = _nNewStartPos;
    sal_Bool bRet = fillMatrix(nNewSt,_nNewEndPos);
    m_nStartPos = nNewSt - 1;
    rotateCacheIterator(static_cast<sal_Int16>(m_nFetchSize+1)); // forces that every iterator will be set to null
    return bRet;
}
// -----------------------------------------------------------------------------
sal_Bool ORowSetCache::fill(ORowSetMatrix::iterator& _aIter,const ORowSetMatrix::iterator& _aEnd,sal_Int32& _nPos,sal_Bool _bCheck)
{
    sal_Int32 nColumnCount = m_xMetaData->getColumnCount();
    for(; _bCheck && _aIter != _aEnd;)
    {
        if ( !_aIter->isValid() )
            *_aIter = new ORowSetValueVector(nColumnCount);
        else
        {
            TOldRowSetRows::iterator aOldRowEnd = m_aOldRows.end();
            for (TOldRowSetRows::iterator aOldRowIter = m_aOldRows.begin(); aOldRowIter != aOldRowEnd; ++aOldRowIter)
            {
                if ( aOldRowIter->getBody().getRow().isEqualBody(*_aIter) )
                    *_aIter = new ORowSetValueVector(nColumnCount);
            }
        }
        m_pCacheSet->fillValueRow(*_aIter++,++_nPos);
        _bCheck = m_pCacheSet->next();
    }
    return _bCheck;
}
// -----------------------------------------------------------------------------
bool ORowSetCache::isResultSetChanged() const
{
    return m_pCacheSet->isResultSetChanged();
}
// -----------------------------------------------------------------------------
void ORowSetCache::reset(const Reference< XResultSet>& _xDriverSet)
{
    m_xMetaData.set(Reference< XResultSetMetaDataSupplier >(_xDriverSet,UNO_QUERY)->getMetaData());
    m_pCacheSet->reset(_xDriverSet);

    m_bRowCountFinal = sal_False;
    m_nRowCount = 0;
    reFillMatrix(m_nStartPos+1,m_nEndPos+1);
}
// -----------------------------------------------------------------------------
void ORowSetCache::impl_updateRowFromCache_throw(ORowSetValueVector::Vector& io_aRow
                                           ,::std::vector<sal_Int32>& o_ChangedColumns)
{
    if ( o_ChangedColumns.size() > 1 )
    {
        ORowSetMatrix::iterator aIter = m_pMatrix->begin();
        for(;aIter != m_pMatrix->end();++aIter)
        {
            if ( aIter->isValid() && m_pCacheSet->updateColumnValues((*aIter)->get(),io_aRow,o_ChangedColumns))
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
// -----------------------------------------------------------------------------
