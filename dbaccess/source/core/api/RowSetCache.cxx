/*************************************************************************
 *
 *  $RCSfile: RowSetCache.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-02 11:14:53 $
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
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_COMPAREBOOKMARK_HPP_
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#endif

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
    ,m_bFirst(sal_False)
    ,m_bLast(sal_False)
    ,m_bRowCountFinal(sal_False)
    ,m_bInserted(sal_False)
    ,m_bDeleted(sal_False)
    ,m_bUpdated(sal_False)
    ,m_xMetaData(Reference< XResultSetMetaDataSupplier >(_xRs,UNO_QUERY)->getMetaData())
    ,m_nFetchSize(0)
    ,m_bNew(_bNew)
    ,m_bModified(_bModified)
    ,m_pMatrix(NULL)
    ,m_pInsertMatrix(NULL)
{
    // check if all keys of the updateable table are fetched
    sal_Bool bAllKeysFound = sal_False;

    ::rtl::OUString aUpdateTableName = _rUpdateTableName;
    Reference< XConnection> xConnection;
    if(_xComposer.is())
    {
        Reference<XTablesSupplier> xTabSup(_xComposer,UNO_QUERY);
        OSL_ENSURE(xTabSup.is(),"ORowSet::execute composer isn't a tablesupplier!");
        Reference<XNameAccess> xTables = xTabSup->getTables();


        if(_rUpdateTableName.getLength() && xTables->hasByName(_rUpdateTableName))
            xTables->getByName(_rUpdateTableName) >>= m_aUpdateTable;
        else
        {
            aUpdateTableName = xTables->getElementNames()[0];
            xTables->getByName(aUpdateTableName) >>= m_aUpdateTable;
        }

        if(m_aUpdateTable.is())
        {
            Reference<XKeysSupplier> xKeys(m_aUpdateTable,UNO_QUERY);
            if(xKeys.is())
            {
                Reference< XIndexAccess> xKeyIndex = xKeys->getKeys();
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
                    bAllKeysFound = (sal_Int32)aColumnNames.size() == xColumns->getElementNames().getLength();
                }
            }
        }
    }
    Reference< XPropertySet> xProp(_xRs,UNO_QUERY);

    // first check if resultset is bookmarkable
    if(xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_ISBOOKMARKABLE) && any2bool(xProp->getPropertyValue(PROPERTY_ISBOOKMARKABLE)))
    {
        m_pCacheSet = new OBookmarkSet(_xRs);

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
    else
    {
        // || !(comphelper::hasProperty(PROPERTY_CANUPDATEINSERTEDROWS,xProp) && any2bool(xProp->getPropertyValue(PROPERTY_CANUPDATEINSERTEDROWS)))
        if(!bAllKeysFound || (xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_RESULTSETTYPE) &&
            comphelper::getINT32(xProp->getPropertyValue(PROPERTY_RESULTSETTYPE)) == ResultSetType::FORWARD_ONLY) )
        {
            m_pCacheSet = new OStaticSet(_xRs);
//          if(bAllKeysFound)
//              m_nPrivileges = Privilege::INSERT | Privilege::DELETE | Privilege::UPDATE;
//          else
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

            m_pCacheSet = new OKeySet(_xRs,m_aUpdateTable,aUpdateTableName ,_xComposer);

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

    }
    // last check
    if(xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_RESULTSETCONCURRENCY) &&
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
    m_xStatement    = NULL;
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
        ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
        for(;aCacheIter != m_aCacheIterators.end();++aCacheIter)
        {
            if(!aCacheIter->second.aIterator)
                continue;
            if(aCacheIter->second.aIterator != m_aInsertRow && !m_bInserted && !m_bModified)
            {
                sal_Int16 nDist = (aCacheIter->second.aIterator - m_pMatrix->begin());
                aPositions.push_back(nDist);
            }
        }
        sal_Int32 nKeyPos = (m_aMatrixIter - m_pMatrix->begin());
        m_pMatrix->resize(_nSize);
        m_aMatrixIter = m_pMatrix->begin() + nKeyPos;
        m_aMatrixEnd = m_pMatrix->end();
        aCacheIter = m_aCacheIterators.begin();
        ::std::vector<sal_Int32>::iterator aIter = aPositions.begin();
        for(;aCacheIter != m_aCacheIterators.end();++aCacheIter)
        {
            if(!aCacheIter->second.aIterator)
                continue;
            if(aCacheIter->second.aIterator != m_aInsertRow && !m_bInserted && !m_bModified)
            {
                aCacheIter->second.aIterator = m_pMatrix->begin() + *aIter++;
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
}
// -------------------------------------------------------------------------
// OComponentHelper
void SAL_CALL ORowSetCache::disposing(void)
{
}
// -------------------------------------------------------------------------

// ::com::sun::star::lang::XEventListener
void SAL_CALL ORowSetCache::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(RuntimeException)
{
}
// -------------------------------------------------------------------------

// XCloseable
void SAL_CALL ORowSetCache::close(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

// XResultSetMetaDataSupplier
Reference< XResultSetMetaData > SAL_CALL ORowSetCache::getMetaData(  ) throw(SQLException, RuntimeException)
{
    return m_xMetaData;
}
// -------------------------------------------------------------------------

// ::com::sun::star::sdbcx::XColumnsSupplier
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL ORowSetCache::getColumns(  ) throw(RuntimeException)
{
    return Reference< ::com::sun::star::container::XNameAccess >();
}
// -------------------------------------------------------------------------

// XRow
sal_Bool SAL_CALL ORowSetCache::wasNull(  ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    return (*(*m_aMatrixIter))[m_nLastColumnIndex].isNull();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORowSetCache::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    ::rtl::OUString aRet;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL ORowSetCache::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL ORowSetCache::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetCache::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL ORowSetCache::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
float SAL_CALL ORowSetCache::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
double SAL_CALL ORowSetCache::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL ORowSetCache::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL ORowSetCache::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL ORowSetCache::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL ORowSetCache::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return (*(*m_aMatrixIter))[m_nLastColumnIndex];
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSetCache::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return new ::comphelper::SequenceInputStream((*(*m_aMatrixIter))[m_nLastColumnIndex].getSequence());
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSetCache::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"Iterator is equal end()");

    m_nLastColumnIndex = columnIndex;
    return new ::comphelper::SequenceInputStream((*(*m_aMatrixIter))[m_nLastColumnIndex].getSequence());
}
// -------------------------------------------------------------------------
Any SAL_CALL ORowSetCache::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    return Any();
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL ORowSetCache::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    return Reference< XRef >();
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ORowSetCache::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    return Reference< XBlob >();
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL ORowSetCache::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    return Reference< XClob >();
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ORowSetCache::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

    return Reference< XArray >();
}
// -------------------------------------------------------------------------

// ::com::sun::star::sdbcx::XRowLocate
Any SAL_CALL ORowSetCache::getBookmark(  ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast)
        throw FunctionSequenceException(m_xSet.get());

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
sal_Bool SAL_CALL ORowSetCache::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
//  if(m_bInserted)
//      m_bInserted = sal_False;

    if(m_pCacheSet->moveToBookmark(bookmark))
    {
        m_bDeleted  = m_bBeforeFirst    = sal_False;
        m_nPosition = m_pCacheSet->getRow();
        if(m_bRowCountFinal)
        {
            m_bAfterLast    = m_nPosition > m_nRowCount;
            m_bLast         = m_nPosition == m_nRowCount;
            if(m_bAfterLast)
                m_nPosition = 0;//m_nRowCount;
        }
        if(!m_bAfterLast)
        {
            moveWindow();
            if(m_bRowCountFinal) // check again
            {
                m_bAfterLast    = m_nPosition > m_nRowCount;
                m_bLast         = m_nPosition == m_nRowCount;
                if(m_bAfterLast)
                    m_nPosition = 0;//m_nRowCount;
            }
            if(!m_bAfterLast)
                m_aMatrixIter = m_pMatrix->begin() + (m_nPosition - m_nStartPos) - 1; // must be -1
            else
                m_aMatrixIter = m_pMatrix->end();
        }
        else
            m_aMatrixIter = m_pMatrix->end();
    }

    return m_aMatrixIter != m_pMatrix->end() && (*m_aMatrixIter).isValid();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
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
sal_Int32 SAL_CALL ORowSetCache::compareBookmarks( const Any& first, const Any& second ) throw(SQLException, RuntimeException)
{
    if(!first.hasValue() || !second.hasValue())
        return CompareBookmark::NOT_COMPARABLE;

    return m_pCacheSet->compareBookmarks(first,second);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    return m_pCacheSet->hasOrderedBookmarks();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetCache::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    return m_pCacheSet->hashBookmark(bookmark);
}
// -------------------------------------------------------------------------

// XRowUpdate
void SAL_CALL ORowSetCache::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex].setNull();
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = (sal_Int32)x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = (sal_Int32)x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateFloat( sal_Int32 columnIndex, float x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateDouble( sal_Int32 columnIndex, double x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex].setFromDouble(x,m_xMetaData->getColumnType(columnIndex));
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex < 1 || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateBinaryStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex < 1 || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    Sequence<sal_Int8> aSeq;
    if(x.is())
        x->readSomeBytes(aSeq,length);
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = aSeq;
    //  (*(*m_aInsertRow))[columnIndex].setTypeKind(DataType::BLOB);
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateCharacterStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex < 1 || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    Sequence<sal_Int8> aSeq;
    if(x.is())
        x->readSomeBytes(aSeq,length);

    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = aSeq;
    //  (*(*m_aInsertRow))[columnIndex].setTypeKind(DataType::CLOB);
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex < 1 || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    if(m_bAfterLast || columnIndex < 1 || columnIndex >= (sal_Int32)(*m_aInsertRow)->size())
        throw FunctionSequenceException(m_xSet.get());

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    (*(*m_aInsertRow))[columnIndex].setBound(sal_True);
    (*(*m_aInsertRow))[columnIndex] = x;
    (*(*m_aInsertRow))[columnIndex].setModified();
    m_bModified = sal_True;
}
// -------------------------------------------------------------------------
// XResultSet
sal_Bool SAL_CALL ORowSetCache::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(isAfterLast())
        return sal_False;

    m_bBeforeFirst = sal_False;
    ++m_nPosition;
    // after we increment the position we have to check if we are already after the last row
    if(m_bRowCountFinal)
    {
        m_bAfterLast    = m_nPosition > m_nRowCount;
        m_bLast         = m_nPosition == m_nRowCount;
        if(m_bAfterLast)
            m_nPosition = 0;//m_nRowCount;
    }
    if(!m_bAfterLast)
    {

        moveWindow();
        //  readForward();

        OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
        m_aMatrixIter = m_pMatrix->begin() + m_nPosition - m_nStartPos -1; // -1 because rows start at zero
        if(m_bRowCountFinal)
        {
            m_bAfterLast    = m_nPosition > m_nRowCount;
            m_bLast         = m_nPosition == m_nRowCount;
            if(m_bAfterLast)
                m_nPosition = 0;//m_nRowCount;
        }
    }

    return !m_bAfterLast;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    //  return !m_nPosition;
    return m_bBeforeFirst;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    //  return m_pCacheSet->isAfterLast();
    //  return m_bRowCountFinal ? m_bAfterLast : m_pCacheSet->isAfterLast();
    return m_bAfterLast;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::isFirst(  ) throw(SQLException, RuntimeException)
{
    return m_nPosition == 1; // ask resultset for
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::isLast(  ) throw(SQLException, RuntimeException)
{
    //  return m_bRowCountFinal ? (m_nPosition==m_nRowCount) : m_pCacheSet->isLast();
    return m_nPosition==m_nRowCount;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

//  if(m_bInserted)
//      m_bInserted = sal_False;

    m_bAfterLast    = m_bLast = sal_False;
    m_nPosition     = 0;
    m_bBeforeFirst  = sal_True;
    m_pCacheSet->beforeFirst();
    moveWindow();
    m_aMatrixIter = m_pMatrix->end();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::afterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

//  if(m_bInserted)
//      m_bInserted = sal_False;

    m_bBeforeFirst = m_bLast = sal_False;
    m_bAfterLast = sal_True;

    m_pCacheSet->last();
    if(!m_bRowCountFinal)
    {
        m_bRowCountFinal = sal_True;
        m_nRowCount = m_pCacheSet->getRow();// + 1 removed
    }
    m_pCacheSet->afterLast();

    m_nPosition = 0;
    m_aMatrixIter = m_pMatrix->end();
}
// -------------------------------------------------------------------------
sal_Bool ORowSetCache::fillMatrix(sal_Int32& _nNewStartPos,sal_Int32 _nNewEndPos)
{
    // fill the whole window with new data
    ORowSetMatrix::iterator aIter = m_pMatrix->begin();
    sal_Bool bCheck = m_pCacheSet->absolute(_nNewStartPos); // -1 no need to

    sal_Int32 i=_nNewStartPos;
    for(;i<_nNewEndPos;++i,++aIter)
    {
        if(bCheck)
        {
            if(!aIter->isValid())
                *aIter = new connectivity::ORowVector< ORowSetValue >(m_xMetaData->getColumnCount());
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
                            *aIter = new connectivity::ORowVector< ORowSetValue >(m_xMetaData->getColumnCount());
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
           m_nRowCount = max(i,m_nRowCount);

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

    if(m_nPosition <= m_nStartPos)
    {   // the window is behind the new start pos
        if(!m_nStartPos)
            return sal_False;
        // the new position should be the nPos - nFetchSize/2
        if(nNewEndPos > m_nStartPos)
        {   // but the two regions are overlapping
            // fill the rows behind the new end

            ORowSetMatrix::iterator aEnd; // the iterator we need for rotate
            ORowSetMatrix::iterator aIter; // the iterator we fill with new values

            sal_Bool bCheck = sal_True;
            if(nNewStartPos < 1)
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

            if(bCheck)
            {
                sal_Int32 nPos = m_nStartPos+1;
                for(; aIter != m_pMatrix->end() && bCheck;)
                {
                    if(!aIter->isValid())
                        *aIter = new connectivity::ORowVector< ORowSetValue >(m_xMetaData->getColumnCount());
                    m_pCacheSet->fillValueRow(*aIter++,nPos++);
                    bCheck = m_pCacheSet->next();

                }
                ::std::rotate(m_pMatrix->begin(),aEnd,m_pMatrix->end());
                // now correct the iterator in our iterator vector
                //  rotateCacheIterator(aEnd-m_pMatrix->begin());
                sal_Int16 _nDist = aEnd-m_pMatrix->begin();
                ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
                for(;aCacheIter != m_aCacheIterators.end();++aCacheIter)
                {
                    if(!aCacheIter->second.aIterator)
                        continue;
                    if(aCacheIter->second.aIterator != m_aInsertRow && !m_bInserted && !m_bModified)
                    {
                        sal_Int16 nDist = (aCacheIter->second.aIterator - m_pMatrix->begin());
                        if(nDist > _nDist)
                        {
                            aCacheIter->second.aIterator = NULL;
                        }
                        else
                            aCacheIter->second.aIterator += _nDist;
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
                            *aIter = new connectivity::ORowVector< ORowSetValue >(m_xMetaData->getColumnCount());
                        m_pCacheSet->fillValueRow(*aIter,i+1);
                    }
                    else
                        *aIter = NULL;

                }
            }
            else
            {
                sal_Int32 nNewSt = nNewStartPos;
                bRet = fillMatrix(nNewSt,nNewEndPos);
                m_nStartPos = nNewSt - 1;
                rotateCacheIterator(m_nFetchSize+1); // forces that every iterator will be set to null
            }
        }
    }
    else if(m_nPosition > m_nStartPos)
    {   // the new start pos is above the startpos of the window

        if(m_nPosition <= (m_nStartPos+m_nFetchSize))
        {   // position in window
            OSL_ENSURE((m_nPosition - m_nStartPos -1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
            m_aMatrixIter = m_pMatrix->begin() + m_nPosition - m_nStartPos -1;
            if(!m_aMatrixIter->isValid())
            {
                sal_Bool bOk;
                if(bOk = m_pCacheSet->absolute(m_nPosition))
                {
                    *m_aMatrixIter = new connectivity::ORowVector< ORowSetValue >(m_xMetaData->getColumnCount());
                    m_pCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
                    // we have to read one row forward to enshure that we know when we are on last row
                    // but only when we don't know it already
                    if(!m_bRowCountFinal && (bOk = m_pCacheSet->absolute(m_nPosition+1)))
                        m_nRowCount = max(sal_Int32(m_nPosition+1),m_nRowCount);
                }
                if(!bOk)
                {
                    if(!m_bRowCountFinal)
                    {
                        if(m_pCacheSet->previous()) // because we stand after the last row
                            m_nRowCount = m_pCacheSet->getRow();//  + 1 removed
                        else
                            m_nRowCount = 0;
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
            sal_Bool bCheck = m_pCacheSet->absolute(m_nStartPos+m_nFetchSize);
            sal_Int32 nPos = m_nStartPos+m_nFetchSize;
            for(; bCheck && aIter != aEnd;)
            {
                if(bCheck = m_pCacheSet->next()) // resultset stands on right position
                {
                    if(!aIter->isValid())
                        *aIter = new connectivity::ORowVector< ORowSetValue >(m_xMetaData->getColumnCount());
                    m_pCacheSet->fillValueRow(*aIter++,++nPos);
                }
            }
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
            }
            else
            {   // the end was reached before end() so we can set the start before nNewStartPos

                m_nStartPos += (aIter - m_pMatrix->begin());
                ::std::rotate(m_pMatrix->begin(),aIter,m_pMatrix->end());
                // now correct the iterator in our iterator vector
                rotateCacheIterator(aIter - m_pMatrix->begin());

                if(!m_bRowCountFinal)
                {
                    m_pCacheSet->previous();                                    // because we stand after the last row
                    m_nRowCount      = max(m_nRowCount,m_pCacheSet->getRow());  // here we have the row count
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
                        *aIter = new connectivity::ORowVector< ORowSetValue >(m_xMetaData->getColumnCount());
                        m_pCacheSet->fillValueRow(*aIter,++nPos);
                    }
                }
            }
        }
        else // no rows can be reused so fill again
        {
            sal_Int32 nNewSt = nNewStartPos;
            bRet = fillMatrix(nNewSt,nNewEndPos);
            m_nStartPos = nNewSt - 1;
            rotateCacheIterator(m_nFetchSize+1);
        }
    }

    if(!m_bRowCountFinal)
       m_nRowCount = max(m_nPosition,m_nRowCount);
    OSL_ENSURE(m_nStartPos >= 0,"ORowSetCache::moveWindow: m_nStartPos is less than 0!");

    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::first(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

//  if(m_bInserted)
//      m_bInserted = sal_False;

    // first move to the first row
    // then check if the cache window is at the begining
    // when not postionize the window and fill it with data
    // smart moving of the window -> clear only the rows whom are out of range
    sal_Bool bRet = m_pCacheSet->first();
    if(bRet)
    {
        m_bBeforeFirst  = m_bAfterLast = m_bLast = sal_False;
        m_nPosition     = 1;
        moveWindow();
        m_aMatrixIter   = m_pMatrix->begin();
    }
    else
        m_aMatrixIter = m_pMatrix->end();
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

//  if(m_bInserted)
//      m_bInserted = sal_False;

    sal_Bool bRet = m_pCacheSet->last();
    if(bRet)
    {
        m_bBeforeFirst = m_bAfterLast = sal_False;
        if(!m_bRowCountFinal)
        {
            m_bRowCountFinal = sal_True;
            m_nRowCount = m_nPosition = m_pCacheSet->getRow(); // not  + 1
        }
        m_bLast = sal_True;
        m_nPosition = m_pCacheSet->getRow();
        moveWindow();
        // we have to repositioning because moveWindow can modify the cache
        m_pCacheSet->last();
        if(m_nPosition > m_nFetchSize)
            m_aMatrixIter = m_pMatrix->end() -1;
        else
            m_aMatrixIter = m_pMatrix->begin() + m_nPosition - 1;
    }
    else
        m_aMatrixIter = m_pMatrix->end();
#ifdef DEBUG
    if(bRet)
    {
        OSL_ENSURE((*m_aMatrixIter).isValid(),"ORowSetCache::last: Row not valid!");
    }
#endif

    return bRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetCache::getRow(  ) throw(SQLException, RuntimeException)
{
    if(isBeforeFirst() || isAfterLast())
        return 0;
    return m_nPosition;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(!row )
        throw SQLException();

    if(row < 0)
    {
        if(m_bRowCountFinal || last())
        {
            m_nPosition = m_nRowCount + row; // + row because row is negative
            if(m_nPosition < 1)
            {
                m_bBeforeFirst = sal_True;
                m_bAfterLast = m_bFirst = m_bLast = sal_False;
                m_aMatrixIter = m_pMatrix->end();
            }
            else
            {
                m_bBeforeFirst  = sal_False;
                m_bAfterLast    = m_nPosition > m_nRowCount;
                m_bLast         = m_nPosition == m_nRowCount;
                moveWindow();
                OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
                m_aMatrixIter = m_pMatrix->begin() + (m_nPosition - m_nStartPos) - 1; // if row == -1 that means it stands on the last
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
        if(m_bRowCountFinal)
        {
            m_bAfterLast    = m_nPosition > m_nRowCount;
            m_bLast         = m_nPosition == m_nRowCount;
            if(m_bAfterLast)
                m_nPosition = 0;//m_nRowCount;
        }

        if(!m_bAfterLast)
        {
            moveWindow();
            if(m_bRowCountFinal) // check again
            {
                m_bAfterLast    = m_nPosition > m_nRowCount;
                m_bLast         = m_nPosition == m_nRowCount;
                if(m_bAfterLast)
                    m_nPosition = 0;//m_nRowCount;
            }
            if(!m_bAfterLast)
                m_aMatrixIter = m_pMatrix->begin() + (m_nPosition - m_nStartPos) - 1; // must be -1
            else
                m_aMatrixIter = m_pMatrix->end();
        }
        else
            m_aMatrixIter = m_pMatrix->end();
    }

    return !(m_bAfterLast || m_bBeforeFirst);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(!rows)
        return sal_True;
    if(m_bBeforeFirst || (m_bRowCountFinal && m_bAfterLast))
        throw SQLException();

    if(!(m_nPosition + rows))
        return !(m_bBeforeFirst = sal_True);
    sal_Bool bErg = absolute(m_nPosition + rows);

    return bErg && !isAfterLast() && !isBeforeFirst();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(isBeforeFirst())
        return sal_False;

//  if(m_bInserted)
//      m_bInserted = sal_False;

    if(m_bAfterLast)
        return last();

    m_bAfterLast = sal_False;
    --m_nPosition;
    moveWindow();
    OSL_ENSURE(((m_nPosition - m_nStartPos) - 1) < (sal_Int32)m_pMatrix->size(),"Position is behind end()!");
    m_aMatrixIter = m_pMatrix->begin() + m_nPosition - m_nStartPos -1; // must be -1

    if(m_bRowCountFinal)
    {
        m_bLast         = m_nRowCount == m_nPosition;
        m_bAfterLast    = m_nPosition > m_nRowCount;
    }

    if(!m_nPosition)
    {
        m_bBeforeFirst = sal_True;
        m_aMatrixIter = m_pMatrix->end();
        return sal_False;
    }

    return (*m_aMatrixIter).isValid();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(isAfterLast())
        throw SQLException();
    OSL_ENSURE(m_aMatrixIter != m_pMatrix->end(),"refreshRow() called for invalid row!");
    m_pCacheSet->refreshRow();
    m_pCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    return m_pCacheSet->rowUpdated();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::rowInserted(  ) throw(SQLException, RuntimeException)
{
    return m_pCacheSet->rowInserted();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetCache::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    //  return m_pCacheSet->rowDeleted();
    return m_bDeleted;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ORowSetCache::getStatement(  ) throw(SQLException, RuntimeException)
{
    return m_pCacheSet->getStatement();
}
// -------------------------------------------------------------------------

// XResultSetUpdate
void SAL_CALL ORowSetCache::insertRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(!m_bInserted || !m_aInsertRow->isValid())
        throw SQLException();

    m_pCacheSet->insertRow(*m_aInsertRow,m_aUpdateTable);

    if(rowInserted())
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

    m_bNew      = sal_False;
    m_bModified = sal_False;
    m_bInserted = sal_False;

}
// -------------------------------------------------------------------------
void ORowSetCache::cancelInsert()
{
    m_bNew          = sal_False;
    m_bModified     = sal_False;
    m_bInserted     = sal_False;
    m_aInsertRow    = m_aMatrixIter;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(isAfterLast() || isBeforeFirst())
        throw SQLException();

    m_pCacheSet->updateRow(*m_aInsertRow,*m_aMatrixIter,m_aUpdateTable);
    //  moveToBookmark((*(*m_aInsertRow))[0].makeAny());
//  if(m_pCacheSet->rowUpdated())
//      *m_aMatrixIter = m_aInsertRow;
    m_bModified = sal_False;
    refreshRow(  );
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::updateRow( ORowSetMatrix::iterator& _rUpdateRow ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(isAfterLast() || isBeforeFirst())
        throw SQLException();

    moveToBookmark((*(*_rUpdateRow))[0].makeAny());
    m_pCacheSet->updateRow(*_rUpdateRow,*m_aMatrixIter,m_aUpdateTable);
    //  moveToBookmark((*(*m_aInsertRow))[0].makeAny());
//  if(m_pCacheSet->rowUpdated())
//      *m_aMatrixIter = m_aInsertRow;
    m_bModified = sal_False;
    refreshRow(  );
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::deleteRow(  ) throw(SQLException, RuntimeException)
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
        ORowSetMatrix::iterator aPos = m_pMatrix->begin() + m_nPosition - m_nStartPos - 1;
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
void SAL_CALL ORowSetCache::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    m_bNew      = m_bInserted = m_bModified = sal_False;
    m_pCacheSet->absolute(m_nPosition);
    m_pCacheSet->fillValueRow(*m_aMatrixIter,m_nPosition);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

    if(m_bInserted)
        throw SQLException();

    m_bNew      = sal_True;
    m_bInserted = sal_True;
    m_bUpdated  = m_bDeleted = m_bAfterLast = sal_False;

    m_aInsertRow = m_pInsertMatrix->begin();
    if(!m_aInsertRow->isValid())
        *m_aInsertRow = new connectivity::ORowVector< ORowSetValue >(m_xMetaData->getColumnCount());

    // we don't unbound the bookmark column
    connectivity::ORowVector< ORowSetValue >::iterator aIter = (*m_aInsertRow)->begin()+1;
    for(;aIter != (*m_aInsertRow)->end();++aIter)
    {
        aIter->setBound(sal_False);
        aIter->setModified(sal_False);
        aIter->setNull();
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aRowCountMutex );

//  if(m_bInserted)
//  {
//      m_pCacheSet->moveToCurrentRow();
//      m_bInserted = sal_False;
//  }
}
// -------------------------------------------------------------------------

// XRowSet
void SAL_CALL ORowSetCache::execute(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
// ::com::sun::star::util::XCancellable
void SAL_CALL ORowSetCache::cancel(  ) throw(RuntimeException)
{
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL ORowSetCache::deleteRows( const Sequence< Any >& rows ) throw(SQLException, RuntimeException)
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
        if(moveToBookmark(*pBegin))
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
    // now correct the iterator in our iterator vector
    ORowSetCacheMap::iterator aCacheIter = m_aCacheIterators.begin();
    for(;aCacheIter != m_aCacheIterators.end();++aCacheIter)
    {
        if(!aCacheIter->second.aIterator)
            continue;
        if(aCacheIter->second.aIterator != m_aInsertRow && !m_bInserted && !m_bModified)
        {
            sal_Int16 nDist = (aCacheIter->second.aIterator - m_pMatrix->begin());
            if(nDist < _nDist)
            {
                aCacheIter->second.aIterator = NULL;
            }
            else
                aCacheIter->second.aIterator -= _nDist;
        }
    }
}
// -----------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL ORowSetCache::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any ();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetCache::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void ORowSetCache::setUpdateIterator(const ORowSetMatrix::iterator& _rOriginalRow)
{
    m_aInsertRow = m_pInsertMatrix->begin();
    if(!m_aInsertRow->isValid())
        *m_aInsertRow = new connectivity::ORowVector< ORowSetValue >(m_xMetaData->getColumnCount());

    (*(*m_aInsertRow)) = (*(*_rOriginalRow));
    // we don't unbound the bookmark column
    connectivity::ORowVector< ORowSetValue >::iterator aIter = (*m_aInsertRow)->begin();
    for(;aIter != (*m_aInsertRow)->end();++aIter)
        aIter->setModified(sal_False);
}
// -----------------------------------------------------------------------------
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.26  2001/03/15 08:19:18  fs
    cppuhelper/extract -> comphelper/extract

    Revision 1.25  2001/02/14 13:18:24  oj
    impl sql stmt

    Revision 1.24  2001/02/01 14:23:57  oj
    change for insert , delete and update rows

    Revision 1.23  2001/01/26 15:18:17  oj
    #83216# check if we stands after the last row after a next

    Revision 1.22  2001/01/24 09:52:19  oj
    #82628# rowset modifications

    Revision 1.21  2001/01/22 07:38:24  oj
    #82632# change member

    Revision 1.20  2001/01/09 15:38:59  oj
    look for concurrency

    Revision 1.19  2001/01/04 14:30:37  oj
    check columns

    Revision 1.18  2000/12/14 11:41:19  oj
    #82061# beforeFirst called everytime

    Revision 1.17  2000/12/12 12:19:01  oj
    #80933# change flush of attributes

    Revision 1.16  2000/12/06 09:55:44  oj
    #80219# correted deleterow(s) and remeber position

    Revision 1.15  2000/11/22 14:56:33  oj
    #80276# resolve some trouble with positioning

    Revision 1.14  2000/11/15 15:57:40  oj
    change for rowset

    Revision 1.13  2000/11/14 13:28:20  oj
    change for rowset when getRow returns 0

    Revision 1.12  2000/11/10 16:05:41  oj
    check for afterlast and before first

    Revision 1.11  2000/11/10 14:17:54  oj
    search for primarykey not only keys

    Revision 1.10  2000/11/10 11:05:43  oj
    bookmark error corrected

    Revision 1.9  2000/11/07 13:19:27  oj
    read one row forward because of isLast

    Revision 1.8  2000/11/03 14:40:45  oj
    some problems with refcount resolved

    Revision 1.7  2000/10/25 07:30:24  oj
    make strings unique for lib's

    Revision 1.6  2000/10/17 10:18:12  oj
    some changes for the rowset

    Revision 1.5  2000/10/11 11:18:11  fs
    replace unotools with comphelper

    Revision 1.4  2000/10/05 14:52:16  oj
    last changed

    Revision 1.3  2000/10/04 13:34:40  oj
    some changes for deleteRow and updateRow

    Revision 1.2  2000/09/29 15:20:51  oj
    rowset impl

    Revision 1.1.1.1  2000/09/19 00:15:38  hr
    initial import

    Revision 1.2  2000/09/18 14:52:47  willem.vandorp
    OpenOffice header added.

    Revision 1.1  2000/09/01 15:21:04  oj
    rowset addons

    Revision 1.0 26.07.2000 12:37:41  oj
------------------------------------------------------------------------*/

