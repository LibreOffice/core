/*************************************************************************
 *
 *  $RCSfile: RowSetCache.cxx,v $
 *
 *  $Revision: 1.68 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-21 12:27:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _________ Ocke Janssen____________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "RowSetCache.hxx"
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef DBACCESS_CORE_API_BOOKMARKSET_HXX
#include "BookmarkSet.hxx"
#endif
#ifndef DBACCESS_CORE_API_STATICSET_HXX
#include "StaticSet.hxx"
#endif
#ifndef DBACCESS_CORE_API_KEYSET_HXX
#include "KeySet.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _DBACORE_DATACOLUMN_HXX_
#include "CRowSetDataColumn.hxx"
#endif
#ifndef DBACCESS_CORE_API_CROWSETCOLUMN_HXX
#include "CRowSetColumn.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif
#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_COMPAREBOOKMARK_HPP_
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#endif

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

// -------------------------------------------------------------------------
ORowSetCache::ORowSetCache(const Reference< XResultSet >& _xRs,
                           const Reference< XSQLQueryComposer >& _xComposer,
                           const Reference< XMultiServiceFactory >& _xServiceFactory,
                           const ORowSetValueVector&    _rParameterRow,
                           const ::rtl::OUString& _rUpdateTableName,
                           sal_Bool&    _bModified,
                           sal_Bool&    _bNew)
    : m_xSet(_xRs)
    ,m_nStartPos(0)
    ,m_nEndPos(0)
    ,m_nPosition(0)
    ,m_nRowCount(0)
    ,m_bBeforeFirst(sal_True)
    ,m_bAfterLast(sal_False)
    ,m_bRowCountFinal(sal_False)
    ,m_bInserted(sal_False)
    ,m_bDeleted(sal_False)
    ,m_bUpdated(sal_False)
    ,m_xMetaData(Reference< XResultSetMetaDataSupplier >(_xRs,UNO_QUERY)->getMetaData())
    ,m_xServiceFactory(_xServiceFactory)
    ,m_nFetchSize(0)
    ,m_bNew(_bNew)
    ,m_bModified(_bModified)
    ,m_pMatrix(NULL)
    ,m_pInsertMatrix(NULL)
    ,m_pCacheSet(NULL)
{
    // check if all keys of the updateable table are fetched
    sal_Bool bAllKeysFound = sal_False;
    sal_Int32 nTablesCount = 0;

    ::rtl::OUString aUpdateTableName = _rUpdateTableName;
    Reference< XConnection> xConnection;
    if(_xComposer.is())
    {
        try
        {
            Reference<XTablesSupplier> xTabSup(_xComposer,UNO_QUERY);
            OSL_ENSURE(xTabSup.is(),"ORowSet::execute composer isn't a tablesupplier!");
            Reference<XNameAccess> xTables = xTabSup->getTables();


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
                Reference<XKeysSupplier> xKeys(m_aUpdateTable,UNO_QUERY);
                if(xKeys.is())
                {
                    Reference< XIndexAccess> xKeyIndex = xKeys->getKeys();
                    if ( xKeyIndex.is() )
                    {
                        Reference<XColumnsSupplier> xColumnsSupplier;
                        // search the one and only primary key
                        for(sal_Int32 i=0;i< xKeyIndex->getCount();++i)
                        {
                            Reference<XPropertySet> xProp;
                            ::cppu::extractInterface(xProp,xKeyIndex->getByIndex(i));
                            sal_Int32 nKeyType = 0;
                            xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                            if(KeyType::PRIMARY == nKeyType)
                            {
                                xColumnsSupplier = Reference<XColumnsSupplier>(xProp,UNO_QUERY);
                                break;
                            }
                        }

                        if(xColumnsSupplier.is())
                        {
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

                            Reference<XNameAccess> xColumns = xColumnsSupplier->getColumns();
                            Reference<XColumnsSupplier> xColSup(_xComposer,UNO_QUERY);
                            Reference<XNameAccess> xSelColumns = xColSup->getColumns();

                            OColumnNamePos aColumnNames(xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers() ? true : false);
                            ::dbaccess::getColumnPositions(xSelColumns,xColumns,aUpdateTableName,aColumnNames);
                            bAllKeysFound = sal_Int32(aColumnNames.size()) == xColumns->getElementNames().getLength();
                        }
                    }
                }
            }
        }
        catch(Exception&)
        {
        }
    }
    Reference< XPropertySet> xProp(_xRs,UNO_QUERY);

    sal_Bool bNeedKeySet = !(xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_ISBOOKMARKABLE) &&
                             any2bool(xProp->getPropertyValue(PROPERTY_ISBOOKMARKABLE)));
    bNeedKeySet = bNeedKeySet || (xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_RESULTSETCONCURRENCY) &&
                            ::comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETCONCURRENCY)) == ResultSetConcurrency::READ_ONLY);

    // first check if resultset is bookmarkable
    if(!bNeedKeySet)
    {
        try
        {
            m_pCacheSet = new OBookmarkSet();
            m_pCacheSet->construct(_xRs);

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
        bAllKeysFound = bAllKeysFound && (nTablesCount == 1 || checkJoin(xConnection,_xComposer,aUpdateTableName));

        // || !(comphelper::hasProperty(PROPERTY_CANUPDATEINSERTEDROWS,xProp) && any2bool(xProp->getPropertyValue(PROPERTY_CANUPDATEINSERTEDROWS)))

        // oj removed because keyset uses only the next// || (xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_RESULTSETTYPE) && comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETTYPE)) == ResultSetType::FORWARD_ONLY)
        if(!bAllKeysFound )
        {
            m_pCacheSet = new OStaticSet();
            m_pCacheSet->construct(_xRs);
            m_nPrivileges = Privilege::SELECT;
        }
        else
        {
            OColumnNamePos aColumnNames(xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers() ? true : false);
            Reference<XColumnsSupplier> xColSup(_xComposer,UNO_QUERY);
            Reference<XNameAccess> xSelColumns  = xColSup->getColumns();
            Reference<XNameAccess> xColumns     = m_aUpdateTable->getColumns();
            ::dbaccess::getColumnPositions(xSelColumns,xColumns,aUpdateTableName,aColumnNames);

            // check privileges
            m_nPrivileges = Privilege::SELECT;
            sal_Bool bNoInsert = sal_False;

            Sequence< ::rtl::OUString> aNames(xColumns->getElementNames());
            const ::rtl::OUString* pBegin   = aNames.getConstArray();
            const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
            for(;pBegin != pEnd;++pBegin)
            {
                Reference<XPropertySet> xColumn;
                ::cppu::extractInterface(xColumn,xColumns->getByName(*pBegin));
                OSL_ENSURE(xColumn.is(),"Column in table is null!");
                if(xColumn.is())
                {
                    sal_Int32 nNullable = 0;
                    xColumn->getPropertyValue(PROPERTY_ISNULLABLE) >>= nNullable;
                    if(nNullable == ColumnValue::NO_NULLS && aColumnNames.find(*pBegin) == aColumnNames.end())
                    { // we found a column where null is not allowed so we can't insert new values
                        bNoInsert = sal_True;
                        break; // one column is enough
                    }
                }
            }

            OKeySet* pKeySet = new OKeySet(m_aUpdateTable,aUpdateTableName ,_xComposer);
            try
            {
                pKeySet->construct(_xRs);

                // now we need to set the extern parameters because the select stmt could contain a :var1
                pKeySet->setExternParameters(_rParameterRow);
                m_pCacheSet = pKeySet;

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
                    delete m_pCacheSet;
                m_pCacheSet = new OStaticSet();
                m_pCacheSet->construct(_xRs);
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
    delete m_pCacheSet;
    m_pCacheSet = NULL;
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

// -------------------------------------------------------------------------
void ORowSetCache::setMaxRowSize(sal_Int32 _nSize)
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
        for(;aCacheIter != m_aCacheIterators.end();++aCacheIter)
        {
            aCacheIterToChange[aCacheIter->first] = sal_False;
            if(!aCacheIter->second.aIterator)
                continue;
            if(aCacheIter->second.aIterator != m_aInsertRow && !m_bInserted && !m_bModified)
            {
                sal_Int16 nDist = (aCacheIter->second.aIterator - m_pMatrix->begin());
                aPositions.push_back(nDist);
                aCacheIterToChange[aCacheIter->first] = sal_True;
            }
        }
        sal_Int32 nKeyPos = (m_aMatrixIter - m_pMatrix->begin());
        m_pMatrix->resize(_nSize);
        m_aMatrixIter = m_pMatrix->begin() + nKeyPos;
        m_aMatrixEnd = m_pMatrix->end();

        // now adjust their positions because a resize invalid all iterators
        ::std::vector<sal_Int32>::const_iterator aIter = aPositions.begin();
        ::std::map<sal_Int32,sal_Bool>::const_iterator aPosChangeIter = aCacheIterToChange.begin();
        for(    aCacheIter = m_aCacheIterators.begin();
                aPosChangeIter != aCacheIterToChange.end();
                ++aPosChangeIter,++aCacheIter)
        {
            if(aPosChangeIter->second)
                aCacheIter->second.aIterator = m_pMatrix->begin() + *aIter++;
        }
    }
    if(!m_nPosition)
    {
        sal_Int32 nNewSt = 1;
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
// XRow
sal_Bool ORowSetCache::wasNull(  )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    return (*(*m_aMatrixIter))[m_nLastColumnIndex].isNull();
}
// -----------------------------------------------------------------------------
ORowSetValue ORowSetCache::getValue(sal_Int32 columnIndex)
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
::rtl::OUString ORowSetCache::getString( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::getBoolean( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int8 ORowSetCache::getByte( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int16 ORowSetCache::getShort( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int32 ORowSetCache::getInt( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int64 ORowSetCache::getLong( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
float ORowSetCache::getFloat( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
double ORowSetCache::getDouble( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > ORowSetCache::getBytes( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date ORowSetCache::getDate( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time ORowSetCache::getTime( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime ORowSetCache::getTimestamp( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > ORowSetCache::getBinaryStream( sal_Int32 columnIndex )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return new ::comphelper::SequenceInputStream((*(*m_aMatrixIter))[m_nLastColumnIndex].getSequence());
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > ORowSetCache::getCharacterStream( sal_Int32 columnIndex )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return new ::comphelper::SequenceInputStream((*(*m_aMatrixIter))[m_nLastColumnIndex].getSequence());
}
// -------------------------------------------------------------------------
Any ORowSetCache::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    return Any();
}
// -------------------------------------------------------------------------
Reference< XRef > ORowSetCache::getRef( sal_Int32 columnIndex )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    return Reference< XRef >();
}
// -------------------------------------------------------------------------
Reference< XBlob > ORowSetCache::getBlob( sal_Int32 columnIndex )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    return Reference< XBlob >();
}
// -------------------------------------------------------------------------
Reference< XClob > ORowSetCache::getClob( sal_Int32 columnIndex )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    return Reference< XClob >();
}
// -------------------------------------------------------------------------
Reference< XArray > ORowSetCache::getArray( sal_Int32 columnIndex )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    return Reference< XArray >();
}
// -------------------------------------------------------------------------

// ::com::sun::star::sdbcx::XRowLocate
Any ORowSetCache::getBookmark(  )
{
    if(m_bAfterLast)
        throwFunctionSequenceException(m_xSet.get());

    if(m_aMatrixIter == m_pMatrix->end() || !(*m_aMatrixIter).isValid())
    {
        return Any(); // this is allowed here because the rowset knowns what it is doing
    }

    switch((*(*m_aMatrixIter))[0].getTypeKind())
    {
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
            return makeAny((sal_Int32)(*(*m_aMatrixIter))[0]);
            break;
        default:
            if((*(*m_aMatrixIter))[0].isNull())
                (*(*m_aMatrixIter))[0] = m_pCacheSet->getBookmark(*m_aMatrixIter);
            return (*(*m_aMatrixIter))[0].getAny();
    }
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::moveToBookmark( const Any& bookmark )
{
//  if(m_bInserted)
//      m_bInserted = sal_False;

    if ( m_pCacheSet->moveToBookmark(bookmark) )
    {
        m_bDeleted  = m_bBeforeFirst    = sal_False;
        m_nPosition = m_pCacheSet->getRow();

        checkPositionFlags();

        if(!m_bAfterLast)
        {
            moveWindow();
            checkPositionFlags();
            if ( !m_bAfterLast )
            {
                m_aMatrixIter = calcPosition();
                OSL_ENSURE(m_aMatrixIter->isValid(),"Iterator after moveto bookmark not valid");
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
    sal_Bool bRet;
    if(bRet = moveToBookmark(bookmark))
    {
        m_nPosition = m_pCacheSet->getRow() + rows;
        absolute(m_nPosition);
        //  for(sal_Int32 i=0;i<rows && m_aMatrixIter != m_pMatrix->end();++i,++m_aMatrixIter) ;

        bRet = m_aMatrixIter != m_pMatrix->end() && (*m_aMatrixIter).isValid();
    }

    return bRet;
}
// -------------------------------------------------------------------------
sal_Int32 ORowSetCache::compareBookmarks( const Any& first, const Any& second )
{
    return (!first.hasValue() || !second.hasValue()) ? CompareBookmark::NOT_COMPARABLE : m_pCacheSet->compareBookmarks(first,second);
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
// -------------------------------------------------------------------------
// XRowUpdate
void ORowSetCache::updateNull( sal_Int32 columnIndex )
{
    updateValue(columnIndex,ORowSetValue());
}
// -----------------------------------------------------------------------------
void ORowSetCache::updateValue(sal_Int32 columnIndex,const ORowSetValue& x)
{
    checkUpdateConditions(columnIndex);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void ORowSetCache::updateBinaryStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length )
{
    checkUpdateConditions(columnIndex);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    Sequence<sal_Int8> aSeq;
    if(x.is())
        x->readSomeBytes(aSeq,length);
    updateValue(columnIndex,aSeq);
}
// -------------------------------------------------------------------------
void ORowSetCache::updateCharacterStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length )
{
    checkUpdateConditions(columnIndex);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    Sequence<sal_Int8> aSeq;
    if(x.is())
        x->readSomeBytes(aSeq,length);

    updateValue(columnIndex,aSeq);
}
// -------------------------------------------------------------------------
void ORowSetCache::updateObject( sal_Int32 columnIndex, const Any& x )
{
    checkUpdateConditions(columnIndex);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void ORowSetCache::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 scale )
{
    checkUpdateConditions(columnIndex);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;

}
// -------------------------------------------------------------------------
// XResultSet
sal_Bool ORowSetCache::next(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

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
    //  return !m_nPosition;
    ::osl::MutexGuard aGuard( m_aRowCountMutex );
    return m_bBeforeFirst;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::isAfterLast(  )
{
    //  return m_pCacheSet->isAfterLast();
    //  return m_bRowCountFinal ? m_bAfterLast : m_pCacheSet->isAfterLast();
    ::osl::MutexGuard aGuard( m_aRowCountMutex );
    return m_bAfterLast;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::isFirst(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );
    return m_nPosition == 1; // ask resultset for
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::isLast(  )
{
    //  return m_bRowCountFinal ? (m_nPosition==m_nRowCount) : m_pCacheSet->isLast();
    ::osl::MutexGuard aGuard( m_aRowCountMutex );
    return m_nPosition == m_nRowCount;
}
// -------------------------------------------------------------------------
void ORowSetCache::beforeFirst(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(!m_bBeforeFirst)
    {
        m_bAfterLast    = sal_False;
        m_nPosition     = 0;
        m_bBeforeFirst  = sal_True;
        m_pCacheSet->beforeFirst();
        moveWindow();
        m_aMatrixIter = m_pMatrix->end();
    }
}
// -------------------------------------------------------------------------
void ORowSetCache::afterLast(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(!m_bAfterLast)
    {
        m_bBeforeFirst = sal_False;
        m_bAfterLast = sal_True;

        if(!m_bRowCountFinal)
        {
            m_pCacheSet->last();
            m_bRowCountFinal = sal_True;
            m_nRowCount = m_pCacheSet->getRow();// + 1 removed
        }
        m_pCacheSet->afterLast();

        m_nPosition = 0;
        m_aMatrixIter = m_pMatrix->end();
    }
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::fillMatrix(sal_Int32& _nNewStartPos,sal_Int32 _nNewEndPos)
{
    OSL_ENSURE(_nNewStartPos != _nNewEndPos,"ORowSetCache::fillMatrix: StartPos and EndPos can not be equal!");
    // fill the whole window with new data
    ORowSetMatrix::iterator aIter = m_pMatrix->begin();
    sal_Bool bCheck = m_pCacheSet->absolute(_nNewStartPos); // -1 no need to

    sal_Int32 i=_nNewStartPos;
    for(;i<_nNewEndPos;++i,++aIter)
    {
        if(bCheck)
        {
            if(!aIter->isValid())
                *aIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
            m_pCacheSet->fillValueRow(*aIter,i);
        }
        else
        {   // there are no more rows found so we can fetch some before start

            if(!m_bRowCountFinal)
            {
                if(m_pCacheSet->previous()) // because we stand after the last row
                    m_nRowCount = m_pCacheSet->getRow(); // here we have the row count
                if(!m_nRowCount)
                    m_nRowCount = i-1; // it can be that getRow return zero
                m_bRowCountFinal = sal_True;
            }
            if(m_nRowCount > m_nFetchSize)
            {
                ORowSetMatrix::iterator aEnd = aIter;
                sal_Int32 nPos = m_nRowCount - m_nFetchSize + 1;
                _nNewStartPos = nPos;
                bCheck = m_pCacheSet->absolute(_nNewStartPos);

                for(;bCheck && aIter != m_pMatrix->end();++aIter)
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
                    ::std::rotate(m_pMatrix->begin(),aEnd,m_pMatrix->end());
            }
            break;
        }
        bCheck = m_pCacheSet->next();
    }
    //  m_nStartPos = _nNewStartPos;
    // we have to read one row forward to enshure that we know when we are on last row
    // but only when we don't know it already
    if(!m_bRowCountFinal)
    {
        if(!m_pCacheSet->next())
        {
            if(m_pCacheSet->previous()) // because we stand after the last row
                m_nRowCount = m_pCacheSet->getRow(); // here we have the row count
            m_bRowCountFinal = sal_True;
        }
        else
           m_nRowCount = std::max(i,m_nRowCount);

    }
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
                //  aEnd = m_pMatrix->begin() + (sal_Int32)(m_nFetchSize*0.5);
                OSL_ENSURE((nNewEndPos - m_nStartPos - nNewStartPos) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
                aEnd = m_pMatrix->begin() + nNewEndPos - m_nStartPos - nNewStartPos;
                aIter = aEnd;
                m_nStartPos = 0;
            }
            else
            {
                OSL_ENSURE((nNewEndPos - m_nStartPos -1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
                aEnd = m_pMatrix->begin() + (nNewEndPos - m_nStartPos)-1;
                aIter = m_pMatrix->begin() + (nNewEndPos - m_nStartPos)-1;
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
                sal_Int16 nNewDist = aEnd - m_pMatrix->begin();
                sal_Int16 nOffSet = m_pMatrix->end() - aEnd;
                ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
                for(;aCacheIter != m_aCacheIterators.end();++aCacheIter)
                {
                    if (    aCacheIter->second.aIterator
                        &&  aCacheIter->second.aIterator != m_aInsertRow
                        && !m_bInserted && !m_bModified )
                    {
                        sal_Int16 nDist = (aCacheIter->second.aIterator - m_pMatrix->begin());
                        if ( nDist >= nNewDist )
                        {
                            aCacheIter->second.aIterator = NULL;
                        }
                        else
                        {
#if OSL_DEBUG_LEVEL > 0
                            ORowSetMatrix::iterator aOldPos = aCacheIter->second.aIterator;
#endif
                            aCacheIter->second.aIterator += nOffSet;
#if OSL_DEBUG_LEVEL > 0
                            ORowSetMatrix::iterator aCurrentPos = aCacheIter->second.aIterator;
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

                m_pCacheSet->beforeFirst();

                sal_Bool bCheck;
                ORowSetMatrix::iterator aIter = m_pMatrix->begin();
                for(sal_Int32 i=0;i<m_nFetchSize;++i,++aIter)
                {
                    if(bCheck = m_pCacheSet->next())
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
                sal_Bool bOk;
                if(bOk = m_pCacheSet->absolute(m_nPosition))
                {
                    *m_aMatrixIter = new ORowSetValueVector(m_xMetaData->getColumnCount());
                    m_pCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
                    // we have to read one row forward to enshure that we know when we are on last row
                    // but only when we don't know it already
                    if(!m_bRowCountFinal && (bOk = m_pCacheSet->absolute(m_nPosition+1)))
                        m_nRowCount = std::max(sal_Int32(m_nPosition+1),m_nRowCount);
                }
                if(!bOk)
                {
                    if(!m_bRowCountFinal)
                    {
                        // because we stand after the last row
                        m_nRowCount = m_pCacheSet->previous() ? m_pCacheSet->getRow() : 0;//  + 1 removed
                        m_bRowCountFinal = sal_True;
                    }
                }
            }
        }
        else if(nNewStartPos < (m_nStartPos+m_nFetchSize))
        {   // position behind window but the region is overlapping
            // the rows from begin() to (begin + nNewStartPos - m_nStartPos) can be refilled with the new rows
            // the rows behind this can be reused
            ORowSetMatrix::iterator aIter = m_pMatrix->begin();
            OSL_ENSURE((nNewStartPos - m_nStartPos - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
            ORowSetMatrix::iterator aEnd  = m_pMatrix->begin() + nNewStartPos - m_nStartPos - 1;

            sal_Int32 nPos = m_nStartPos + m_nFetchSize + 1;
            sal_Bool bCheck = m_pCacheSet->absolute(nPos);
            bCheck = fill(aIter,aEnd,nPos,bCheck); // refill the region wew don't need anymore

//          // we know that this is the current maximal rowcount here
//          if ( !m_bRowCountFinal && bCheck )
//              m_nRowCount = std::max(nPos,m_nRowCount);
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
                rotateCacheIterator(aIter - m_pMatrix->begin());
                m_nStartPos = nNewStartPos - 1; // must be -1
                // now I can say how many rows we have
                if(!bOk)
                {
                    m_pCacheSet->previous(); // because we stand after the last row
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
                rotateCacheIterator(aIter - m_pMatrix->begin());

                if ( !m_bRowCountFinal )
                {
                    m_pCacheSet->previous();                            // because we stand after the last row
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
                bCheck  = m_pCacheSet->absolute(m_nStartPos);
                for(; !aIter->isValid() && bCheck;++aIter)
                {
                    if(bCheck = m_pCacheSet->next()) // resultset stands on right position
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
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

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

        OSL_ENSURE(m_bBeforeFirst || (m_bNew && m_bInserted),"ORowSetCache::first return false and BeforeFirst isn't true");
        m_aMatrixIter = m_pMatrix->end();
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::last(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

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
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    return (isBeforeFirst() || isAfterLast()) ? 0 : m_nPosition;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::absolute( sal_Int32 row )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(!row )
        throw SQLException();

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
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    sal_Bool bErg = sal_True;
    if(rows)
    {
        if(m_bBeforeFirst || (m_bRowCountFinal && m_bAfterLast))
            throw SQLException();

        if(m_nPosition + rows)
        {
            bErg = absolute(m_nPosition + rows);

            bErg = bErg && !isAfterLast() && !isBeforeFirst();
        }
        else
            bErg = !(m_bBeforeFirst = sal_True);
    }
    return bErg;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::previous(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

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
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(isAfterLast())
        throw SQLException();
    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"refreshRow() called for invalid row!");
    m_pCacheSet->refreshRow();
    m_pCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
    if ( m_bInserted )
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
sal_Bool ORowSetCache::rowDeleted(  )
{
    //  return m_pCacheSet->rowDeleted();
    return m_bDeleted;
}
// -------------------------------------------------------------------------
Reference< XInterface > ORowSetCache::getStatement(  )
{
    return m_pCacheSet->getStatement();
}
// -------------------------------------------------------------------------

// XResultSetUpdate
sal_Bool ORowSetCache::insertRow(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if ( !m_bInserted || !m_aInsertRow->isValid() )
        throw SQLException();

    sal_Bool bRet;
    m_pCacheSet->insertRow(*m_aInsertRow,m_aUpdateTable);

    if ( bRet = rowInserted() )
    {
        ++m_nRowCount;
        Any aBookmark = (*(*m_aInsertRow))[0].makeAny();
        m_bAfterLast = m_bBeforeFirst = sal_False;
        if(aBookmark.hasValue())
            moveToBookmark(aBookmark);
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
    m_bInserted = sal_False;
}
// -------------------------------------------------------------------------
void ORowSetCache::cancelRowModification()
{
    resetInsertRow(sal_False);

    // clear the insertrow references   -> implies that the current row of the rowset changes as well
    ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
    for(;aCacheIter != m_aCacheIterators.end();++aCacheIter)
    {
        if(aCacheIter->second.aIterator)
        {
            ORowSetMatrix::iterator aOldIter = aCacheIter->second.aIterator;
            if(aOldIter == m_aInsertRow)
                aCacheIter->second.aIterator = NULL;
        }
    }
}
// -------------------------------------------------------------------------
void ORowSetCache::updateRow(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(isAfterLast() || isBeforeFirst())
        throw SQLException();

    m_pCacheSet->updateRow(*m_aInsertRow,*m_aMatrixIter,m_aUpdateTable);

    clearInsertRow();

    // we don't need to repositioning here refresh will do it for us
    m_bModified = sal_False;
    refreshRow(  );
}
// -------------------------------------------------------------------------
void ORowSetCache::updateRow( ORowSetMatrix::iterator& _rUpdateRow )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(isAfterLast() || isBeforeFirst())
        throw SQLException();

    Any aBookmark = (*(*_rUpdateRow))[0].makeAny();
    OSL_ENSURE(aBookmark.hasValue(),"Bookmark must have a value!");
    // here we don't have to reposition our CacheSet, when we try to update a row,
    // the row was already fetched
    moveToBookmark(aBookmark);
    m_pCacheSet->updateRow(*_rUpdateRow,*m_aMatrixIter,m_aUpdateTable);
    //  *(*m_aMatrixIter) = *(*_rUpdateRow);
    // refetch the whole row
    (*m_aMatrixIter) = NULL;
    moveToBookmark(aBookmark);

    //  moveToBookmark((*(*m_aInsertRow))[0].makeAny());
//  if(m_pCacheSet->rowUpdated())
//      *m_aMatrixIter = m_aInsertRow;
    m_bModified = sal_False;
    //  refreshRow(  );
}
// -------------------------------------------------------------------------
void ORowSetCache::deleteRow(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(isAfterLast() || isBeforeFirst())
        throw SQLException();

    //  m_pCacheSet->absolute(m_nPosition);
    m_pCacheSet->deleteRow(*m_aMatrixIter,m_aUpdateTable);
    if(m_bDeleted = m_pCacheSet->rowDeleted())
    {
        --m_nRowCount;
        OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
        ORowSetMatrix::iterator aPos = calcPosition();
        (*aPos)   = NULL;
        //  (*m_pMatrix)[(m_nPosition - m_nStartPos)] = NULL; // set the deleted row to NULL


        for(++aPos;aPos->isValid() && aPos != m_pMatrix->end();++aPos)
        {
            *(aPos-1) = *aPos;
            (*aPos)   = NULL;
        }
        m_aMatrixIter = m_pMatrix->end();

        --m_nPosition;
    }
}
// -------------------------------------------------------------------------
void ORowSetCache::cancelRowUpdates(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    m_bNew      = m_bInserted = m_bModified = sal_False;
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
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    m_bNew      = sal_True;
    m_bInserted = sal_True;
    m_bUpdated  = m_bDeleted = m_bAfterLast = sal_False;

    m_aInsertRow = m_pInsertMatrix->begin();
    if(!m_aInsertRow->isValid())
        *m_aInsertRow = new ORowSetValueVector(m_xMetaData->getColumnCount());

    // we don't unbound the bookmark column
    ORowSetValueVector::iterator aIter = (*m_aInsertRow)->begin()+1;
    for(;aIter != (*m_aInsertRow)->end();++aIter)
    {
        aIter->setBound(sal_False);
        aIter->setModified(sal_False);
        aIter->setNull();
    }
}
// -------------------------------------------------------------------------
void ORowSetCache::moveToCurrentRow(  )
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

//  if(m_bInserted)
//  {
//      m_pCacheSet->moveToCurrentRow();
//      m_bInserted = sal_False;
//  }
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > ORowSetCache::deleteRows( const Sequence< Any >& rows )
{
    // TODO impl. a better version which is faster than tis one

    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    Sequence< sal_Int32 > aRet(rows.getLength());
    sal_Int32 *pRet = aRet.getArray();

    const Any *pBegin   = rows.getConstArray();
    const Any *pEnd     = pBegin + rows.getLength();

    sal_Int32 nOldPosition;
    for(;pBegin != pEnd;++pBegin,++pRet)
    {
        // first we have to position our own and then we have to position our CacheSet again,
        // it could be repositioned in the moveToBookmark call
        if ( moveToBookmark(*pBegin) && m_pCacheSet->moveToBookmark(*pBegin) )
        {
            nOldPosition = m_nPosition;
            deleteRow();
            *pRet = (nOldPosition != m_nPosition) ? 1 : 0;
            nOldPosition = m_nPosition;
        }
    }
    return aRet;
}

// -------------------------------------------------------------------------
ORowSetCacheIterator ORowSetCache::createIterator()
{
    ORowSetCacheIterator_Helper aHelper;
    aHelper.aIterator = m_pMatrix->end();
    return ORowSetCacheIterator(m_aCacheIterators.insert(m_aCacheIterators.begin(),ORowSetCacheMap::value_type(m_aCacheIterators.size()+1,aHelper)),this);
}
// -----------------------------------------------------------------------------
void ORowSetCache::rotateCacheIterator(sal_Int16 _nDist)
{
    if(_nDist)
    {
        // now correct the iterator in our iterator vector
        ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
        for(;aCacheIter != m_aCacheIterators.end();++aCacheIter)
        {
            if(aCacheIter->second.aIterator && aCacheIter->second.aIterator != m_aInsertRow && !m_bInserted && !m_bModified)
            {
                sal_Int16 nDist = (aCacheIter->second.aIterator - m_pMatrix->begin());
                if(nDist < _nDist)
                {
                    aCacheIter->second.aIterator = NULL;
                }
                else
                {
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
    ORowSetValueVector::iterator aIter = (*m_aInsertRow)->begin();
    for(;aIter != (*m_aInsertRow)->end();++aIter)
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
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throwFunctionSequenceException(m_xSet.get());
}
// -----------------------------------------------------------------------------
sal_Bool ORowSetCache::checkJoin(const Reference< XConnection>& _xConnection,
                                 const Reference< XSQLQueryComposer >& _xComposer,
                                 const ::rtl::OUString& _sUpdateTableName )
{
    sal_Bool bOk = sal_False;
    ::rtl::OUString sSql = _xComposer->getQuery();
    ::rtl::OUString sErrorMsg;
    ::connectivity::OSQLParser aSqlParser(m_xServiceFactory);
    ::connectivity::OSQLParseNode* pSqlParseNode = aSqlParser.parseTree(sErrorMsg,sSql);
    if(pSqlParseNode)
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
                    OSQLParseNode* pTableRef = pJoin->getByRule(::connectivity::OSQLParseNode::qualified_join);
                    if(bLeftSide)
                        pTableRef = pJoin->getChild(0);
                    else
                        pTableRef = pJoin->getChild(3);
                    OSL_ENSURE(SQL_ISRULE(pTableRef,table_ref),"Must be a tableref here!");

                    ::rtl::OUString sTableRange;
                    if(pTableRef->count() == 4)
                        sTableRange = pTableRef->getChild(2)->getTokenValue(); // Tabellenrange an Pos 2
                    if(!sTableRange.getLength())
                        pTableRef->getChild(0)->parseNodeToStr(sTableRange,_xConnection->getMetaData(),NULL,sal_False,sal_False);
                    bOk =  sTableRange == _sUpdateTableName;
                }
            }

        }
        delete pSqlParseNode;
    }
    return bOk;
}
// -----------------------------------------------------------------------------
void ORowSetCache::clearInsertRow()
{
    // we don't unbound the bookmark column
    ORowSetValueVector::iterator aIter = (*m_aInsertRow)->begin()+1;
    for(;aIter != (*m_aInsertRow)->end();++aIter)
    {
        aIter->setBound(sal_False);
        aIter->setModified(sal_False);
        aIter->setNull();
    }
}
// -----------------------------------------------------------------------------
ORowSetMatrix::iterator ORowSetCache::calcPosition() const
{
    sal_Int32 nValue = (m_nPosition - m_nStartPos) - 1;
    OSL_ENSURE(nValue >= 0 && nValue < m_pMatrix->size(),"Position is invalid!");
    return (nValue < 0) ? m_pMatrix->end() : (m_pMatrix->begin() + nValue);
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
    for (TOldRowSetRows::iterator aOldRowIter = m_aOldRows.begin(); aOldRowIter != m_aOldRows.end(); ++aOldRowIter)
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
    for (TOldRowSetRows::iterator aOldRowIter = m_aOldRows.begin(); aOldRowIter != m_aOldRows.end(); ++aOldRowIter)
    {
        if ( aOldRowIter->isValid() && aOldRowIter->getBody().getRow().isValid() )
            aOldRowIter->getBody().setRow(new ORowSetValueVector(aOldRowIter->getBody().getRow().getBody()) );
    }
    sal_Int32 nNewSt = _nNewStartPos;
    sal_Bool bRet = fillMatrix(nNewSt,_nNewEndPos);
    m_nStartPos = nNewSt - 1;
    rotateCacheIterator(m_nFetchSize+1); // forces that every iterator will be set to null
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
            for (TOldRowSetRows::iterator aOldRowIter = m_aOldRows.begin(); aOldRowIter != m_aOldRows.end(); ++aOldRowIter)
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
