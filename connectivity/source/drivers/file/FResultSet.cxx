/*************************************************************************
 *
 *  $RCSfile: FResultSet.cxx,v $
 *
 *  $Revision: 1.68 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-10 08:10:23 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <limits>  // included here to prevent problems if compiling with C52

#ifdef GCC
#include <iostream>
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_ORESULTSET_HXX_
#include "file/FResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_ORESULTSETMETADATA_HXX_
#include "file/FResultSetMetaData.hxx"
#endif
#ifndef _COM_SUN_STAR_SQLC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include "connectivity/dbconversion.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _ITERATOR_
#include <iterator>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_FETCHDIRECTION_HPP_
#include <com/sun/star/sdbc/FetchDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XINDEXESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#endif


#include <algorithm>
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::file;
using namespace ::cppu;
using namespace dbtools;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;

// Maximale Anzahl von Rows, die mit ORDER BY sortiert durchlaufen werden koennen:
#if defined (WIN)
#define MAX_KEYSET_SIZE 0x3ff0  // Etwas weniger als ein Segment, damit
                                    // noch Platz fuer Memory Debug-Informationen
#else
#define MAX_KEYSET_SIZE 0x40000 // 256K
#endif
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.drivers.file.ResultSet","com.sun.star.sdbc.ResultSet");
// -------------------------------------------------------------------------
OResultSet::OResultSet(OStatement_Base* pStmt,OSQLParseTreeIterator&    _aSQLIterator) :    OResultSet_BASE(m_aMutex)
                        ,::comphelper::OPropertyContainer(OResultSet_BASE::rBHelper)
                        ,m_aStatement((OWeakObject*)pStmt)
                        ,m_nRowPos(-1)
                        ,m_bLastRecord(sal_False)
                        ,m_bEOF(sal_False)
                        ,m_xDBMetaData(pStmt->getConnection()->getMetaData())
                        ,m_xMetaData(NULL)
                        ,m_aSQLIterator(_aSQLIterator)
                        ,m_pTable(NULL)
                        ,m_nRowCountResult(-1)
                        ,m_nFilePos(0)
                        ,m_nLastVisitedPos(-1)
                        ,m_pParseTree(pStmt->getParseTree())
                        ,m_pFileSet(NULL)
                        ,m_pEvaluationKeySet(NULL)
                        ,m_pSortIndex(NULL)
                        ,m_aAssignValues(NULL)
                        ,m_bInserted(sal_False)
                        ,m_bRowUpdated(sal_False)
                        ,m_bRowInserted(sal_False)
                        ,m_bRowDeleted(sal_False)
                        ,m_nFetchSize(0)
                        ,m_nResultSetType(ResultSetType::SCROLL_INSENSITIVE)
                        ,m_nResultSetConcurrency(ResultSetConcurrency::UPDATABLE)
                        ,m_nFetchDirection(FetchDirection::FORWARD)
                        ,m_nTextEncoding(pStmt->getOwnConnection()->getTextEncoding())
                        ,m_nCurrentPosition(0)
                        ,m_bShowDeleted(pStmt->getOwnConnection()->showDeleted())
{
    osl_incrementInterlockedCount( &m_refCount );


    for (int jj = 0; jj < sizeof m_nOrderbyColumnNumber / sizeof (* m_nOrderbyColumnNumber); jj++)
    {
        m_nOrderbyColumnNumber[jj] = SQL_COLUMN_NOTFOUND;
        bOrderbyAscending[jj] = TRUE;
    }
    m_nResultSetConcurrency = isCount() ? ResultSetConcurrency::READ_ONLY : ResultSetConcurrency::UPDATABLE;
    construct();
    osl_decrementInterlockedCount( &m_refCount );
}

// -------------------------------------------------------------------------
OResultSet::~OResultSet()
{
    delete m_pSQLAnalyzer;
}
// -------------------------------------------------------------------------
void OResultSet::construct()
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),           PROPERTY_ID_FETCHSIZE,          0,&m_nFetchSize,        ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),        PROPERTY_ID_RESULTSETTYPE,      PropertyAttribute::READONLY,&m_nResultSetType,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),      PROPERTY_ID_FETCHDIRECTION,     0,&m_nFetchDirection,   ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY), PROPERTY_ID_RESULTSETCONCURRENCY,PropertyAttribute::READONLY,&m_nResultSetConcurrency,                ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
}
// -------------------------------------------------------------------------
void OResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    m_aStatement    = NULL;
    m_xMetaData     = NULL;
    m_pParseTree    = NULL;
    m_xColNames     = NULL;
    m_xColumns      = NULL;
    m_xParamColumns = NULL;

    if(m_pSQLAnalyzer)
        m_pSQLAnalyzer->dispose();
    if(m_pTable)
    {
        m_pTable->release();
        m_pTable = NULL;
    }
    DELETEZ(m_pFileSet);
    DELETEZ(m_pEvaluationKeySet);
    DELETEZ(m_pSortIndex);

    if(m_aRow.isValid())
        m_aRow->clear();
    if(m_aEvaluateRow.isValid())
        m_aEvaluateRow->clear();
    if(m_aInsertRow.isValid())
        m_aInsertRow->clear();
    if(m_aAssignValues.isValid())
        m_aAssignValues->clear();

    m_aBookmarksPositions.clear();
    m_aBookmarks.clear();
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Any aRet = OPropertySetHelper::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OResultSet::getTypes(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OTypeCollection aTypes( ::getCppuType( (const Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                            ::getCppuType( (const Reference< ::com::sun::star::beans::XPropertySet > *)0 ),
                            ::getCppuType( (const Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
            break;
    return i;
}
// -----------------------------------------------------------------------------
const ORowSetValue& OResultSet::getValue(sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    checkIndex(columnIndex );

    columnIndex = mapColumn(columnIndex);
    m_bWasNull = (*m_aRow)[columnIndex].isNull();
    return (*m_aRow)[columnIndex];
}
// -----------------------------------------------------------------------------
void OResultSet::checkIndex(sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException)
{
    if(columnIndex <= 0 || columnIndex > (sal_Int32)m_xColumns->size())
        ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

::com::sun::star::util::Date SAL_CALL OResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

double SAL_CALL OResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

float SAL_CALL OResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nPos = (sal_Int32)(*m_aRow)[0];
    ::std::map<sal_Int32,sal_Int32>::const_iterator aFind = m_aBookmarks.find(nPos);
    OSL_ENSURE(aFind != m_aBookmarks.end(),"OResultSet::getRow() invalid bookmark!");
    return aFind->second;
//  ::std::map<sal_Int32,sal_Int32>::const_iterator aIter = m_aBookmarkToPos.begin();
//  for(;aIter != aFind;++aIter,++nRowPos)
//      ;
//  return nRowPos;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return sal_Int64();
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_xColumns,m_aSQLIterator.getTables().begin()->first,m_pTable);
    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex).makeAny();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL OResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == sal_Int32(m_pFileSet->size());
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == sal_Int32(m_pFileSet->size() - 1);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(first())
        previous();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(last())
        next();
    m_bEOF = sal_True;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::first(  ) throw(SQLException, RuntimeException)
{
    return SkipDeleted(OFileTable::FILE_FIRST,1,sal_True);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::last(  ) throw(SQLException, RuntimeException)
{
    // here I know definitely that I stand on the last record
    return SkipDeleted(OFileTable::FILE_LAST,1,sal_True);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    return SkipDeleted(OFileTable::FILE_ABSOLUTE,row,sal_True);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    return SkipDeleted(OFileTable::FILE_RELATIVE,row,sal_True);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    return SkipDeleted(OFileTable::FILE_PRIOR,0,sal_True);
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_aStatement.get();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bRowDeleted;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{   ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bRowInserted;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bRowUpdated;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == -1;
}
// -------------------------------------------------------------------------
sal_Bool OResultSet::evaluate()
{
    OSL_ENSURE(m_pSQLAnalyzer,"OResultSet::evaluate: Analyzer isn't set!");
    sal_Bool bRet = sal_True;
    while(!m_pSQLAnalyzer->evaluateRestriction())
    {
        if(m_pEvaluationKeySet)
        {
            if(m_aEvaluateIter == m_pEvaluationKeySet->end())
                return sal_False;
            bRet = m_pTable->seekRow(OFileTable::FILE_BOOKMARK,(*m_aEvaluateIter),m_nRowPos);
            ++m_aEvaluateIter;
        }
        else
            bRet = m_pTable->seekRow(OFileTable::FILE_NEXT,1,m_nRowPos);
        if(bRet)
        {
            if(m_pEvaluationKeySet)
            {
                bRet = m_pTable->fetchRow(m_aEvaluateRow,m_pTable->getTableColumns().getBody(),sal_True,sal_True);
                evaluate();

            }
            else
                bRet = m_pTable->fetchRow(m_aRow,m_xColumns.getBody(),sal_False,sal_True);
        }
    }

    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_pTable ? SkipDeleted(OFileTable::FILE_NEXT,1,sal_True) : sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bWasNull;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::insertRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_bInserted)
        throwFunctionSequenceException(*this);

    // we know that we append new rows at the end
    // so we have to know where the end is
    SkipDeleted(OFileTable::FILE_LAST,1,sal_False);
    m_bRowInserted = m_pTable->InsertRow(m_aInsertRow.getBody(), TRUE,Reference<XIndexAccess>(m_xColNames,UNO_QUERY));
    if(m_bRowInserted && m_pFileSet)
    {
        sal_Int32 nPos = (*m_aInsertRow)[0];
        m_pFileSet->push_back(nPos);
        clearInsertRow();

        m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type((sal_Int32)(*m_aRow)[0],m_aBookmarksPositions.size()+1)).first);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(m_pTable->isReadOnly())
        throw SQLException(::rtl::OUString::createFromAscii("Table is readonly!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
    m_bRowUpdated = m_pTable->UpdateRow(m_aInsertRow.getBody(), m_aRow,Reference<XIndexAccess>(m_xColNames,UNO_QUERY));
    (*m_aInsertRow)[0] = (sal_Int32)(*m_aRow)[0];

    clearInsertRow();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::deleteRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(m_pTable->isReadOnly())
        throw SQLException(::rtl::OUString::createFromAscii("Table is readonly!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
    if (m_bShowDeleted)
        throw SQLException(::rtl::OUString::createFromAscii("Row could not be deleted. The option \"Display inactive records\" is set!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
    if(m_aRow->isDeleted())
        throw SQLException(::rtl::OUString::createFromAscii("Row was already deleted!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());

    sal_Int32 nPos = (sal_Int32)(*m_aRow)[0];
    m_bRowDeleted = m_pTable->DeleteRow(m_xColumns.getBody());
    if(m_bRowDeleted && m_pFileSet)
    {
        m_aRow->setDeleted(sal_True);
        // don't touch the m_pFileSet member here
        TInt2IntMap::iterator aFind = m_aBookmarks.find(nPos);
        OSL_ENSURE(aFind != m_aBookmarks.end(),"OResultSet::deleteRow() bookmark not found!");
        TInt2IntMap::iterator aIter = aFind;
        ++aIter;
        for (; aIter != m_aBookmarks.end() ; ++aIter)
            --(aIter->second);
        m_aBookmarksPositions.erase(m_aBookmarksPositions.begin() + aFind->second-1);
        m_aBookmarks.erase(nPos);
    }
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bInserted     = sal_False;
    m_bRowUpdated   = sal_False;
    m_bRowInserted  = sal_False;
    m_bRowDeleted   = sal_False;

    if(m_aInsertRow.isValid())
    {
        OValueVector::iterator aIter = m_aInsertRow->begin()+1;
        for(;aIter != m_aInsertRow->end();++aIter)
        {
            aIter->setBound(sal_False);
            aIter->setNull();
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(m_pTable->isReadOnly())
        throw SQLException(::rtl::OUString::createFromAscii("Table is readonly!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());

    Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);

    m_bInserted     = sal_True;

    OValueVector::iterator aIter = m_aInsertRow->begin()+1;
    for(;aIter != m_aInsertRow->end();++aIter)
    {
        aIter->setBound(sal_False);
        aIter->setNull();
    }
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void OResultSet::updateValue(sal_Int32 columnIndex ,const ORowSetValue& x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkIndex(columnIndex );
    columnIndex = mapColumn(columnIndex);
    (*m_aInsertRow)[columnIndex].setBound(sal_True);
    (*m_aInsertRow)[columnIndex] = x;
}
// -----------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkIndex(columnIndex );

    columnIndex = mapColumn(columnIndex);

    (*m_aInsertRow)[columnIndex].setBound(sal_True);
    (*m_aInsertRow)[columnIndex].setNull();

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionSequenceException(*this);
}
// -----------------------------------------------------------------------
void SAL_CALL OResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readSomeBytes(aSeq,length);
    updateValue(columnIndex,aSeq);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    updateBinaryStream(columnIndex,x,length);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}
// -------------------------------------------------------------------------
IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *const_cast<OResultSet*>(this)->getArrayHelper();
}

//------------------------------------------------------------------
BOOL OResultSet::ExecuteRow(OFileTable::FilePosition eFirstCursorPosition,
                               INT32 nFirstOffset,
                               BOOL bRebind,
                               BOOL bEvaluate,
                               BOOL bRetrieveData)
{
    OSL_ENSURE(m_pSQLAnalyzer,"OResultSet::ExecuteRow: Analyzer isn't set!");

    // Fuer weitere Fetch-Operationen werden diese Angaben ggf. veraendert ...
    OFileTable::FilePosition eCursorPosition = eFirstCursorPosition;
    INT32  nOffset = nFirstOffset;
    UINT32 nLoopCount = 0;

again:

    // protect from reading over the end when someboby is inserting while we are reading
    // this method works only for dBase at the moment !!!!
    if (eCursorPosition == OFileTable::FILE_NEXT && m_nFilePos == m_nLastVisitedPos)
    {
        return sal_False;
    }

    if (!m_pTable->seekRow(eCursorPosition, nOffset, m_nFilePos))
    {
        return sal_False;
    }

    if (!bEvaluate) // Laeuft keine Auswertung, dann nur Ergebniszeile fuellen
    {
        m_pTable->fetchRow(m_aRow,m_pTable->getTableColumns().getBody(), sal_True,bRetrieveData);

        //  if (bShowDeleted && m_aRow->isDeleted())
            //  m_aRow->setState(ROW_DELETED);
    }
    else
    {
        m_pTable->fetchRow(m_aEvaluateRow, m_pTable->getTableColumns().getBody(), sal_True,TRUE);

        if ((!m_bShowDeleted && m_aEvaluateRow->isDeleted()) ||
            (m_pSQLAnalyzer->hasRestriction() && //!bShowDeleted && m_aEvaluateRow->isDeleted() ||// keine Anzeige von geloeschten Sätzen
                !m_pSQLAnalyzer->evaluateRestriction()))         // Auswerten der Bedingungen
        {                                                // naechsten Satz auswerten
            // aktuelle Zeile loeschen im Keyset
            if (m_pEvaluationKeySet)
            {
                ++m_aEvaluateIter;
                if (m_pEvaluationKeySet->end() != m_aEvaluateIter)
                    nOffset = (*m_aEvaluateIter);
                else
                {
                    return sal_False;
                }
            }
            else if (m_pFileSet)
            {
                OSL_ENSURE(//!m_pFileSet->IsFrozen() &&
                            eCursorPosition == OFileTable::FILE_NEXT, "Falsche CursorPosition!");
                eCursorPosition = OFileTable::FILE_NEXT;
                nOffset = 1;
            }
            else if (eCursorPosition == OFileTable::FILE_FIRST ||
                     eCursorPosition == OFileTable::FILE_NEXT ||
                     eCursorPosition == OFileTable::FILE_ABSOLUTE)
            {
                eCursorPosition = OFileTable::FILE_NEXT;
                nOffset = 1;
            }
            else if (eCursorPosition == OFileTable::FILE_LAST ||
                     eCursorPosition == OFileTable::FILE_PRIOR)
            {
                eCursorPosition = OFileTable::FILE_PRIOR;
                nOffset = 1;
            }
            else if (eCursorPosition == OFileTable::FILE_RELATIVE)
            {
                eCursorPosition = (nOffset >= 0) ? OFileTable::FILE_NEXT : OFileTable::FILE_PRIOR;
            }
            else
            {
                //  aStatus.Set(SQL_STAT_NO_DATA_FOUND);
                return sal_False;
            }
            // Nochmal probieren ...
            goto again;
        }
    }
    // Evaluate darf nur gesetzt sein,
    // wenn der Keyset weiter aufgebaut werden soll
    if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT && !isCount() &&
        (m_pFileSet || m_pSortIndex) && bEvaluate)
    {
        if (m_pSortIndex)
        {
            OFILEKeyValue* pKeyValue = GetOrderbyKeyValue(m_aEvaluateRow);
            if (!m_pSortIndex->AddKeyValue(pKeyValue))
            {
                // Ueberwachung auf Ueberschreitung der Index-Kapazitaet:
            }
        }
        else if (m_pFileSet)
        {
            //  OSL_ENSURE(!m_pFileSet->IsFrozen() , "Falsche CursorPosition!");
            sal_uInt32 nBookmarkValue = Abs((sal_Int32)(*m_aEvaluateRow)[0]);
            m_pFileSet->push_back(nBookmarkValue);
        }
    }
    else if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_UPDATE)
    {
        sal_Bool bOK = sal_True;
        if (bEvaluate)
        {
            // jetzt die eigentliche Ergebniszeile Lesen
            bOK = m_pTable->fetchRow(m_aEvaluateRow, m_pTable->getTableColumns().getBody(), sal_True,TRUE);
        }

        if (bOK)
        {
            // Nur die zu aendernden Werte uebergeben:
            if(!m_pTable->UpdateRow(m_aAssignValues.getBody(),m_aEvaluateRow,Reference<XIndexAccess>(m_xColNames,UNO_QUERY)))
                return sal_False;
        }
    }
    else if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_DELETE)
    {
        sal_Bool bOK = sal_True;
        if (bEvaluate)
        {
            bOK = m_pTable->fetchRow(m_aEvaluateRow, m_pTable->getTableColumns().getBody(), sal_True,TRUE);
        }
        if (bOK)
        {
            if(!m_pTable->DeleteRow(m_xColumns.getBody()))
                return sal_False;
        }
    }
    return sal_True;
}

//-------------------------------------------------------------------
BOOL OResultSet::Move(OFileTable::FilePosition eCursorPosition, INT32 nOffset, BOOL bRetrieveData)
{

//IgnoreDeletedRows:
//
    INT32 nTempPos = m_nRowPos;
    // exclusiver zugriff auf die Tabelle
    //  NAMESPACE_VOS(OGuard)* pGuard = m_pTable->Lock();

    if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT &&
        !isCount())
    {
        if (m_pFileSet == NULL) // kein Index verfuegbar
        {
            // Normales FETCH
            ExecuteRow(eCursorPosition,nOffset,TRUE,FALSE,bRetrieveData);

            // now set the bookmark for outside this is the logical pos  and not the file pos
            (*m_aRow->begin()) = sal_Int32(m_nRowPos + 1);
        }
        else
        {
            switch(eCursorPosition)
            {
                case OFileTable::FILE_NEXT:
                    m_nRowPos++;
                    break;
                case OFileTable::FILE_PRIOR:
                    if (m_nRowPos >= 0)
                        m_nRowPos--;
                    break;
                case OFileTable::FILE_FIRST:
                    m_nRowPos = 0;
                    break;
                case OFileTable::FILE_LAST:
                    //  OSL_ENSURE(IsRowCountFinal(), "Fehler im Keyset!"); // muß eingefroren sein, sonst Fehler beim SQLCursor
                    m_nRowPos = m_pFileSet->size() - 1;
                    break;
                case OFileTable::FILE_RELATIVE:
                    m_nRowPos += nOffset;
                    break;
                case OFileTable::FILE_ABSOLUTE:
                    m_nRowPos = nOffset - 1;
                    break;
                case OFileTable::FILE_BOOKMARK:
                    m_nRowPos = nOffset - 1;
                    break;
            }

            // OffRange?
            // Der FileCursor ist außerhalb des gueltigen Bereichs, wenn
            // a.) m_nRowPos < 1
            // b.) Ein KeySet besteht und m_nRowPos > m_pFileSet->size()
            if (m_nRowPos < 0 || (m_pFileSet->isFrozen() && eCursorPosition != OFileTable::FILE_BOOKMARK && m_nRowPos >= (INT32)m_pFileSet->size() )) // && m_pFileSet->IsFrozen()
            {
                //  aStatus.Set(SQL_STAT_NO_DATA_FOUND);
                goto Error;
            }
            else
            {
                if (m_nRowPos < (INT32)m_pFileSet->size())
                {
                    // Fetch ueber Index
                    ExecuteRow(OFileTable::FILE_BOOKMARK,(*m_pFileSet)[m_nRowPos],TRUE,FALSE,bRetrieveData);

                    // now set the bookmark for outside
                    (*m_aRow->begin()) = sal_Int32(m_nRowPos + 1);
                }
                else // Index muß weiter aufgebaut werden
                {
                    // Zunaechst auf die letzte bekannte Zeile setzen
                    if (m_pFileSet->size())
                    {
                        m_aFileSetIter = m_pFileSet->end()-1;
                        //  m_pFileSet->SeekPos(m_pFileSet->size()-1);
                        m_pTable->seekRow(OFileTable::FILE_BOOKMARK, *m_aFileSetIter, m_nFilePos);
                    }
                    sal_Bool bOK = sal_True;
                    // Ermitteln der Anzahl weiterer Fetches
                    while (bOK && m_nRowPos >= (INT32)m_pFileSet->size())
                    {
                        if (m_pEvaluationKeySet)
                        {
                            if (m_nRowPos >= (INT32)m_pEvaluationKeySet->size())
                                return sal_False;
                                //  aStatus.Set(SQL_STAT_NO_DATA_FOUND);
                            else if (m_nRowPos == 0)
                            {
                                m_aEvaluateIter = m_pEvaluationKeySet->begin();
                                bOK = ExecuteRow(OFileTable::FILE_BOOKMARK,*m_aEvaluateIter,FALSE,TRUE, bRetrieveData);
                            }
                            else
                            {
                                ++m_aEvaluateIter;
                                bOK = ExecuteRow(OFileTable::FILE_BOOKMARK,*m_aEvaluateIter,FALSE,TRUE, bRetrieveData);
                            }
                        }
                        else
                            bOK = ExecuteRow(OFileTable::FILE_NEXT,1,FALSE,TRUE, bRetrieveData);
                    }

                    if (bOK)
                    {
                        // jetzt nochmal die Ergebnisse lesen
                        m_pTable->fetchRow(m_aRow, m_pTable->getTableColumns().getBody(), sal_True,bRetrieveData);

                        // now set the bookmark for outside
                        (*m_aRow->begin()) = sal_Int32(m_nRowPos + 1);
                    }
                    else if (!m_pFileSet->isFrozen())                   // keinen gueltigen Satz gefunden
                    {
                        //m_pFileSet->Freeze();
                        m_pFileSet->setFrozen();

                        DELETEZ(m_pEvaluationKeySet);
                        //  aStatus.Set(SQL_STAT_NO_DATA_FOUND);
                        goto Error;
                    }
                }
            }
        }
    }
    else if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT && isCount())
    {
        // Fetch des COUNT(*)
        switch (eCursorPosition)
        {
            case OFileTable::FILE_NEXT:
                m_nRowPos++;
                break;
            case OFileTable::FILE_PRIOR:
                m_nRowPos--;
                break;
            case OFileTable::FILE_FIRST:
                m_nRowPos = 0;
                break;
            case OFileTable::FILE_LAST:
                m_nRowPos = 0;
                break;
            case OFileTable::FILE_RELATIVE:
                m_nRowPos += nOffset;
                break;
            case OFileTable::FILE_BOOKMARK:
            case OFileTable::FILE_ABSOLUTE:
                m_nRowPos = nOffset - 1;
                break;
        }

        if (m_nRowPos == 0)
        {
            // COUNT(*) in Ergebnisrow packen
            // (muss die erste und einzige Variable in der Row sein)
            if (m_aRow->size() >= 2)
            {
                (*m_aRow)[1] = m_nRowCountResult;
                (*m_aRow)[0] = sal_Int32(1);
                (*m_aRow)[1].setBound(sal_True);
            }
        }
        else
        {
            m_bEOF = sal_True;
            m_nRowPos = 1;
            return sal_False;
        }
    }
    else
        // Fetch nur bei SELECT moeglich!
        return sal_False;

    return sal_True;

Error:
    // steht der Cursor vor dem ersten Satz
    // dann wird die position beibehalten
    if (nTempPos == -1)
        m_nRowPos = nTempPos;
    else
    {
        switch(eCursorPosition)
        {
            case OFileTable::FILE_PRIOR:
            case OFileTable::FILE_FIRST:
                m_nRowPos = -1;
                break;
            case OFileTable::FILE_LAST:
            case OFileTable::FILE_NEXT:
            case OFileTable::FILE_ABSOLUTE:
            case OFileTable::FILE_RELATIVE:
                if (nOffset > 0)
                    m_nRowPos = (m_pFileSet) ? m_pFileSet->size() : -1;
                else if (nOffset < 0)
                    m_nRowPos = -1;
                break;
            case OFileTable::FILE_BOOKMARK:
                m_nRowPos = nTempPos;    // vorherige Position
        }
    }
    //  delete pGuard;
    //  rMode = (!bShowDeleted && aStatus.IsSuccessful() && m_aRow->isDeleted()) ?  // keine Anzeige von gelöschten Sätzen
                //  OCursor::SQL_MOD_INVALID : OCursor::SQL_MOD_NONE;
    return sal_False;
}

//------------------------------------------------------------------
BOOL OResultSet::SkipDeleted(OFileTable::FilePosition eCursorPosition, INT32 nOffset, BOOL bRetrieveData)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = sal_False;
    OSL_ENSURE(eCursorPosition != OFileTable::FILE_BOOKMARK,"OResultSet::SkipDeleted can't be called for BOOKMARK");

    OFileTable::FilePosition eDelPosition = eCursorPosition;
    INT32 nDelOffset = abs(nOffset);
    BOOL bDataFound;
    BOOL bDone = sal_True;

    switch (eCursorPosition)
    {
        case OFileTable::FILE_ABSOLUTE:
        case OFileTable::FILE_FIRST:                    // erster Satz nicht gültig, gehe auf nächsten
            eDelPosition = OFileTable::FILE_NEXT;
            nDelOffset = 1;
            break;
        case OFileTable::FILE_LAST:
            eDelPosition = OFileTable::FILE_PRIOR; // letzter Satz nicht gültig, gehe auf vorherigen
            nDelOffset = 1;
            break;
        case OFileTable::FILE_RELATIVE:
            eDelPosition = (nOffset >= 0) ? OFileTable::FILE_NEXT : OFileTable::FILE_PRIOR;
            break;
    }

    sal_Int32 nNewOffset    = nOffset;

    if (eCursorPosition == OFileTable::FILE_ABSOLUTE)
    {
        return moveAbsolute(nOffset,bRetrieveData);
    }
    else if (eCursorPosition == OFileTable::FILE_LAST)
    {
        sal_Int32 nBookmark = 0;
        sal_Int32 nCurPos = 1;
        // first position on the last known row
        if(m_aBookmarks.empty())
        {
            bDataFound = Move(OFileTable::FILE_FIRST, 1, bRetrieveData);
            if(bDataFound && (m_bShowDeleted || !m_aRow->isDeleted()))
                m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type((sal_Int32)(*m_aRow)[0],m_aBookmarksPositions.size()+1)).first);
        }
        else
        {
            // I already have a bookmark so we can positioned on that and look if it is the last one
            nBookmark = (*m_aBookmarksPositions.rbegin())->first;

            bDataFound = Move(OFileTable::FILE_BOOKMARK, nBookmark, bRetrieveData);
            OSL_ENSURE((m_bShowDeleted || !m_aRow->isDeleted()),"A bookmark should not be deleted!");
            nCurPos    = (*m_aBookmarksPositions.rbegin())->second;
        }


        // and than move forward until we are after the last row
        while(bDataFound)
        {
            bDataFound = Move(OFileTable::FILE_NEXT, 1, sal_False); // we don't need the data here
            if(bDataFound && (m_bShowDeleted || !m_aRow->isDeleted()))
            {   // we weren't on the last row we remember it and move on
                ++nCurPos;
                m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type((sal_Int32)(*m_aRow)[0],m_aBookmarksPositions.size()+1)).first);
            }
            else if(!bDataFound && m_aBookmarks.size())
            {
                // i already know the last bookmark :-)
                // now we only have to repositioning us to the last row
                nBookmark = (*m_aBookmarksPositions.rbegin())->first;
                bDataFound = Move(OFileTable::FILE_BOOKMARK, nBookmark, bRetrieveData);
                break;
            }
        }
        return bDataFound;
    }
    else if (eCursorPosition != OFileTable::FILE_RELATIVE)
    {
        bDataFound = Move(eCursorPosition, nOffset, bRetrieveData);
        bDone = bDataFound && (m_bShowDeleted || !m_aRow->isDeleted());
    }
    else
    {
        bDataFound = Move(eDelPosition, 1, bRetrieveData);
        if (bDataFound && (m_bShowDeleted || !m_aRow->isDeleted()))
        {
            m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type((sal_Int32)(*m_aRow)[0],m_aBookmarksPositions.size()+1)).first);
            bDone = (--nDelOffset) == 0;
        }
        else
            bDone = FALSE;
    }
    sal_Int32 nRowPos = m_nRowPos;

    while (bDataFound && !bDone)            // solange iterieren bis man auf einem gültigen Satz ist
    {
        bDataFound = Move(eDelPosition, 1, bRetrieveData);
        if (eCursorPosition != OFileTable::FILE_RELATIVE)
            bDone = bDataFound && (m_bShowDeleted || !m_aRow->isDeleted());
        else if (bDataFound && (m_bShowDeleted || !m_aRow->isDeleted()))
        {
            m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type((sal_Int32)(*m_aRow)[0],m_aBookmarksPositions.size()+1)).first);
            bDone = (--nDelOffset) == 0;
        }
        else
            bDone = FALSE;
    }

    if(bDataFound && bDone && m_aBookmarks.find((sal_Int32)(*m_aRow)[0]) == m_aBookmarks.end())
        m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type((sal_Int32)(*m_aRow)[0],m_aBookmarksPositions.size()+1)).first);

    return bDataFound;
}
// -------------------------------------------------------------------------
sal_Bool OResultSet::moveAbsolute(sal_Int32 _nOffset,sal_Bool _bRetrieveData)
{
    sal_Bool bDataFound = sal_False;
    sal_Int32 nNewOffset = _nOffset;
    if(nNewOffset > 0)
    {
        if((sal_Int32)m_aBookmarks.size() < nNewOffset)
        {
            // bookmark isn't known yet
            // start at the last position
            sal_Int32 nCurPos = 0,nLastBookmark = 1;
            OFileTable::FilePosition eFilePos = OFileTable::FILE_FIRST;
            if(!m_aBookmarks.empty())
            {
                nLastBookmark   = (*m_aBookmarksPositions.rbegin())->first;
                nCurPos         = (*m_aBookmarksPositions.rbegin())->second;
                nNewOffset      = nNewOffset - nCurPos;
                bDataFound      = Move(OFileTable::FILE_BOOKMARK, nLastBookmark, _bRetrieveData);
            }
            else
            {
                bDataFound = Move(OFileTable::FILE_FIRST, 1, _bRetrieveData);
                if(bDataFound && (m_bShowDeleted || !m_aRow->isDeleted()))
                {
                    ++nCurPos;
                    m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type((sal_Int32)(*m_aRow)[0],m_aBookmarksPositions.size()+1)).first);
                    --nNewOffset;
                }
            }
            // now move to that row we need and don't count deleted rows
            while (bDataFound && nNewOffset)
            {
                bDataFound = Move(OFileTable::FILE_NEXT, 1, _bRetrieveData);
                if(bDataFound && (m_bShowDeleted || !m_aRow->isDeleted()))
                {
                    ++nCurPos;
                    m_aBookmarksPositions.push_back(m_aBookmarks.insert(TInt2IntMap::value_type((sal_Int32)(*m_aRow)[0],m_aBookmarksPositions.size()+1)).first);
                    --nNewOffset;
                }
            }
        }
        else
        {
            sal_Int32 nBookmark = m_aBookmarksPositions[nNewOffset-1]->first;
            bDataFound = Move(OFileTable::FILE_BOOKMARK,nBookmark, _bRetrieveData);
            OSL_ENSURE((m_bShowDeleted || !m_aRow->isDeleted()),"moveAbsolute row can't be deleted!");
        }
    }
    else
    {
        ++nNewOffset;
        bDataFound = SkipDeleted(OFileTable::FILE_LAST,1,nNewOffset == 0);

        for(sal_Int32 i=nNewOffset+1;bDataFound && i <= 0;++i)
            bDataFound = SkipDeleted(OFileTable::FILE_PRIOR,1,i == 0);

    }
    return bDataFound;
}
// -------------------------------------------------------------------------
BOOL OResultSet::OpenImpl()
{
    m_pSQLAnalyzer = createAnalyzer();
    const OSQLTables& xTabs = m_aSQLIterator.getTables();
    if ((xTabs.begin() == xTabs.end()) || !xTabs.begin()->second.is())
        throwGenericSQLException(   ::rtl::OUString::createFromAscii("The statement is invalid."),
                                    static_cast<XWeak*>(this),
                                    makeAny(m_aSQLIterator.getWarning())
                                );

    OSQLTable xTable = xTabs.begin()->second;
    m_xColumns = m_aSQLIterator.getSelectColumns();

    m_xColNames = xTable->getColumns();

    Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);

    m_aRow          = new OValueVector(xNames->getCount());
    m_aEvaluateRow  = new OValueVector(xNames->getCount());
    m_aInsertRow    = new OValueVector(xNames->getCount());

    (*m_aRow)[0].setBound(sal_True);
    (*m_aEvaluateRow)[0].setBound(sal_True);
    (*m_aInsertRow)[0].setBound(sal_True);
    // unbound all values so that we only update the really needed columns
    OValueVector::iterator aInsertIter = m_aInsertRow->begin()+1;
    for(;aInsertIter != m_aInsertRow->end();++aInsertIter)
        aInsertIter->setBound(sal_False);

    if(m_aAssignValues.isValid())
        m_aAssignValues->clear();
    m_aAssignValues = new OAssignValues();

    if(!m_xParamColumns.isValid())
        m_xParamColumns = new OSQLColumns();

    OSL_ENSURE(m_pSQLAnalyzer,"OResultSet::OpenImpl: Analyzer isn't set!");
    m_pSQLAnalyzer->clean();

    doTableSpecials(xTable);

    OSL_ENSURE(m_pTable,"We need a table object!");
    Reference<XIndexesSupplier> xIndexSup(xTable,UNO_QUERY);
    if(xIndexSup.is())
        m_pSQLAnalyzer->setIndexes(xIndexSup->getIndexes());
    m_nResultSetConcurrency = (m_pTable->isReadOnly() || isCount()) ? ResultSetConcurrency::READ_ONLY : ResultSetConcurrency::UPDATABLE;

    GetAssignValues(); // assign values and describe parameter columns
    m_pSQLAnalyzer->setParameterColumns(m_xParamColumns);
    anylizeSQL();
    if (m_xParamColumns->size())
        m_pSQLAnalyzer->describeParam(m_xParamColumns);

    sal_Int32 i=0;
    // initialize the column index map (mapping select columns to table columns)
    m_aColMapping.resize(m_xColumns->size() + 1);
    for (i=0; i<(sal_Int32)m_aColMapping.size(); ++i)
        m_aColMapping[i] = i;

    // now check which columns are bound
    setBoundedColumns(m_aRow,m_xColumns,xNames,sal_True);
    // Parameter substituieren (AssignValues und Kriterien):
    if (m_xParamColumns->size())
    {
        // Zunächst AssignValues
        USHORT nParaCount=0; // gibt die aktuelle Anzahl der bisher gesetzen Parameter an

        // Nach zu substituierenden Parametern suchen:
        UINT16 nCount = m_aAssignValues.isValid() ? m_aAssignValues->size() : 1; // 1 ist wichtig für die Kriterien
        for (UINT16 j = 1; j < nCount; j++)
        {
//          if ((*m_aAssignValues)[j].isNull())
//              continue;

            UINT32 nParameter = (*m_aAssignValues).getParameterIndex(j);
            if (nParameter == SQL_NO_PARAMETER)
                continue;   // dieser AssignValue ist kein Parameter

            ++nParaCount; // ab hier ist der Parameter gueltig
            // Parameter ersetzen. Wenn Parameter nicht verfuegbar,
            // Value auf NULL setzen.
            (*m_aAssignValues)[j] = (*m_aParameterRow)[(UINT16)nParameter];
        }

        if (m_aParameterRow.isValid() &&  nParaCount < m_aParameterRow->size())
        {
            setBoundedColumns(m_aEvaluateRow,m_xParamColumns,xNames,sal_False);
            m_pSQLAnalyzer->bindParameterRow(m_aParameterRow);
        }
    }

    // Neuen Index aufbauen:
    DELETEZ(m_pFileSet);
    DELETEZ(m_pEvaluationKeySet);

    //  if (!aEvaluateRow.Is())
    {
        //  aEvaluateRow = new ODbRow(*aFileColumns, FALSE);
        //  (*aEvaluateRow)[0] = new ODbVariant();

        // Row zur Auswertung binden, wenn Preprocessing erfolg, dann bereits ein Keyset
        m_pEvaluationKeySet = m_pSQLAnalyzer->bindResultRow(m_aEvaluateRow);    // Werte im Code des Compilers setzen
                                                    // (Verbindung zur ResultRow herstellen)
    }

    // An den Anfang positionieren
    m_nRowPos = -1;
    m_nFilePos  = 0;
    m_nRowCountResult = -1;

    // exclusiver zugriff auf die Tabelle
    //  NAMESPACE_VOS(OGuard)* pGuard = pTable->Lock();
    m_nLastVisitedPos = m_pTable->getCurrentLastPos();

    UINT32 nLoopCount = 0;
    switch(m_aSQLIterator.getStatementType())
    {
        case SQL_STATEMENT_SELECT:
        {
            if(isCount())
            {
                m_nRowCountResult = 0;
                // Vorlaeufig einfach ueber alle Datensaetze iterieren und
                // dabei die Aktionen bearbeiten (bzw. einfach nur zaehlen):
                {
                    sal_Bool bOK = sal_True;
                    if (m_pEvaluationKeySet)
                    {
                        m_aEvaluateIter = m_pEvaluationKeySet->begin();
                        bOK = m_aEvaluateIter == m_pEvaluationKeySet->end();

                    }
                    while (bOK)
                    {
                        if (m_pEvaluationKeySet)
                            ExecuteRow(OFileTable::FILE_BOOKMARK,(*m_aEvaluateIter),FALSE,TRUE);
                        else
                            bOK = ExecuteRow(OFileTable::FILE_NEXT,1,FALSE,TRUE);

                        if (bOK)
                        {
                            m_nRowCountResult++;
                            if(m_pEvaluationKeySet)
                            {
                                ++m_aEvaluateIter;
                                bOK = m_aEvaluateIter == m_pEvaluationKeySet->end();
                            }
                        }
                    }

                    // Ergebnis von COUNT(*) in m_nRowCountResult merken.
                    // nRowCount, also die Anzahl der Rows in der Ergebnismenge, ist bei dieser
                    // Anfrage = 1!
                    DELETEZ(m_pEvaluationKeySet);
                }
            }
            else
            {
                INT32 nMaxRowCount = m_pTable->getCurrentLastPos();

                BOOL bDistinct = FALSE;
                BOOL bWasSorted = FALSE;
                OSQLParseNode *pDistinct = m_pParseTree->getChild(1);
                if (pDistinct && pDistinct->getTokenID() == SQL_TOKEN_DISTINCT )
                {
                    if(!IsSorted())
                    {
                        m_nOrderbyColumnNumber[0] = m_aColMapping[1];
                        bOrderbyAscending[0] = FALSE;
                    }
                    else
                        bWasSorted = TRUE;
                    bDistinct = TRUE;
                }
                // Ohne Restriction und Sortierung RowCount bekannt.
                //  if (!HasRestriction() && !IsSorted() && bShowDeleted)
                    //  SetRowCount(MaxRowCount());

                OSL_ENSURE(sizeof m_nOrderbyColumnNumber / sizeof (* m_nOrderbyColumnNumber) == SQL_ORDERBYKEYS,"Maximale Anzahl der ORDER BY Columns muss derzeit genau 3 sein!");
                OKeyType eKeyType[SQL_ORDERBYKEYS];
                OValueVector::iterator aRowIter = m_aRow->begin()+1;
                for (int i = 0; i < SQL_ORDERBYKEYS; i++)
                {
                    if (m_nOrderbyColumnNumber[i] == SQL_COLUMN_NOTFOUND)
                        eKeyType[i] = SQL_ORDERBYKEY_NONE;
                    else
                    {
                        OSL_ENSURE((sal_Int32)m_aRow->size() > m_nOrderbyColumnNumber[i],"Invalid Index");
                        switch ((m_aRow->begin()+m_nOrderbyColumnNumber[i])->getTypeKind())
                        {
                        case DataType::CHAR:
                            case DataType::VARCHAR:
                                eKeyType[i] = SQL_ORDERBYKEY_STRING;
                                break;

                            case DataType::OTHER:
                            case DataType::TINYINT:
                            case DataType::SMALLINT:
                            case DataType::INTEGER:
                            case DataType::DECIMAL:
                            case DataType::NUMERIC:
                            case DataType::REAL:
                            case DataType::DOUBLE:
                            case DataType::DATE:
                            case DataType::TIME:
                            case DataType::TIMESTAMP:
                            case DataType::BIT:
                                eKeyType[i] = SQL_ORDERBYKEY_DOUBLE;
                                break;

                        // Andere Typen sind nicht implementiert (und damit immer FALSE)
                            default:
                                eKeyType[i] = SQL_ORDERBYKEY_NONE;
                                OSL_ASSERT("OFILECursor::Execute: Datentyp nicht implementiert");
                                break;
                        }
                        (*m_aEvaluateRow)[m_nOrderbyColumnNumber[i]].setBound(sal_True);
                    }
                }

                // Nur wenn Sortierung gewuenscht, ueber alle Datensaetze iterieren und
                // dabei den "Key", nach dem sortiert werden soll, in den Index eintragen:
                if (IsSorted())
                {
                    if (!m_pSQLAnalyzer->hasRestriction() && m_nOrderbyColumnNumber[1] == SQL_COLUMN_NOTFOUND)
                    {
                        // Ist nur ein Feld fuer die Sortierung angegeben
                        // Und diese Feld ist indiziert, dann den Index ausnutzen
                        Reference<XIndexesSupplier> xIndexSup(xTable,UNO_QUERY);
                        Reference<XIndexAccess> xIndexes;
                        if(xIndexSup.is())
                        {
                            xIndexes = Reference<XIndexAccess>(xIndexSup->getIndexes(),UNO_QUERY);
                            Reference<XPropertySet> xColProp;
                            if(m_nOrderbyColumnNumber[0] < xIndexes->getCount())
                            {
                                ::cppu::extractInterface(xColProp,xIndexes->getByIndex(m_nOrderbyColumnNumber[0]));
                                // iterate through the indexes to find the matching column
                                for(sal_Int32 i=0;i<xIndexes->getCount();++i)
                                {
                                    Reference<XColumnsSupplier> xIndex;
                                    ::cppu::extractInterface(xIndex,xIndexes->getByIndex(i));
                                    Reference<XNameAccess> xIndexCols = xIndex->getColumns();
                                    if(xIndexCols->hasByName(comphelper::getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)))))
                                    {
                                        m_pFileSet = new OKeySet();

                                        if(fillIndexValues(xIndex))
                                            goto DISTINCT;
                                    }
                                }
                            }
                        }
                    }

                    m_pSortIndex = new OFILESortIndex(eKeyType,
                                                      bOrderbyAscending,
                                                      nMaxRowCount,m_nTextEncoding);

                    sal_Bool bOK = sal_True;
                    if (m_pEvaluationKeySet)
                    {
                        if (m_pEvaluationKeySet->size())
                            m_aEvaluateIter = m_pEvaluationKeySet->begin();

                    }
                    while (bOK)
                    {
                        if (m_pEvaluationKeySet)
                        {
                            ExecuteRow(OFileTable::FILE_BOOKMARK,(*m_aEvaluateIter),FALSE,TRUE);
                            ++m_aEvaluateIter;
                            bOK = m_aEvaluateIter == m_pEvaluationKeySet->end();
                        }
                        else
                            bOK = ExecuteRow(OFileTable::FILE_NEXT,1,FALSE,TRUE);
                    }

                    // Sortiertes Keyset erzeugen
                    DELETEZ(m_pEvaluationKeySet);
                    if(m_pFileSet)
                        delete m_pFileSet;
                    m_pFileSet = m_pSortIndex->CreateKeySet();
                    //  if(!bDistinct)
                        //  SetRowCount(pFileSet->count());
                    DELETEZ(m_pSortIndex);
                    // Nun kann ueber den Index sortiert zugegriffen werden.
                }

                if (!m_pFileSet)
                {
                    m_pFileSet = new OKeySet();

                    if (!m_pSQLAnalyzer->hasRestriction())
                    // jetzt kann das Keyset schon gefuellt werden!
                    // Aber Achtung: es wird davon ausgegangen, das die FilePositionen als Folge 1..n
                    // abgelegt werden!
                    {
                        for (sal_Int32 i = 0; i < m_pTable->getCurrentLastPos(); i++)
                           m_pFileSet->push_back(i + 1);
                    }
                }
                OSL_ENSURE(m_pFileSet,"Kein KeySet vorhanden! :-(");
    DISTINCT:   if(bDistinct && m_pFileSet)   // sicher ist sicher
                {
                    OValueRow aSearchRow = new OValueVector(m_aRow->size());
                    OValueVector::iterator aRowIter = m_aRow->begin();
                    OValueVector::iterator aSearchIter = aSearchRow->begin();
                    for (   ++aRowIter,++aSearchIter;   // the first column is the bookmark column
                            aRowIter != m_aRow->end();
                            ++aRowIter,++aSearchIter)
                                aSearchIter->setBound(aRowIter->isBound());
                    //  ODbRowRef aSearchRow = new ODbRow(*aFileRow); // nach dieser wird gesucht
                    //  const ODbRow &rSearchRow = *aSearchRow,
                        //  &rFileRow = *aFileRow;
                    INT32 nPos;
                    UINT16 nMaxRow = m_pFileSet->size();
                    if (nMaxRow)
                    {
    #if DEBUG
                        INT32 nFound=0;
    #endif
                        vector<BOOL> nWasAllwaysFound(nMaxRow,FALSE);
                        INT32 nPrev_i;
                        for(INT32 j= nMaxRow-1;j >= 0;j--)
                        {
                            nPos = (*m_pFileSet)[j]; // aktuell zu löschender Key
                            if(!nWasAllwaysFound[j] && nPos) // nur falls noch nicht nach dieser Row gesucht wurde
                            {
                                ExecuteRow(OFileTable::FILE_BOOKMARK,nPos,TRUE,FALSE);
                                *aSearchRow = *m_aRow;
                                // jetzt den Rest nach doppelten durchsuchen
                                INT32 nKey;
                                nPrev_i = j;
                                for(INT32 i = j-1; i >= 0 ;i--)
                                {
                                    nKey = (*m_pFileSet)[i];
                                    ExecuteRow(OFileTable::FILE_BOOKMARK,nKey ,TRUE,FALSE);
                                    if(!nWasAllwaysFound[i])
                                    {
                                        OValueVector::iterator aRowIter = m_aRow->begin();
                                        OValueVector::iterator aSearchIter = aSearchRow->begin();
                                        for (   ++aRowIter,++aSearchIter;   // the first column is the bookmark column
                                                aRowIter != m_aRow->end();
                                                ++aRowIter,++aSearchIter)
                                        {
                                            if(aRowIter->isBound() && !(*aRowIter == *aSearchIter))
                                                break;
                                        }
                                        if(aRowIter == m_aRow->end())
                                        {
                                            // gefunden
                                            // Key an der Stelle 0 setzen.
                                            (*m_pFileSet)[nPrev_i] = 0;
                                            // und altes i merken
                                            nPrev_i = i;
                                            nPos = nKey; // auf naechste gültige Position setzen
                                            nWasAllwaysFound[i] = TRUE;
                                        }
                                    }
                                }
                            }
    #if DEBUG
                            else
                                nFound++;
    #endif
                        }
                        vector<INT32>::iterator aIter = m_pFileSet->end()-1;
                        while(aIter != m_pFileSet->begin())
                        {
                            if(!(*aIter))
                                m_pFileSet->erase(aIter);
                            aIter--;
                        }
                        if (!bWasSorted)
                        {
                            m_nOrderbyColumnNumber[0] = SQL_COLUMN_NOTFOUND;
                            sort(m_pFileSet->begin(),m_pFileSet->end());
                        }
                    }
                    //  SetRowCount(m_pFileSet->count());
                }
            }
        }   break;

        case SQL_STATEMENT_SELECT_COUNT:
        case SQL_STATEMENT_UPDATE:
        case SQL_STATEMENT_DELETE:
            // waehrend der Bearbeitung die Anzahl der bearbeiteten Rows zaehlen:
            m_nRowCountResult = 0;
            // Vorlaeufig einfach ueber alle Datensaetze iterieren und
            // dabei die Aktionen bearbeiten (bzw. einfach nur zaehlen):
            {

                sal_Bool bOK = sal_True;
                if (m_pEvaluationKeySet)
                {
                    m_aEvaluateIter = m_pEvaluationKeySet->begin();
                    bOK = m_aEvaluateIter == m_pEvaluationKeySet->end();

                }
                while (bOK)
                {
                    if (m_pEvaluationKeySet)
                        ExecuteRow(OFileTable::FILE_BOOKMARK,(*m_aEvaluateIter),FALSE,TRUE);
                    else
                        bOK = ExecuteRow(OFileTable::FILE_NEXT,1,FALSE,TRUE);

                    if (bOK)
                    {
                        m_nRowCountResult++;
                        if(m_pEvaluationKeySet)
                        {
                            ++m_aEvaluateIter;
                            bOK = m_aEvaluateIter == m_pEvaluationKeySet->end();
                        }
                    }
                }

                // Ergebnis von COUNT(*) in nRowCountResult merken.
                // nRowCount, also die Anzahl der Rows in der Ergebnismenge, ist bei dieser
                // Anfrage = 1!
                DELETEZ(m_pEvaluationKeySet);
            }
            //  SetRowCount(1);
            break;
        case SQL_STATEMENT_INSERT:
            m_nRowCountResult = 0;

            if(!m_pTable->InsertRow(m_aAssignValues.getBody(), TRUE,Reference<XIndexAccess>(m_xColNames,UNO_QUERY)))
            {
                m_nFilePos  = 0;
                return sal_False;
            }

            m_nRowCountResult = 1;
            break;
    }

    // FilePos zuruecksetzen
    m_nFilePos  = 0;

    return sal_True;
}
//------------------------------------------------------------------
void OResultSet::anylizeSQL()
{
    OSL_ENSURE(m_pSQLAnalyzer,"OResultSet::anylizeSQL: Analyzer isn't set!");
    // start analysing the statement
    m_pSQLAnalyzer->setOrigColumns(m_xColNames);
    m_pSQLAnalyzer->start(m_pParseTree);

    const OSQLParseNode* pOrderbyClause = m_aSQLIterator.getOrderTree();
    if(pOrderbyClause)
    {
        OSQLParseNode * pOrderingSpecCommalist = pOrderbyClause->getChild(2);
        OSL_ENSURE(SQL_ISRULE(pOrderingSpecCommalist,ordering_spec_commalist),"OResultSet: Fehler im Parse Tree");

        for (sal_uInt32 m = 0; m < pOrderingSpecCommalist->count(); m++)
        {
            OSQLParseNode * pOrderingSpec = pOrderingSpecCommalist->getChild(m);
            OSL_ENSURE(SQL_ISRULE(pOrderingSpec,ordering_spec),"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(pOrderingSpec->count() == 2,"OResultSet: Fehler im Parse Tree");

            OSQLParseNode * pColumnRef = pOrderingSpec->getChild(0);
            if(!SQL_ISRULE(pColumnRef,column_ref))
            {
                //  aStatus.SetStatementTooComplex();
                return;
            }
            OSQLParseNode * pAscendingDescending = pOrderingSpec->getChild(1);
            setOrderbyColumn((UINT16)m,pColumnRef,pAscendingDescending);
        }
    }
}


//------------------------------------------------------------------
void OResultSet::setOrderbyColumn(UINT16 nOrderbyColumnNo,
                                     OSQLParseNode* pColumnRef,
                                     OSQLParseNode* pAscendingDescending)
{
    if (nOrderbyColumnNo >= (sizeof m_nOrderbyColumnNumber / sizeof (* m_nOrderbyColumnNumber)))
    {
        //  aStatus.SetStatementTooComplex();
        return;
    }

    ::rtl::OUString aColumnName;
    if (pColumnRef->count() == 1)
        aColumnName = pColumnRef->getChild(0)->getTokenValue();
    else if (pColumnRef->count() == 3)
    {
        // Nur die Table Range-Variable darf hier vorkommen:
//      if (!(pColumnRef->getChild(0)->getTokenValue() == aTableRange))
//      {
//          aStatus.Set(SQL_STAT_ERROR,
//                      String::CreateFromAscii("S1000"),
//                      aStatus.CreateErrorMessage(String(SdbResId(STR_STAT_INVALID_RANGE_VAR))),
//                      0, String() );
            //  return;
        //  }
        pColumnRef->getChild(2)->parseNodeToStr(aColumnName,m_xDBMetaData,NULL,sal_False,sal_False);
    }
    else
    {
        //  aStatus.SetStatementTooComplex();
        return;
    }

    Reference<XColumnLocate> xColLocate(m_xColNames,UNO_QUERY);
    if(!xColLocate.is())
        return;
    // Alles geprueft und wir haben den Namen der Column.
    // Die wievielte Column ist das?
    m_nOrderbyColumnNumber[nOrderbyColumnNo] = xColLocate->findColumn(aColumnName);

    // Ascending or Descending?
    bOrderbyAscending[nOrderbyColumnNo]  = (SQL_ISTOKEN(pAscendingDescending,DESC)) ?
                                            FALSE : TRUE;
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OResultSet::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OResultSet::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return 0;
}
//------------------------------------------------------------------
void OResultSet::GetAssignValues()
{
    if (m_pParseTree == NULL)
    {
        ::dbtools::throwFunctionSequenceException(*this);
        return;
    }

    if (SQL_ISRULE(m_pParseTree,select_statement))
        // Keine zu setzenden Werte bei SELECT
        return;
    else if (SQL_ISRULE(m_pParseTree,insert_statement))
    {
        // Row fuer die zu setzenden Werte anlegen (Referenz durch new)
        if(m_aAssignValues.isValid())
            m_aAssignValues->clear();
        m_aAssignValues = new OAssignValues(Reference<XIndexAccess>(m_xColNames,UNO_QUERY)->getCount());

        // Liste der Columns-Namen, die in der column_commalist vorkommen (mit ; getrennt):
        ::std::vector<String> aColumnNameList;

        OSL_ENSURE(m_pParseTree->count() >= 4,"OResultSet: Fehler im Parse Tree");

        OSQLParseNode * pOptColumnCommalist = m_pParseTree->getChild(3);
        OSL_ENSURE(pOptColumnCommalist != NULL,"OResultSet: Fehler im Parse Tree");
        OSL_ENSURE(SQL_ISRULE(pOptColumnCommalist,opt_column_commalist),"OResultSet: Fehler im Parse Tree");
        if (pOptColumnCommalist->count() == 0)
        {
            const Sequence< ::rtl::OUString>& aNames = m_xColNames->getElementNames();
            const ::rtl::OUString* pBegin = aNames.getConstArray();
            aColumnNameList.insert(aColumnNameList.begin(),::std::vector<String>::const_iterator(pBegin),::std::vector<String>::const_iterator(pBegin + aNames.getLength()));
        }
        else
        {
            OSL_ENSURE(pOptColumnCommalist->count() == 3,"OResultSet: Fehler im Parse Tree");

            OSQLParseNode * pColumnCommalist = pOptColumnCommalist->getChild(1);
            OSL_ENSURE(pColumnCommalist != NULL,"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(SQL_ISRULE(pColumnCommalist,column_commalist),"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(pColumnCommalist->count() > 0,"OResultSet: Fehler im Parse Tree");

            // Alle Columns in der column_commalist ...
            for (sal_uInt32 i = 0; i < pColumnCommalist->count(); i++)
            {
                OSQLParseNode * pCol = pColumnCommalist->getChild(i);
                OSL_ENSURE(pCol != NULL,"OResultSet: Fehler im Parse Tree");
                aColumnNameList.push_back(pCol->getTokenValue());
            }
        }
        if(!aColumnNameList.size())
            throwFunctionSequenceException(*this);

        // Werte ...
        OSQLParseNode * pValuesOrQuerySpec = m_pParseTree->getChild(4);
        OSL_ENSURE(pValuesOrQuerySpec != NULL,"OResultSet: pValuesOrQuerySpec darf nicht NULL sein!");
        OSL_ENSURE(SQL_ISRULE(pValuesOrQuerySpec,values_or_query_spec),"OResultSet: ! SQL_ISRULE(pValuesOrQuerySpec,values_or_query_spec)");
        OSL_ENSURE(pValuesOrQuerySpec->count() > 0,"OResultSet: pValuesOrQuerySpec->count() <= 0");

        // nur "VALUES" ist erlaubt ...
        if (! SQL_ISTOKEN(pValuesOrQuerySpec->getChild(0),VALUES))
            throwFunctionSequenceException(*this);

        OSL_ENSURE(pValuesOrQuerySpec->count() == 2,"OResultSet: pValuesOrQuerySpec->count() != 2");

        // Liste von Werten
        OSQLParseNode * pInsertAtomCommalist = pValuesOrQuerySpec->getChild(1);
        OSL_ENSURE(pInsertAtomCommalist != NULL,"OResultSet: pInsertAtomCommalist darf nicht NULL sein!");
        OSL_ENSURE(pInsertAtomCommalist->count() > 0,"OResultSet: pInsertAtomCommalist <= 0");

        String aColumnName;
        OSQLParseNode * pRow_Value_Const;
        xub_StrLen nIndex=0;
        for (sal_uInt32 i = 0; i < pInsertAtomCommalist->count(); i++)
        {
            pRow_Value_Const = pInsertAtomCommalist->getChild(i); // row_value_constructor
            if(pRow_Value_Const->count() == 3)  // '(' row_value_const_list ')'
            {
                pRow_Value_Const = pRow_Value_Const->getChild(1); // row_value_const_list
                OSL_ENSURE(pRow_Value_Const != NULL,"OResultSet: pRow_Value_Const darf nicht NULL sein!");
                if(SQL_ISRULE(pRow_Value_Const,parameter))
                {
                    if(pRow_Value_Const->count() == aColumnNameList.size())
                        ParseAssignValues(aColumnNameList,pRow_Value_Const,nIndex++); // kann nur ein Columnname vorhanden sein pro Schleife
                    else
                    {
//                      aStatus.Set(SQL_STAT_ERROR,
//                      String::CreateFromAscii("S1000"),
//                      aStatus.CreateErrorMessage(String(SdbResId(STR_STAT_SYNTAX_ERROR))),
//                      0, String() );
                        throwFunctionSequenceException(*this);
                    }
                }
                else
                {
                    if(pRow_Value_Const->count() == aColumnNameList.size())
                    {
                        for (sal_uInt32 j = 0; j < pRow_Value_Const->count(); ++j)
                            ParseAssignValues(aColumnNameList,pRow_Value_Const->getChild(j),nIndex++);
                    }
                    else
                    {
//                      aStatus.Set(SQL_STAT_ERROR,
//                      String::CreateFromAscii("S1000"),
//                      aStatus.CreateErrorMessage(String(SdbResId(STR_STAT_SYNTAX_ERROR))),
//                      0, String() );
                        throwFunctionSequenceException(*this);
                    }

                }
            }
            else
            {
                //  aStatus.SetStatementTooComplex();
                throwFunctionSequenceException(*this);
            }
        }
    }
    else if (SQL_ISRULE(m_pParseTree,update_statement_positioned) ||
               SQL_ISRULE(m_pParseTree,update_statement_searched))
    {
        if(m_aAssignValues.isValid())
            m_aAssignValues->clear();
        m_aAssignValues = new OAssignValues(Reference<XIndexAccess>(m_xColNames,UNO_QUERY)->getCount());

        OSL_ENSURE(m_pParseTree->count() >= 4,"OResultSet: Fehler im Parse Tree");

        OSQLParseNode * pAssignmentCommalist = m_pParseTree->getChild(3);
        OSL_ENSURE(pAssignmentCommalist != NULL,"OResultSet: pAssignmentCommalist == NULL");
        OSL_ENSURE(SQL_ISRULE(pAssignmentCommalist,assignment_commalist),"OResultSet: Fehler im Parse Tree");
        OSL_ENSURE(pAssignmentCommalist->count() > 0,"OResultSet: pAssignmentCommalist->count() <= 0");

        // Alle Zuweisungen (Kommaliste) bearbeiten ...
        ::std::vector< String> aList(1);
        for (sal_uInt32 i = 0; i < pAssignmentCommalist->count(); i++)
        {
            OSQLParseNode * pAssignment = pAssignmentCommalist->getChild(i);
            OSL_ENSURE(pAssignment != NULL,"OResultSet: pAssignment == NULL");
            OSL_ENSURE(SQL_ISRULE(pAssignment,assignment),"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(pAssignment->count() == 3,"OResultSet: pAssignment->count() != 3");

            OSQLParseNode * pCol = pAssignment->getChild(0);
            OSL_ENSURE(pCol != NULL,"OResultSet: pCol == NULL");

            OSQLParseNode * pComp = pAssignment->getChild(1);
            OSL_ENSURE(pComp != NULL,"OResultSet: pComp == NULL");
            OSL_ENSURE(pComp->getNodeType() == SQL_NODE_EQUAL,"OResultSet: pComp->getNodeType() != SQL_NODE_COMPARISON");
            if (pComp->getTokenValue().toChar() != '=')
            {
                //  aStatus.SetInvalidStatement();
                throwFunctionSequenceException(*this);
            }

            OSQLParseNode * pVal = pAssignment->getChild(2);
            OSL_ENSURE(pVal != NULL,"OResultSet: pVal == NULL");
            aList[0] = pCol->getTokenValue();
            ParseAssignValues(aList,pVal,0);
        }

    }
}
// -------------------------------------------------------------------------
void OResultSet::ParseAssignValues(const ::std::vector< String>& aColumnNameList,OSQLParseNode* pRow_Value_Constructor_Elem,xub_StrLen nIndex)
{
    OSL_ENSURE(nIndex <= aColumnNameList.size(),"SdbFileCursor::ParseAssignValues: nIndex > aColumnNameList.GetTokenCount()");
    String aColumnName(aColumnNameList[nIndex]);
    OSL_ENSURE(aColumnName.Len() > 0,"OResultSet: Column-Name nicht gefunden");
    OSL_ENSURE(pRow_Value_Constructor_Elem != NULL,"OResultSet: pRow_Value_Constructor_Elem darf nicht NULL sein!");

    if (pRow_Value_Constructor_Elem->getNodeType() == SQL_NODE_STRING ||
        pRow_Value_Constructor_Elem->getNodeType() == SQL_NODE_INTNUM ||
        pRow_Value_Constructor_Elem->getNodeType() == SQL_NODE_APPROXNUM)
    {
        // Wert setzen:
        SetAssignValue(aColumnName, pRow_Value_Constructor_Elem->getTokenValue());
    }
    else if (SQL_ISTOKEN(pRow_Value_Constructor_Elem,NULL))
    {
        // NULL setzen
        SetAssignValue(aColumnName, String(), TRUE);
    }
    else if (SQL_ISRULE(pRow_Value_Constructor_Elem,parameter))
    {
        // Parameter hinzufuegen, Typ ... entsprechend der Column, der der Wert zugewiesen wird
        Reference<XPropertySet> xCol;
        m_xColNames->getByName(aColumnName) >>= xCol;
        sal_Int32 nParameter = -1;
        if(m_xParamColumns.isValid())
        {
            OSQLColumns::const_iterator aIter = find(m_xParamColumns->begin(),m_xParamColumns->end(),aColumnName,::comphelper::UStringMixEqual(m_pTable->isCaseSensitive()));
            if(aIter != m_xParamColumns->end())
                nParameter = m_xParamColumns->size() - (m_xParamColumns->end() - aIter) +1;// +1 because the rows start at 1
        }
        if(nParameter == -1)
            nParameter = AddParameter(pRow_Value_Constructor_Elem,xCol);
        // Nr. des Parameters in der Variablen merken:
        SetAssignValue(aColumnName, String(), TRUE, nParameter);
    }
    else
    {
        //  aStatus.SetStatementTooComplex();
        throwFunctionSequenceException(*this);
    }
}
//------------------------------------------------------------------
void OResultSet::SetAssignValue(const String& aColumnName,
                                   const String& aValue,
                                   BOOL bSetNull,
                                   UINT32 nParameter)
{
    Reference<XPropertySet> xCol;
    ::cppu::extractInterface(xCol,m_xColNames->getByName(aColumnName));
    sal_Int32 nId = Reference<XColumnLocate>(m_xColNames,UNO_QUERY)->findColumn(aColumnName);
    // Kommt diese Column ueberhaupt in der Datei vor?

    if (!xCol.is())
    {
        // Diese Column gibt es nicht!
//      aStatus.Set(SQL_STAT_ERROR,
//                  String::CreateFromAscii("S0022"),
//                  aStatus.CreateErrorMessage(String(SdbResId(STR_STAT_COLUMN_NOT_FOUND))),
//                  0, String() );
        throwFunctionSequenceException(*this);
    }

    // Value an die Row mit den zuzuweisenden Werten binden:
    //  const ODbVariantRef& xValue = (*aAssignValues)[pFileColumn->GetId()];

    // Alles geprueft und wir haben den Namen der Column.
    // Jetzt eine Value allozieren, den Wert setzen und die Value an die Row binden.
    if (bSetNull)
        (*m_aAssignValues)[nId].setNull();
    else
    {
        switch (getINT32(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))))
        {
            // Kriterium je nach Typ als String oder double in die Variable packen ...
            case DataType::CHAR:
            case DataType::VARCHAR:
                (*m_aAssignValues)[nId] = aValue;
                // Zeichensatz ist bereits konvertiert, da ja das gesamte Statement konvertiert wurde
                break;

            case DataType::BIT:
                {
                    // nyi: gemischte Gross-/Kleinschreibung???
                    if (aValue.EqualsIgnoreCaseAscii("TRUE")  || aValue.GetChar(0) == '1')
                        (*m_aAssignValues)[nId] = sal_True;
                    else if (aValue.EqualsIgnoreCaseAscii("FALSE") || aValue.GetChar(0) == '0')
                        (*m_aAssignValues)[nId] = sal_False;
                    else
                    {
                        //  aStatus.Set(SQL_STAT_ERROR);    // nyi: genauer!
                        throwFunctionSequenceException(*this);
                    }
                }
                break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::REAL:
            case DataType::DOUBLE:
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            {
                (*m_aAssignValues)[nId] = aValue; // .ToDouble
//              try
//              {
//                  double n = xValue->toDouble();
//                  xValue->setDouble(n);
//              }
//              catch ( ... )
//              {
//                  aStatus.SetDriverNotCapableError();
//              }
            }   break;
            default:
                throwFunctionSequenceException(*this);
        }
    }

    // Parameter-Nr. merken (als User Data)
    // SQL_NO_PARAMETER = kein Parameter.
    m_aAssignValues->setParameterIndex(nId,nParameter);
    //  ((ODbAssignRow*)aAssignValues.getBodyPtr())->setAssignedIndex(pFileColumn->GetId(), nParameter);
}

//------------------------------------------------------------------
UINT32 OResultSet::AddParameter(OSQLParseNode * pParameter, const Reference<XPropertySet>& _xCol)
{

    // Nr. des neu hinzuzufuegenden Parameters:
    UINT32 nParameter = m_xParamColumns->size()+1;

    OSL_ENSURE(SQL_ISRULE(pParameter,parameter),"OResultSet::AddParameter: Argument ist kein Parameter");
    OSL_ENSURE(pParameter->count() > 0,"OResultSet: Fehler im Parse Tree");
    OSQLParseNode * pMark = pParameter->getChild(0);

    String aParameterName;
    if (SQL_ISPUNCTUATION(pMark,"?"))
        aParameterName = '?';
    else if (SQL_ISPUNCTUATION(pMark,":"))
        aParameterName = pParameter->getChild(1)->getTokenValue();
    else if (SQL_ISPUNCTUATION(pMark,"["))
        aParameterName = pParameter->getChild(1)->getTokenValue();
    else
    {
        OSL_ASSERT("OResultSet: Fehler im Parse Tree");
    }

    // Parameter-Column aufsetzen:
    sal_Int32 eType = DataType::VARCHAR;
    UINT32 nPrecision = 255;
    sal_Int32 nScale = 0;
    sal_Int32 nNullable = ColumnValue::NULLABLE;

    if (_xCol.is())
    {
        // Typ, Precision, Scale ... der angegebenen Column verwenden,
        // denn dieser Column wird der Wert zugewiesen bzw. mit dieser
        // Column wird der Wert verglichen.
        eType = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));
        nPrecision = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)));
        nScale = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)));
        nNullable = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)));;
    }

    Reference<XPropertySet> xParaColumn = new connectivity::sdbcx::OColumn(aParameterName
                                                    ,::rtl::OUString()
                                                    ,::rtl::OUString()
                                                    ,nNullable
                                                    ,nPrecision
                                                    ,nScale
                                                    ,eType
                                                    ,sal_False
                                                    ,sal_False
                                                    ,sal_False
                                                    ,m_aSQLIterator.isCaseSensitive());
    m_xParamColumns->push_back(xParaColumn);
    return nParameter;
}
// -----------------------------------------------------------------------------
void OResultSet::describeColumn(OSQLParseNode* _pParameter,OSQLParseNode* _pNode,const OSQLTable& _xTable)
{
    Reference<XPropertySet> xProp;
    if(SQL_ISRULE(_pNode,column_ref))
    {
        ::rtl::OUString sColumnName,sTableRange;
        m_aSQLIterator.getColumnRange(_pNode,sColumnName,sTableRange);
        if(sColumnName.getLength())
        {
            Reference<XNameAccess> xNameAccess = _xTable->getColumns();
            if(xNameAccess->hasByName(sColumnName))
                xNameAccess->getByName(sColumnName) >>= xProp;;
            AddParameter(_pParameter,xProp);
        }
    }
    else
        AddParameter(_pParameter,xProp);
//  else if(SQL_ISRULE(_pNode,num_value_exp))
//  {
//      describeColumn(*aIter,_pNode->getChild(),_xTable);
//  }
//  else if(SQL_ISRULE(_pNode,term))
//  {
//      describeColumn(*aIter,,_xTable);
//  }
//  else
//  {
//      describeColumn(*aIter,,_xTable);
//  }
}
// -------------------------------------------------------------------------
void OResultSet::describeParameter()
{
    ::std::vector< OSQLParseNode*> aParseNodes;
    scanParameter(m_pParseTree,aParseNodes);
    if(aParseNodes.size())
    {
        m_xParamColumns = new OSQLColumns();
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        if(xTabs.size())
        {
            OSQLTable xTable = xTabs.begin()->second;
            ::std::vector< OSQLParseNode*>::const_iterator aIter = aParseNodes.begin();
            for (;aIter != aParseNodes.end();++aIter )
            {
                describeColumn(*aIter,(*aIter)->getParent()->getChild(0),xTable);
            }
        }

//      ::vos::ORef<OSQLColumns> xColumns = m_aSQLIterator.getParameters();
//      if(xColumns.isValid())
//      {
//          OSL_ENSURE(aParseNodes.size() == xColumns->size(),"Not all parameters found!");
//          ::rtl::OUString aTabName,aColName,aParameterName;
//          OSQLColumns::iterator aIter = xColumns->begin();
//          for(;aIter != xColumns->end();++aIter)
//          {
//              m_xParamColumns->push_back(*aIter);
//          }
//      }
    }
}
// -----------------------------------------------------------------------------
void OResultSet::setBoundedColumns(const OValueRow& _rRow,
                                   const ::vos::ORef<connectivity::OSQLColumns>& _rxColumns,
                                   const Reference<XIndexAccess>& _xNames,
                                   sal_Bool _bSetColumnMapping)
{
    ::comphelper::UStringMixEqual aCase(m_xDBMetaData->storesMixedCaseQuotedIdentifiers());

    Reference<XPropertySet> xTableColumn;
    ::rtl::OUString sTableColumnName, sSelectColumnRealName;
    OValueVector::iterator aRowIter = _rRow->begin()+1;
    for (sal_Int32 i=0; // the first column is the bookmark column
            aRowIter != _rRow->end();
            ++i, ++aRowIter
        )
    {
        try
        {
            // get the table column and it's name
            _xNames->getByIndex(i) >>= xTableColumn;
            OSL_ENSURE(xTableColumn.is(), "OResultSet::OpenImpl: invalid table column!");
            if (xTableColumn.is())
                xTableColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sTableColumnName;
            else
                sTableColumnName = ::rtl::OUString();

            // look if we have such a select column
            // TODO: would like to have a O(log n) search here ...
            for (   OSQLColumns::iterator aIter = _rxColumns->begin();
                    aIter != _rxColumns->end();
                    ++aIter
                )
            {
                if((*aIter)->getPropertySetInfo()->hasPropertyByName(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME)))
                    (*aIter)->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME)) >>= sSelectColumnRealName;
                else
                    (*aIter)->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sSelectColumnRealName;

                if (aCase(sTableColumnName, sSelectColumnRealName))
                {
                    if(_bSetColumnMapping)
                    {
                        sal_Int32 nSelectColumnPos = aIter - m_xColumns->begin() + 1;
                            // the getXXX methods are 1-based ...
                        sal_Int32 nTableColumnPos = i + 1;
                            // get first table column is the bookmark column ...
                        m_aColMapping[nSelectColumnPos] = nTableColumnPos;
                    }

                    aRowIter->setBound(sal_True);
                    sal_Int32 nType = DataType::OTHER;
                    if (xTableColumn.is())
                        xTableColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)) >>= nType;
                    aRowIter->setTypeKind(nType);
                }
            }
        }
        catch (Exception&)
        {
            OSL_ENSURE(sal_False, "OResultSet::OpenImpl: caught an Exception!");
        }
    }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::acquire() throw(RuntimeException)
{
    OResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::release() throw(RuntimeException)
{
    OResultSet_BASE::release();
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
OSQLAnalyzer* OResultSet::createAnalyzer()
{
    return new OSQLAnalyzer();
}
// -----------------------------------------------------------------------------
void OResultSet::doTableSpecials(const OSQLTable& _xTable)
{
    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(_xTable,UNO_QUERY);
    if(xTunnel.is())
    {
        m_pTable = (OFileTable*)xTunnel->getSomething(OFileTable::getUnoTunnelImplementationId());
        if(m_pTable)
            m_pTable->acquire();
    }
}
// -----------------------------------------------------------------------------
void OResultSet::clearInsertRow()
{
    m_aRow->setDeleted(sal_False); // set to false here because this is the new row
    OValueVector::iterator aIter = m_aInsertRow->begin();
    for(sal_Int32 nPos = 0;aIter != m_aInsertRow->end();++aIter,++nPos)
    {
        if (aIter->isBound())
        {
            (*m_aRow)[nPos] = (*aIter);
        }
        aIter->setBound(nPos == 0);
        aIter->setModified(sal_False);
        aIter->setNull();
    }
}
// -----------------------------------------------------------------------------

