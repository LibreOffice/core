/*************************************************************************
 *
 *  $RCSfile: FResultSet.cxx,v $
 *
 *  $Revision: 1.88 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:25:33 $
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
DBG_NAME( file_OResultSet )
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
                        ,m_pSQLAnalyzer(NULL)
                        ,m_aSkipDeletedSet(this)
{
    DBG_CTOR( file_OResultSet, NULL );
    osl_incrementInterlockedCount( &m_refCount );

    m_nResultSetConcurrency = isCount() ? ResultSetConcurrency::READ_ONLY : ResultSetConcurrency::UPDATABLE;
    construct();
    osl_decrementInterlockedCount( &m_refCount );
}

// -------------------------------------------------------------------------
OResultSet::~OResultSet()
{
    osl_incrementInterlockedCount( &m_refCount );
    disposing();
    DBG_DTOR( file_OResultSet, NULL );
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
    m_xColsIdx      = NULL;

    if(m_pTable)
    {
        m_pTable->release();
        m_pTable = NULL;
    }
    clear();
}
// -----------------------------------------------------------------------------
void OResultSet::clear()
{
    m_pFileSet = NULL;
    DELETEZ(m_pSortIndex);

    if(m_aInsertRow.isValid())
        m_aInsertRow->clear();

    m_aSkipDeletedSet.clear();
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet_BASE::queryInterface(rType);
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

    //columnIndex = mapColumn(columnIndex);
    checkIndex(columnIndex );


    m_bWasNull = (*m_aSelectRow)[columnIndex]->getValue().isNull();
    return *(*m_aSelectRow)[columnIndex];
}
// -----------------------------------------------------------------------------
void OResultSet::checkIndex(sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException)
{
    if (   columnIndex <= 0
        //  || columnIndex > (sal_Int32)m_xColumns->size()
        || columnIndex >= (sal_Int32)m_aSelectRow->size() )
        ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
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

    OSL_ENSURE((m_bShowDeleted || !m_aRow->isDeleted()),"getRow called for deleted row");

    return m_aSkipDeletedSet.getMappedPosition((*m_aRow)[0]->getValue());
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
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
//  {
//      ::osl::MutexGuard aGuard( m_aMutex );
//      checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
//
//  }
//  dispose();
    ::osl::MutexGuard aGuard( m_aMutex );
    clear();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::first(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::FIRST,1,sal_True) : sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::last(  ) throw(SQLException, RuntimeException)
{
    // here I know definitely that I stand on the last record
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::LAST,1,sal_True) : sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::ABSOLUTE,row,sal_True) : sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::RELATIVE,row,sal_True) : sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::PRIOR,0,sal_True) : sal_False;
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
            bRet = m_pTable->seekRow(IResultSetHelper::BOOKMARK,(*m_aEvaluateIter),m_nRowPos);
            ++m_aEvaluateIter;
        }
        else
            bRet = m_pTable->seekRow(IResultSetHelper::NEXT,1,m_nRowPos);
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

    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::NEXT,1,sal_True) : sal_False;
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


    if(!m_bInserted || !m_pTable)
        throwFunctionSequenceException(*this);

    // we know that we append new rows at the end
    // so we have to know where the end is
    m_aSkipDeletedSet.skipDeleted(IResultSetHelper::LAST,1,sal_False);
    m_bRowInserted = m_pTable->InsertRow(m_aInsertRow.getBody(), TRUE,Reference<XIndexAccess>(m_xColNames,UNO_QUERY));
    if(m_bRowInserted && m_pFileSet.isValid())
    {
        sal_Int32 nPos = (*m_aInsertRow)[0]->getValue();
        m_pFileSet->push_back(nPos);
        *(*m_aInsertRow)[0] = sal_Int32(m_pFileSet->size());
        clearInsertRow();

        m_aSkipDeletedSet.insertNewPosition((*m_aRow)[0]->getValue());
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!m_pTable || m_pTable->isReadOnly())
        throw SQLException(::rtl::OUString::createFromAscii("Table is readonly!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
    m_bRowUpdated = m_pTable->UpdateRow(m_aInsertRow.getBody(), m_aRow,Reference<XIndexAccess>(m_xColNames,UNO_QUERY));
    *(*m_aInsertRow)[0] = (sal_Int32)(*m_aRow)[0]->getValue();

    clearInsertRow();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::deleteRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_pTable || m_pTable->isReadOnly())
        throw SQLException(::rtl::OUString::createFromAscii("Table is readonly!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
    if (m_bShowDeleted)
        throw SQLException(::rtl::OUString::createFromAscii("Row could not be deleted. The option \"Display inactive records\" is set!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
    if(m_aRow->isDeleted())
        throw SQLException(::rtl::OUString::createFromAscii("Row was already deleted!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());

    sal_Int32 nPos = (sal_Int32)(*m_aRow)[0]->getValue();
    m_bRowDeleted = m_pTable->DeleteRow(m_xColumns.getBody());
    if(m_bRowDeleted && m_pFileSet.isValid())
    {
        m_aRow->setDeleted(sal_True);
        // don't touch the m_pFileSet member here
        m_aSkipDeletedSet.deletePosition(nPos);
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
        OValueRefVector::iterator aIter = m_aInsertRow->begin()+1;
        for(;aIter != m_aInsertRow->end();++aIter)
        {
            (*aIter)->setBound(sal_False);
            (*aIter)->setNull();
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!m_pTable || m_pTable->isReadOnly())
        throw SQLException(::rtl::OUString::createFromAscii("Table is readonly!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());

    m_bInserted     = sal_True;

    OValueRefVector::iterator aIter = m_aInsertRow->begin()+1;
    for(;aIter != m_aInsertRow->end();++aIter)
    {
        (*aIter)->setBound(sal_False);
        (*aIter)->setNull();
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

    columnIndex = mapColumn(columnIndex);
    checkIndex(columnIndex );

    (*m_aInsertRow)[columnIndex]->setBound(sal_True);
    *(*m_aInsertRow)[columnIndex] = x;
}
// -----------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    columnIndex = mapColumn(columnIndex);
    checkIndex(columnIndex );



    (*m_aInsertRow)[columnIndex]->setBound(sal_True);
    (*m_aInsertRow)[columnIndex]->setNull();

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
BOOL OResultSet::ExecuteRow(IResultSetHelper::Movement eFirstCursorPosition,
                               INT32 nFirstOffset,
                               BOOL bRebind,
                               BOOL bEvaluate,
                               BOOL bRetrieveData)
{
    OSL_ENSURE(m_pSQLAnalyzer,"OResultSet::ExecuteRow: Analyzer isn't set!");

    // Fuer weitere Fetch-Operationen werden diese Angaben ggf. veraendert ...
    IResultSetHelper::Movement eCursorPosition = eFirstCursorPosition;
    INT32  nOffset = nFirstOffset;
    UINT32 nLoopCount = 0;

again:

    // protect from reading over the end when someboby is inserting while we are reading
    // this method works only for dBase at the moment !!!!
    if (eCursorPosition == IResultSetHelper::NEXT && m_nFilePos == m_nLastVisitedPos)
    {
        return sal_False;
    }

    if (!m_pTable || !m_pTable->seekRow(eCursorPosition, nOffset, m_nFilePos))
    {
        return sal_False;
    }

    if (!bEvaluate) // Laeuft keine Auswertung, dann nur Ergebniszeile fuellen
    {
        m_pTable->fetchRow(m_aRow,m_pTable->getTableColumns().getBody(), sal_True,bRetrieveData);
    }
    else
    {
        m_pTable->fetchRow(m_aEvaluateRow, m_pTable->getTableColumns().getBody(), sal_True,bRetrieveData || m_pSQLAnalyzer->hasRestriction());

        if ((!m_bShowDeleted && m_aEvaluateRow->isDeleted()) ||
            (m_pSQLAnalyzer->hasRestriction() && //!bShowDeleted && m_aEvaluateRow->isDeleted() ||// keine Anzeige von geloeschten S‰tzen
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
            else if (m_pFileSet.isValid())
            {
                OSL_ENSURE(//!m_pFileSet->IsFrozen() &&
                            eCursorPosition == IResultSetHelper::NEXT, "Falsche CursorPosition!");
                eCursorPosition = IResultSetHelper::NEXT;
                nOffset = 1;
            }
            else if (eCursorPosition == IResultSetHelper::FIRST ||
                     eCursorPosition == IResultSetHelper::NEXT ||
                     eCursorPosition == IResultSetHelper::ABSOLUTE)
            {
                eCursorPosition = IResultSetHelper::NEXT;
                nOffset = 1;
            }
            else if (eCursorPosition == IResultSetHelper::LAST ||
                     eCursorPosition == IResultSetHelper::PRIOR)
            {
                eCursorPosition = IResultSetHelper::PRIOR;
                nOffset = 1;
            }
            else if (eCursorPosition == IResultSetHelper::RELATIVE)
            {
                eCursorPosition = (nOffset >= 0) ? IResultSetHelper::NEXT : IResultSetHelper::PRIOR;
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

    if ( (bRetrieveData || m_pSQLAnalyzer->hasRestriction()) && m_pSQLAnalyzer->hasFunctions() )
    {
        m_pSQLAnalyzer->setSelectionEvaluationResult(m_aSelectRow,m_aColMapping);
    }
    // Evaluate darf nur gesetzt sein,
    // wenn der Keyset weiter aufgebaut werden soll
    if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT && !isCount() &&
        (m_pFileSet.isValid() || m_pSortIndex) && bEvaluate)
    {
        if (m_pSortIndex)
        {
            OKeyValue* pKeyValue = GetOrderbyKeyValue(m_aEvaluateRow);
            m_pSortIndex->AddKeyValue(pKeyValue);
        }
        else if (m_pFileSet.isValid())
        {
            //  OSL_ENSURE(!m_pFileSet->IsFrozen() , "Falsche CursorPosition!");
            sal_uInt32 nBookmarkValue = Abs((sal_Int32)(*m_aEvaluateRow)[0]->getValue());
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
BOOL OResultSet::Move(IResultSetHelper::Movement eCursorPosition, INT32 nOffset, BOOL bRetrieveData)
{

//IgnoreDeletedRows:
//
    INT32 nTempPos = m_nRowPos;
    // exclusiver zugriff auf die Tabelle
    //  NAMESPACE_VOS(OGuard)* pGuard = m_pTable->Lock();

    if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT &&
        !isCount())
    {
        if (!m_pFileSet.isValid()) // kein Index verfuegbar
        {
            // Normales FETCH
            ExecuteRow(eCursorPosition,nOffset,TRUE,FALSE,bRetrieveData);

            // now set the bookmark for outside this is the logical pos  and not the file pos
            *(*m_aRow->begin()) = sal_Int32(m_nRowPos + 1);
        }
        else
        {
            switch(eCursorPosition)
            {
                case IResultSetHelper::NEXT:
                    ++m_nRowPos;
                    break;
                case IResultSetHelper::PRIOR:
                    if (m_nRowPos >= 0)
                        --m_nRowPos;
                    break;
                case IResultSetHelper::FIRST:
                    m_nRowPos = 0;
                    break;
                case IResultSetHelper::LAST:
                    //  OSL_ENSURE(IsRowCountFinal(), "Fehler im Keyset!"); // muﬂ eingefroren sein, sonst Fehler beim SQLCursor
                    m_nRowPos = m_pFileSet->size() - 1;
                    break;
                case IResultSetHelper::RELATIVE:
                    m_nRowPos += nOffset;
                    break;
                case IResultSetHelper::ABSOLUTE:
                case IResultSetHelper::BOOKMARK:
                    m_nRowPos = nOffset -1;
                    break;
            }

            // OffRange?
            // Der FileCursor ist auﬂerhalb des gueltigen Bereichs, wenn
            // a.) m_nRowPos < 1
            // b.) Ein KeySet besteht und m_nRowPos > m_pFileSet->size()
            if (m_nRowPos < 0 || (m_pFileSet->isFrozen() && eCursorPosition != IResultSetHelper::BOOKMARK && m_nRowPos >= (INT32)m_pFileSet->size() )) // && m_pFileSet->IsFrozen()
            {
                //  aStatus.Set(SQL_STAT_NO_DATA_FOUND);
                goto Error;
            }
            else
            {
                if (m_nRowPos < (INT32)m_pFileSet->size())
                {
                    // Fetch ueber Index
                    ExecuteRow(IResultSetHelper::BOOKMARK,(*m_pFileSet)[m_nRowPos],TRUE,FALSE,bRetrieveData);

                    // now set the bookmark for outside
                    *(*m_aRow->begin()) = sal_Int32(m_nRowPos + 1);
                }
                else // Index muﬂ weiter aufgebaut werden
                {
                    // Zunaechst auf die letzte bekannte Zeile setzen
                    if (!m_pFileSet->empty())
                    {
                        m_aFileSetIter = m_pFileSet->end()-1;
                        //  m_pFileSet->SeekPos(m_pFileSet->size()-1);
                        m_pTable->seekRow(IResultSetHelper::BOOKMARK, *m_aFileSetIter, m_nFilePos);
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
                                bOK = ExecuteRow(IResultSetHelper::BOOKMARK,*m_aEvaluateIter,FALSE,TRUE, bRetrieveData);
                            }
                            else
                            {
                                ++m_aEvaluateIter;
                                bOK = ExecuteRow(IResultSetHelper::BOOKMARK,*m_aEvaluateIter,FALSE,TRUE, bRetrieveData);
                            }
                        }
                        else
                            bOK = ExecuteRow(IResultSetHelper::NEXT,1,FALSE,TRUE, FALSE);//bRetrieveData);
                    }

                    if (bOK)
                    {
                        // jetzt nochmal die Ergebnisse lesen
                        m_pTable->fetchRow(m_aRow, m_pTable->getTableColumns().getBody(), sal_True,bRetrieveData);

                        // now set the bookmark for outside
                        *(*m_aRow->begin()) = sal_Int32(m_nRowPos + 1);
                    }
                    else if (!m_pFileSet->isFrozen())                   // keinen gueltigen Satz gefunden
                    {
                        //m_pFileSet->Freeze();
                        m_pFileSet->setFrozen();

                        //  DELETEZ(m_pEvaluationKeySet);
                        m_pEvaluationKeySet = NULL;
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
            case IResultSetHelper::NEXT:
                ++m_nRowPos;
                break;
            case IResultSetHelper::PRIOR:
                --m_nRowPos;
                break;
            case IResultSetHelper::FIRST:
                m_nRowPos = 0;
                break;
            case IResultSetHelper::LAST:
                m_nRowPos = 0;
                break;
            case IResultSetHelper::RELATIVE:
                m_nRowPos += nOffset;
                break;
            case IResultSetHelper::ABSOLUTE:
            case IResultSetHelper::BOOKMARK:
                m_nRowPos = nOffset - 1;
                break;
        }

        if (m_nRowPos == 0)
        {
            // COUNT(*) in Ergebnisrow packen
            // (muss die erste und einzige Variable in der Row sein)
            if (m_aRow->size() >= 2)
            {
                *(*m_aRow)[1] = m_nRowCountResult;
                *(*m_aRow)[0] = sal_Int32(1);
                (*m_aRow)[1]->setBound(sal_True);
                (*m_aSelectRow)[1] = (*m_aRow)[1];
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
            case IResultSetHelper::PRIOR:
            case IResultSetHelper::FIRST:
                m_nRowPos = -1;
                break;
            case IResultSetHelper::LAST:
            case IResultSetHelper::NEXT:
            case IResultSetHelper::ABSOLUTE:
            case IResultSetHelper::RELATIVE:
                if (nOffset > 0)
                    m_nRowPos = (m_pFileSet.isValid()) ? m_pFileSet->size() : -1;
                else if (nOffset < 0)
                    m_nRowPos = -1;
                break;
            case IResultSetHelper::BOOKMARK:
                m_nRowPos = nTempPos;    // vorherige Position
        }
    }
    //  delete pGuard;
    //  rMode = (!bShowDeleted && aStatus.IsSuccessful() && m_aRow->isDeleted()) ?  // keine Anzeige von gelˆschten S‰tzen
                //  OCursor::SQL_MOD_INVALID : OCursor::SQL_MOD_NONE;
    return sal_False;
}
// -------------------------------------------------------------------------
BOOL OResultSet::OpenImpl()
{
    OSL_ENSURE(m_pSQLAnalyzer,"No analyzer set with setSqlAnalyzer!");
    if(!m_pTable)
    {
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        if ((xTabs.begin() == xTabs.end()) || !xTabs.begin()->second.is())
            throwGenericSQLException(   ::rtl::OUString::createFromAscii("The statement is invalid."),
                                        static_cast<XWeak*>(this),
                                        makeAny(m_aSQLIterator.getWarning())
                                    );
        if ( xTabs.size() > 1 || m_aSQLIterator.getWarning().Message.getLength() )
            throwGenericSQLException(   ::rtl::OUString::createFromAscii("The statement is invalid. it contains more than one table."),
                                        static_cast<XWeak*>(this),
                                        makeAny(m_aSQLIterator.getWarning()));

        OSQLTable xTable = xTabs.begin()->second;
        m_xColumns = m_aSQLIterator.getSelectColumns();

        m_xColNames = xTable->getColumns();
        m_xColsIdx = Reference<XIndexAccess>(m_xColNames,UNO_QUERY);
        doTableSpecials(xTable);
        Reference<XComponent> xComp(xTable,UNO_QUERY);
        if(xComp.is())
            xComp->addEventListener(this);
    }

    m_pTable->refreshHeader();

    sal_Int32 nColumnCount = m_xColsIdx->getCount();

    initializeRow(m_aRow,nColumnCount);
    initializeRow(m_aEvaluateRow,nColumnCount);
    initializeRow(m_aInsertRow,nColumnCount);


    m_nResultSetConcurrency = (m_pTable->isReadOnly() || isCount()) ? ResultSetConcurrency::READ_ONLY : ResultSetConcurrency::UPDATABLE;

    // Neuen Index aufbauen:
    m_pFileSet = NULL;
    //  DELETEZ(m_pEvaluationKeySet);

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
                if(m_xColumns->size() > 1)
                    throwGenericSQLException(::rtl::OUString::createFromAscii("Count can only be used as single column!"),*this );
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
                            ExecuteRow(IResultSetHelper::BOOKMARK,(*m_aEvaluateIter),FALSE,TRUE);
                        else
                            bOK = ExecuteRow(IResultSetHelper::NEXT,1,FALSE,TRUE);

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
                    m_pEvaluationKeySet = NULL;
                    //  DELETEZ(m_pEvaluationKeySet);
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
                        m_aOrderbyColumnNumber.push_back(m_aColMapping[1]);
                        m_aOrderbyAscending.push_back(SQL_DESC);
                    }
                    else
                        bWasSorted = TRUE;
                    bDistinct = TRUE;
                }

                OSortIndex::TKeyTypeVector eKeyType(m_aOrderbyColumnNumber.size());
                OValueRefVector::iterator aRowIter = m_aRow->begin()+1;
                ::std::vector<sal_Int32>::iterator aOrderByIter = m_aOrderbyColumnNumber.begin();
                for (::std::vector<sal_Int16>::size_type i=0;aOrderByIter != m_aOrderbyColumnNumber.end(); ++aOrderByIter,++i)
                {
                    OSL_ENSURE((sal_Int32)m_aRow->size() > *aOrderByIter,"Invalid Index");
                    switch ((*(m_aRow->begin()+*aOrderByIter))->getValue().getTypeKind())
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
                    (*m_aEvaluateRow)[*aOrderByIter]->setBound(sal_True);
                }

                // Nur wenn Sortierung gewuenscht, ueber alle Datensaetze iterieren und
                // dabei den "Key", nach dem sortiert werden soll, in den Index eintragen:
                if (IsSorted())
                {
                    if (!m_pSQLAnalyzer->hasRestriction() && m_aOrderbyColumnNumber.size() == 1)
                    {
                        // Ist nur ein Feld fuer die Sortierung angegeben
                        // Und diese Feld ist indiziert, dann den Index ausnutzen
                        Reference<XIndexesSupplier> xIndexSup;
                        m_pTable->queryInterface(::getCppuType((const Reference<XIndexesSupplier>*)0)) >>= xIndexSup;
                        //  Reference<XIndexesSupplier> xIndexSup(m_pTable,UNO_QUERY);
                        Reference<XIndexAccess> xIndexes;
                        if(xIndexSup.is())
                        {
                            xIndexes = Reference<XIndexAccess>(xIndexSup->getIndexes(),UNO_QUERY);
                            Reference<XPropertySet> xColProp;
                            if(m_aOrderbyColumnNumber[0] < xIndexes->getCount())
                            {
                                ::cppu::extractInterface(xColProp,xIndexes->getByIndex(m_aOrderbyColumnNumber[0]));
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

                    m_pSortIndex = new OSortIndex(eKeyType,m_aOrderbyAscending);

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
                            ExecuteRow(IResultSetHelper::BOOKMARK,(*m_aEvaluateIter),FALSE,TRUE);
                            ++m_aEvaluateIter;
                            bOK = m_aEvaluateIter == m_pEvaluationKeySet->end();
                        }
                        else
                            bOK = ExecuteRow(IResultSetHelper::NEXT,1,FALSE,TRUE);
                    }

                    // Sortiertes Keyset erzeugen
                    //  DELETEZ(m_pEvaluationKeySet);
                    m_pEvaluationKeySet = NULL;
                    m_pFileSet = NULL;
                    m_pFileSet = m_pSortIndex->CreateKeySet();
                    //  if(!bDistinct)
                        //  SetRowCount(pFileSet->count());
                    DELETEZ(m_pSortIndex);
                    // Nun kann ueber den Index sortiert zugegriffen werden.
                }

                if (!m_pFileSet.isValid())
                {
                    m_pFileSet = new OKeySet();

                    if (!m_pSQLAnalyzer->hasRestriction())
                    // jetzt kann das Keyset schon gefuellt werden!
                    // Aber Achtung: es wird davon ausgegangen, das die FilePositionen als Folge 1..n
                    // abgelegt werden!
                    {
                        if(m_pTable->getCurrentLastPos() > 0)
                            m_pFileSet->reserve(m_pTable->getCurrentLastPos() );
                        for (sal_Int32 i = 0; i < m_pTable->getCurrentLastPos(); i++)
                           m_pFileSet->push_back(i + 1);
                    }
                }
                OSL_ENSURE(m_pFileSet.isValid(),"Kein KeySet vorhanden! :-(");
    DISTINCT:   if(bDistinct && m_pFileSet.isValid())   // sicher ist sicher
                {
                    OValueRow aSearchRow = new OValueVector(m_aRow->size());
                    OValueRefVector::iterator aRowIter = m_aRow->begin();
                    OValueVector::iterator aSearchIter = aSearchRow->begin();
                    for (   ++aRowIter,++aSearchIter;   // the first column is the bookmark column
                            aRowIter != m_aRow->end();
                            ++aRowIter,++aSearchIter)
                                aSearchIter->setBound((*aRowIter)->isBound());

                    INT32 nPos;
                    UINT16 nMaxRow = m_pFileSet->size();
                    if (nMaxRow)
                    {
    #if OSL_DEBUG_LEVEL > 1
                        INT32 nFound=0;
    #endif
                        ::std::vector<sal_Int16> nWasAllwaysFound(nMaxRow,0);
                        INT32 nPrev_i;
                        for(INT32 j= nMaxRow-1;j >= 0;j--)
                        {
                            nPos = (*m_pFileSet)[j]; // aktuell zu lˆschender Key
                            if(!nWasAllwaysFound[j] && nPos) // nur falls noch nicht nach dieser Row gesucht wurde
                            {
                                ExecuteRow(IResultSetHelper::BOOKMARK,nPos,TRUE,FALSE);
                                { // copy row values
                                    OValueRefVector::iterator aRowIter = m_aRow->begin();
                                    OValueVector::iterator aSearchIter = aSearchRow->begin();
                                    for (   ++aRowIter,++aSearchIter;   // the first column is the bookmark column
                                            aRowIter != m_aRow->end();
                                            ++aRowIter,++aSearchIter)
                                                *aSearchIter = *(*aRowIter);
                                    // *aSearchRow = *m_aRow;
                                }

                                // jetzt den Rest nach doppelten durchsuchen
                                INT32 nKey;
                                nPrev_i = j;
                                for(INT32 i = j-1; i >= 0 ;i--)
                                {
                                    nKey = (*m_pFileSet)[i];
                                    ExecuteRow(IResultSetHelper::BOOKMARK,nKey ,TRUE,FALSE);
                                    if(!nWasAllwaysFound[i])
                                    {
                                        OValueRefVector::iterator aRowIter = m_aRow->begin();
                                        OValueVector::iterator aSearchIter = aSearchRow->begin();
                                        for (   ++aRowIter,++aSearchIter;   // the first column is the bookmark column
                                                aRowIter != m_aRow->end();
                                                ++aRowIter,++aSearchIter)
                                        {
                                            if ( (*aRowIter)->isBound() && !( *(*aRowIter) == *aSearchIter) )
                                                break;
                                        }
                                        if(aRowIter == m_aRow->end())
                                        {
                                            // gefunden
                                            // Key an der Stelle 0 setzen.
                                            (*m_pFileSet)[nPrev_i] = 0;
                                            // und altes i merken
                                            nPrev_i = i;
                                            nPos = nKey; // auf naechste g¸ltige Position setzen
                                            nWasAllwaysFound[i] = 1;
                                        }
                                    }
                                }
                            }
    #if OSL_DEBUG_LEVEL > 1
                            else
                                nFound++;
    #endif
                        }

                        m_pFileSet->erase(::std::remove_if(m_pFileSet->begin(),m_pFileSet->end(),
                                                            ::std::bind2nd(::std::equal_to<sal_Int32>(),0))
                                          ,m_pFileSet->end());

                        if (!bWasSorted)
                        {
                            m_aOrderbyColumnNumber.clear();
                            m_aOrderbyAscending.clear();
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
                        ExecuteRow(IResultSetHelper::BOOKMARK,(*m_aEvaluateIter),FALSE,TRUE);
                    else
                        bOK = ExecuteRow(IResultSetHelper::NEXT,1,FALSE,TRUE);

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
                //  DELETEZ(m_pEvaluationKeySet);
                m_pEvaluationKeySet = NULL;
            }
            //  SetRowCount(1);
            break;
        case SQL_STATEMENT_INSERT:
            m_nRowCountResult = 0;

            OSL_ENSURE(m_aAssignValues.isValid(),"No assign values set!");
            if(!m_pTable->InsertRow(m_aAssignValues.getBody(), TRUE,m_xColsIdx))
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
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ?
            (sal_Int64)this
                :
            0;
}
// -----------------------------------------------------------------------------
void OResultSet::setBoundedColumns(const OValueRefRow& _rRow,
                                   const OValueRefRow& _rSelectRow,
                                   const ::vos::ORef<connectivity::OSQLColumns>& _rxColumns,
                                   const Reference<XIndexAccess>& _xNames,
                                   sal_Bool _bSetColumnMapping,
                                   const Reference<XDatabaseMetaData>& _xMetaData,
                                   ::std::vector<sal_Int32>& _rColMapping)
{
    ::comphelper::UStringMixEqual aCase(_xMetaData->storesMixedCaseQuotedIdentifiers());

    Reference<XPropertySet> xTableColumn;
    ::rtl::OUString sTableColumnName, sSelectColumnRealName;

    const ::rtl::OUString sName     = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    const ::rtl::OUString sRealName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME);
    const ::rtl::OUString sType     = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE);

    typedef ::std::map<OSQLColumns::iterator,sal_Bool> IterMap;
    IterMap aSelectIters;
    OValueRefVector::iterator aRowIter = _rRow->begin()+1;
    for (sal_Int32 i=0; // the first column is the bookmark column
            aRowIter != _rRow->end();
            ++i, ++aRowIter
        )
    {
        (*aRowIter)->setBound(sal_False);
        try
        {
            // get the table column and it's name
            _xNames->getByIndex(i) >>= xTableColumn;
            OSL_ENSURE(xTableColumn.is(), "OResultSet::setBoundedColumns: invalid table column!");
            if (xTableColumn.is())
                xTableColumn->getPropertyValue(sName) >>= sTableColumnName;
            else
                sTableColumnName = ::rtl::OUString();

            // look if we have such a select column
            // TODO: would like to have a O(log n) search here ...
            for (   OSQLColumns::iterator aIter = _rxColumns->begin();
                    aIter != _rxColumns->end();
                    ++aIter
                )
            {
                if((*aIter)->getPropertySetInfo()->hasPropertyByName(sRealName))
                    (*aIter)->getPropertyValue(sRealName) >>= sSelectColumnRealName;
                else
                    (*aIter)->getPropertyValue(sName) >>= sSelectColumnRealName;

                if ( aCase(sTableColumnName, sSelectColumnRealName) && !(*aRowIter)->isBound() && aSelectIters.end() == aSelectIters.find(aIter) )
                {
                    aSelectIters.insert(IterMap::value_type(aIter,sal_True));
                    if(_bSetColumnMapping)
                    {
                        sal_Int32 nSelectColumnPos = aIter - _rxColumns->begin() + 1;
                            // the getXXX methods are 1-based ...
                        sal_Int32 nTableColumnPos = i + 1;
                            // get first table column is the bookmark column ...
                        _rColMapping[nSelectColumnPos] = nTableColumnPos;
                        (*_rSelectRow)[nSelectColumnPos] = *aRowIter;
                    }

                    (*aRowIter)->setBound(sal_True);
                    sal_Int32 nType = DataType::OTHER;
                    if (xTableColumn.is())
                        xTableColumn->getPropertyValue(sType) >>= nType;
                    (*aRowIter)->setTypeKind(nType);

                    break;
                }
            }
        }
        catch (Exception&)
        {
            OSL_ENSURE(sal_False, "OResultSet::setBoundedColumns: caught an Exception!");
        }
    }
    // in this case we got more select columns as columns exist in the table
    if ( _bSetColumnMapping && aSelectIters.size() != _rColMapping.size() )
    {
        Reference<XNameAccess> xNameAccess(_xNames,UNO_QUERY);
        Sequence< ::rtl::OUString > aSelectColumns = xNameAccess->getElementNames();

        for (   OSQLColumns::iterator aIter = _rxColumns->begin();
                    aIter != _rxColumns->end();
                    ++aIter
                )
        {
            if ( aSelectIters.end() == aSelectIters.find(aIter) )
            {
                if ( (*aIter)->getPropertySetInfo()->hasPropertyByName(sRealName) )
                    (*aIter)->getPropertyValue(sRealName) >>= sSelectColumnRealName;
                else
                    (*aIter)->getPropertyValue(sName) >>= sSelectColumnRealName;

                if ( xNameAccess->hasByName( sSelectColumnRealName ) )
                {
                    aSelectIters.insert(IterMap::value_type(aIter,sal_True));
                    sal_Int32 nSelectColumnPos = aIter - _rxColumns->begin() + 1;
                    const ::rtl::OUString* pBegin = aSelectColumns.getConstArray();
                    const ::rtl::OUString* pEnd   = pBegin + aSelectColumns.getLength();
                    for(sal_Int32 i=0;pBegin != pEnd;++pBegin,++i)
                    {
                        if ( aCase(*pBegin, sSelectColumnRealName) )
                        {
                            // the getXXX methods are 1-based ...
                            sal_Int32 nTableColumnPos = i + 1;
                                // get first table column is the bookmark column ...
                            _rColMapping[nSelectColumnPos] = nTableColumnPos;
                            (*_rSelectRow)[nSelectColumnPos] = (*_rRow)[nTableColumnPos];
                            break;
                        }
                    }
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::release() throw()
{
    OResultSet_BASE::release();
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
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
    OValueRefVector::iterator aIter = m_aInsertRow->begin();
    for(sal_Int32 nPos = 0;aIter != m_aInsertRow->end();++aIter,++nPos)
    {
        if ( (*aIter)->isBound() )
        {
            (*m_aRow)[nPos]->setValue( (*aIter)->getValue() );
        }
        (*aIter)->setBound(nPos == 0);
        (*aIter)->setModified(sal_False);
        (*aIter)->setNull();
    }
}
// -----------------------------------------------------------------------------
void OResultSet::initializeRow(OValueRefRow& _rRow,sal_Int32 _nColumnCount)
{
    if(!_rRow.isValid())
    {
        _rRow   = new OValueRefVector(_nColumnCount);
        (*_rRow)[0]->setBound(sal_True);
        ::std::for_each(_rRow->begin()+1,_rRow->end(),TSetRefBound(sal_False));
    }
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::fillIndexValues(const Reference< XColumnsSupplier> &_xIndex)
{
    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::move(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, sal_Bool _bRetrieveData)
{
    return Move(_eCursorPosition,_nOffset,_bRetrieveData);
}
// -----------------------------------------------------------------------------
sal_Int32 OResultSet::getDriverPos() const
{
    return (*m_aRow)[0]->getValue();
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::deletedVisible() const
{
    return m_bShowDeleted;
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::isRowDeleted() const
{
    return m_aRow->isDeleted();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::disposing( const EventObject& Source ) throw (RuntimeException)
{
    //  Reference<XInterface> xInt = m_pTable;
    Reference<XPropertySet> xProp = m_pTable;
    if(m_pTable && Source.Source == xProp)
    {
        m_pTable->release();
        m_pTable = NULL;
    }
}
// -----------------------------------------------------------------------------
