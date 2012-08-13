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


#ifdef GCC
#include <iostream>
#endif
#include "connectivity/sdbcx/VColumn.hxx"
#include "file/FResultSet.hxx"
#include "file/FResultSetMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/property.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include "connectivity/dbconversion.hxx"
#include "connectivity/dbtools.hxx"
#include <cppuhelper/propshlp.hxx>
#include <iterator>
#include <tools/debug.hxx>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>

#include <algorithm>
#include <comphelper/extract.hxx>
#include "connectivity/dbexception.hxx"
#include <comphelper/types.hxx>
#include "resource/file_res.hrc"
#include "resource/sharedresources.hxx"
#include <rtl/logfile.hxx>


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

// Maximal number of Rows, that can be processed being sorted with ORDER BY:
#define MAX_KEYSET_SIZE 0x40000 // 256K

namespace
{
    void lcl_throwError(sal_uInt16 _nErrorId,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xContext)
    {
        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sMessage = aResources.getResourceString(_nErrorId);
        ::dbtools::throwGenericSQLException(sMessage ,_xContext);
    }
}
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.drivers.file.ResultSet","com.sun.star.sdbc.ResultSet");
DBG_NAME( file_OResultSet )
// -------------------------------------------------------------------------
OResultSet::OResultSet(OStatement_Base* pStmt,OSQLParseTreeIterator&    _aSQLIterator) :    OResultSet_BASE(m_aMutex)
                        ,::comphelper::OPropertyContainer(OResultSet_BASE::rBHelper)
                        ,m_aAssignValues(NULL)
                        ,m_pEvaluationKeySet(NULL)
                        ,m_aSkipDeletedSet(this)
                        ,m_pFileSet(NULL)
                        ,m_pSortIndex(NULL)
                        ,m_pTable(NULL)
                        ,m_pParseTree(pStmt->getParseTree())
                        ,m_pSQLAnalyzer(NULL)
                        ,m_aSQLIterator(_aSQLIterator)
                        ,m_nFetchSize(0)
                        ,m_nResultSetType(ResultSetType::SCROLL_INSENSITIVE)
                        ,m_nFetchDirection(FetchDirection::FORWARD)
                        ,m_nResultSetConcurrency(ResultSetConcurrency::UPDATABLE)
                        ,m_xStatement(*pStmt)
                        ,m_xMetaData(NULL)
                        ,m_xDBMetaData(pStmt->getOwnConnection()->getMetaData())
                        ,m_nTextEncoding(pStmt->getOwnConnection()->getTextEncoding())
                        ,m_nRowPos(-1)
                        ,m_nFilePos(0)
                        ,m_nLastVisitedPos(-1)
                        ,m_nRowCountResult(-1)
                        ,m_nCurrentPosition(0)
                        ,m_nColumnCount(0)
                        ,m_bWasNull(sal_False)
                        ,m_bEOF(sal_False)
                        ,m_bLastRecord(sal_False)
                        ,m_bInserted(sal_False)
                        ,m_bRowUpdated(sal_False)
                        ,m_bRowInserted(sal_False)
                        ,m_bRowDeleted(sal_False)
                        ,m_bShowDeleted(pStmt->getOwnConnection()->showDeleted())
                        ,m_bIsCount(sal_False)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::OResultSet" );
    DBG_CTOR( file_OResultSet, NULL );
    osl_incrementInterlockedCount( &m_refCount );
    m_bIsCount = (m_pParseTree &&
            m_pParseTree->count() > 2                                                       &&
            SQL_ISRULE(m_pParseTree->getChild(2),scalar_exp_commalist)                      &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0),derived_column)               &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0)->getChild(0),general_set_fct) &&
            m_pParseTree->getChild(2)->getChild(0)->getChild(0)->count() == 4
            );

    m_nResultSetConcurrency = isCount() ? ResultSetConcurrency::READ_ONLY : ResultSetConcurrency::UPDATABLE;
    construct();
    m_aSkipDeletedSet.SetDeletedVisible(m_bShowDeleted);
    osl_decrementInterlockedCount( &m_refCount );
}

// -------------------------------------------------------------------------
OResultSet::~OResultSet()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::~OResultSet" );
    osl_incrementInterlockedCount( &m_refCount );
    disposing();
    DBG_DTOR( file_OResultSet, NULL );
}
// -------------------------------------------------------------------------
void OResultSet::construct()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::construct" );
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),           PROPERTY_ID_FETCHSIZE,          0,&m_nFetchSize,        ::getCppuType(static_cast<sal_Int32*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),        PROPERTY_ID_RESULTSETTYPE,      PropertyAttribute::READONLY,&m_nResultSetType,       ::getCppuType(static_cast<sal_Int32*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),      PROPERTY_ID_FETCHDIRECTION,     0,&m_nFetchDirection,   ::getCppuType(static_cast<sal_Int32*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY), PROPERTY_ID_RESULTSETCONCURRENCY,PropertyAttribute::READONLY,&m_nResultSetConcurrency,                ::getCppuType(static_cast<sal_Int32*>(0)));
}
// -------------------------------------------------------------------------
void OResultSet::disposing(void)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::disposing" );
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    m_xStatement.clear();
    m_xMetaData.clear();
    m_pParseTree    = NULL;
    m_xColNames.clear();
    m_xColumns = NULL;
    m_xParamColumns = NULL;
    m_xColsIdx.clear();

    Reference<XComponent> xComp = m_pTable;
    if ( xComp.is() )
        xComp->removeEventListener(this);
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::clear" );
    m_pFileSet = NULL;
    DELETEZ(m_pSortIndex);

    if(m_aInsertRow.is())
        m_aInsertRow->get().clear();

    m_aSkipDeletedSet.clear();
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::queryInterface" );
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet_BASE::queryInterface(rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OResultSet::getTypes(  ) throw(RuntimeException)
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getTypes" );
    ::osl::MutexGuard aGuard( m_aMutex );

    OTypeCollection aTypes( ::getCppuType( (const Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                            ::getCppuType( (const Reference< ::com::sun::star::beans::XPropertySet > *)0 ),
                            ::getCppuType( (const Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::findColumn" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getValue" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkIndex(columnIndex );


    m_bWasNull = (m_aSelectRow->get())[columnIndex]->getValue().isNull();
    return *(m_aSelectRow->get())[columnIndex];
}
// -----------------------------------------------------------------------------
void OResultSet::checkIndex(sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException)
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::checkIndex" );
    if (   columnIndex <= 0
            || columnIndex >= m_nColumnCount )
        ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getBinaryStream" );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getCharacterStream" );
    return NULL;
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getBoolean" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getByte" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getBytes" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

::com::sun::star::util::Date SAL_CALL OResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getDate" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

double SAL_CALL OResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getDouble" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

float SAL_CALL OResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getFloat" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getInt" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getRow" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_ENSURE((m_bShowDeleted || !m_aRow->isDeleted()),"getRow called for deleted row");

    return m_aSkipDeletedSet.getMappedPosition((m_aRow->get())[0]->getValue());
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getLong" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getMetaData" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_xColumns,m_aSQLIterator.getTables().begin()->first,m_pTable);
    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getArray" );
    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getClob" );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getBlob" );
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getRef" );
    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getObject" );
    return getValue(columnIndex).makeAny();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getShort" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getString" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL OResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getTime" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getTimestamp" );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::isAfterLast" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == sal_Int32(m_pFileSet->get().size());
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::isFirst" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::isLast" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == sal_Int32(m_pFileSet->get().size() - 1);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::beforeFirst" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(first())
        previous();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::afterLast" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(last())
        next();
    m_bEOF = sal_True;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::close(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::close" );
    dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::first(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::first" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::FIRST,1,sal_True) : sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::last(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::last" );
    // here I know definitely that I stand on the last record
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::LAST,1,sal_True) : sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::absolute" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::ABSOLUTE,row,sal_True) : sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::relative" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::RELATIVE,row,sal_True) : sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::previous" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::PRIOR,0,sal_True) : sal_False;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getStatement" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_xStatement;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::rowDeleted" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::rowInserted" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bRowUpdated;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::isBeforeFirst" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == -1;
}
// -------------------------------------------------------------------------
sal_Bool OResultSet::evaluate()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::evaluate" );
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
                bRet = m_pTable->fetchRow(m_aEvaluateRow,*(m_pTable->getTableColumns()),sal_True,sal_True);
                evaluate();

            }
            else
                bRet = m_pTable->fetchRow(m_aRow,*m_xColumns,sal_False,sal_True);
        }
    }

    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::next(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::next" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_pTable ? m_aSkipDeletedSet.skipDeleted(IResultSetHelper::NEXT,1,sal_True) : sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::wasNull" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::cancel" );
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::clearWarnings" );
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getWarnings" );
    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::insertRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::insertRow" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_bInserted || !m_pTable)
        throwFunctionSequenceException(*this);

    // we know that we append new rows at the end
    // so we have to know where the end is
    m_aSkipDeletedSet.skipDeleted(IResultSetHelper::LAST,1,sal_False);
    m_bRowInserted = m_pTable->InsertRow(*m_aInsertRow, sal_True, m_xColsIdx);
    if(m_bRowInserted && m_pFileSet.is())
    {
        sal_Int32 nPos = (m_aInsertRow->get())[0]->getValue();
        m_pFileSet->get().push_back(nPos);
        *(m_aInsertRow->get())[0] = sal_Int32(m_pFileSet->get().size());
        clearInsertRow();

        m_aSkipDeletedSet.insertNewPosition((m_aRow->get())[0]->getValue());
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateRow" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!m_pTable || m_pTable->isReadOnly())
        lcl_throwError(STR_TABLE_READONLY,*this);

    m_bRowUpdated = m_pTable->UpdateRow(*m_aInsertRow, m_aRow,m_xColsIdx);
    *(m_aInsertRow->get())[0] = (sal_Int32)(m_aRow->get())[0]->getValue();

    clearInsertRow();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::deleteRow() throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::deleteRow" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_pTable || m_pTable->isReadOnly())
        lcl_throwError(STR_TABLE_READONLY,*this);
    if (m_bShowDeleted)
        lcl_throwError(STR_DELETE_ROW,*this);
    if(m_aRow->isDeleted())
        lcl_throwError(STR_ROW_ALREADY_DELETED,*this);

    sal_Int32 nPos = (sal_Int32)(m_aRow->get())[0]->getValue();
    m_bRowDeleted = m_pTable->DeleteRow(*m_xColumns);
    if(m_bRowDeleted && m_pFileSet.is())
    {
        m_aRow->setDeleted(sal_True);
        // don't touch the m_pFileSet member here
        m_aSkipDeletedSet.deletePosition(nPos);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::cancelRowUpdates" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bInserted     = sal_False;
    m_bRowUpdated   = sal_False;
    m_bRowInserted  = sal_False;
    m_bRowDeleted   = sal_False;

    if(m_aInsertRow.is())
    {
        OValueRefVector::Vector::iterator aIter = m_aInsertRow->get().begin()+1;
        for(;aIter != m_aInsertRow->get().end();++aIter)
        {
            (*aIter)->setBound(sal_False);
            (*aIter)->setNull();
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::moveToInsertRow" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!m_pTable || m_pTable->isReadOnly())
        lcl_throwError(STR_TABLE_READONLY,*this);

    m_bInserted     = sal_True;

    OValueRefVector::Vector::iterator aIter = m_aInsertRow->get().begin()+1;
    for(;aIter != m_aInsertRow->get().end();++aIter)
    {
        (*aIter)->setBound(sal_False);
        (*aIter)->setNull();
    }
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::moveToCurrentRow" );
}
// -------------------------------------------------------------------------
void OResultSet::updateValue(sal_Int32 columnIndex ,const ORowSetValue& x) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateValue" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkIndex(columnIndex );
    columnIndex = mapColumn(columnIndex);

    (m_aInsertRow->get())[columnIndex]->setBound(sal_True);
    *(m_aInsertRow->get())[columnIndex] = x;
}
// -----------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateNull" );
    ORowSetValue aEmpty;
    updateValue(columnIndex,aEmpty);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateBoolean" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateByte" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateShort" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateInt" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateLong( sal_Int32 /*columnIndex*/, sal_Int64 /*x*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateLong" );
    ::dbtools::throwFeatureNotImplementedException( "XRowUpdate::updateLong", *this );
}
// -----------------------------------------------------------------------
void SAL_CALL OResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateFloat" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateDouble" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateString" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateBytes" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateDate" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateTime" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateTimestamp" );
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateBinaryStream" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    updateValue(columnIndex,aSeq);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateCharacterStream" );
    updateBinaryStream(columnIndex,x,length);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::refreshRow" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateObject" );
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 /*scale*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::updateNumericObject" );
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}
// -------------------------------------------------------------------------
IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::createArrayHelper" );
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getInfoHelper" );
    return *const_cast<OResultSet*>(this)->getArrayHelper();
}

//------------------------------------------------------------------
sal_Bool OResultSet::ExecuteRow(IResultSetHelper::Movement eFirstCursorPosition,
                               sal_Int32 nFirstOffset,
                               sal_Bool bEvaluate,
                               sal_Bool bRetrieveData)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::ExecuteRow" );
    OSL_ENSURE(m_pSQLAnalyzer,"OResultSet::ExecuteRow: Analyzer isn't set!");

    // For further Fetch-Operations this information may possibly be changed ...
    IResultSetHelper::Movement eCursorPosition = eFirstCursorPosition;
    sal_Int32  nOffset = nFirstOffset;

    const OSQLColumns & rTableCols = *(m_pTable->getTableColumns());
    sal_Bool bHasRestriction = m_pSQLAnalyzer->hasRestriction();
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

    if (!bEvaluate) // If no evaluation runs, then just fill the results-row
    {
        m_pTable->fetchRow(m_aRow,rTableCols, sal_True,bRetrieveData);
    }
    else
    {
        m_pTable->fetchRow(m_aEvaluateRow, rTableCols, sal_True,bRetrieveData || bHasRestriction);

        if  (   (   !m_bShowDeleted
                &&  m_aEvaluateRow->isDeleted()
                )
            ||  (   bHasRestriction
                &&  !m_pSQLAnalyzer->evaluateRestriction()
                )
            )
        {                                                // Evaluate the next record
            // delete current row in Keyset
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
            else if (m_pFileSet.is())
            {
                OSL_ENSURE(eCursorPosition == IResultSetHelper::NEXT, "Falsche CursorPosition!");
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
                return sal_False;
            }
            // Try again ...
            goto again;
        }
    }

    // Evaluate may only be set,
    // if the Keyset will be constructed further
    if  (   ( m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT )
        &&  !isCount()
        &&  bEvaluate
        )
    {
        if (m_pSortIndex)
        {
            OKeyValue* pKeyValue = GetOrderbyKeyValue( m_aSelectRow );
            m_pSortIndex->AddKeyValue(pKeyValue);
        }
        else if (m_pFileSet.is())
        {
            sal_uInt32 nBookmarkValue = Abs((sal_Int32)(m_aEvaluateRow->get())[0]->getValue());
            m_pFileSet->get().push_back(nBookmarkValue);
        }
    }
    else if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_UPDATE)
    {
        sal_Bool bOK = sal_True;
        if (bEvaluate)
        {
            // read the actual result-row
            bOK = m_pTable->fetchRow(m_aEvaluateRow, *(m_pTable->getTableColumns()), sal_True,sal_True);
        }

        if (bOK)
        {
            // just give the values to be changed:
            if(!m_pTable->UpdateRow(*m_aAssignValues,m_aEvaluateRow, m_xColsIdx))
                return sal_False;
        }
    }
    else if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_DELETE)
    {
        sal_Bool bOK = sal_True;
        if (bEvaluate)
        {
            bOK = m_pTable->fetchRow(m_aEvaluateRow, *(m_pTable->getTableColumns()), sal_True,sal_True);
        }
        if (bOK)
        {
            if(!m_pTable->DeleteRow(*m_xColumns))
                return sal_False;
        }
    }
    return sal_True;
}

//-------------------------------------------------------------------
sal_Bool OResultSet::Move(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Bool bRetrieveData)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::Move" );

//IgnoreDeletedRows:
//
    sal_Int32 nTempPos = m_nRowPos;

    if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT &&
        !isCount())
    {
        if (!m_pFileSet.is()) //no Index available
        {
            // Normal FETCH
            ExecuteRow(eCursorPosition,nOffset,sal_False,bRetrieveData);

            // now set the bookmark for outside this is the logical pos  and not the file pos
            *(*m_aRow->get().begin()) = sal_Int32(m_nRowPos + 1);
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
                    m_nRowPos = m_pFileSet->get().size() - 1;
                    break;
                case IResultSetHelper::RELATIVE:
                    m_nRowPos += nOffset;
                    break;
                case IResultSetHelper::ABSOLUTE:
                case IResultSetHelper::BOOKMARK:
                    if ( m_nRowPos == (nOffset -1) )
                        return sal_True;
                    m_nRowPos = nOffset -1;
                    break;
            }

            // OffRange?
            // The FileCursor is outside of the valid range, if:
            // a.) m_nRowPos < 1
            // b.) a KeySet exists and m_nRowPos > m_pFileSet->size()
            if (m_nRowPos < 0 || (m_pFileSet->isFrozen() && eCursorPosition != IResultSetHelper::BOOKMARK && m_nRowPos >= (sal_Int32)m_pFileSet->get().size() )) // && m_pFileSet->IsFrozen()
            {
                goto Error;
            }
            else
            {
                if (m_nRowPos < (sal_Int32)m_pFileSet->get().size())
                {
                    // Fetch via Index
                    ExecuteRow(IResultSetHelper::BOOKMARK,(m_pFileSet->get())[m_nRowPos],sal_False,bRetrieveData);

                    // now set the bookmark for outside
                    *(*m_aRow->get().begin()) = sal_Int32(m_nRowPos + 1);
                    if ( (bRetrieveData || m_pSQLAnalyzer->hasRestriction()) && m_pSQLAnalyzer->hasFunctions() )
                    {
                        m_pSQLAnalyzer->setSelectionEvaluationResult(m_aSelectRow,m_aColMapping);
                    }
                }
                else // Index must be further constructed
                {
                    // set first on the last known row
                    if (!m_pFileSet->get().empty())
                    {
                        m_aFileSetIter = m_pFileSet->get().end()-1;
                        m_pTable->seekRow(IResultSetHelper::BOOKMARK, *m_aFileSetIter, m_nFilePos);
                    }
                    sal_Bool bOK = sal_True;
                    // Determine the number of further Fetches
                    while (bOK && m_nRowPos >= (sal_Int32)m_pFileSet->get().size())
                    {
                        if (m_pEvaluationKeySet)
                        {
                            if (m_nRowPos >= (sal_Int32)m_pEvaluationKeySet->size())
                                return sal_False;
                            else if (m_nRowPos == 0)
                            {
                                m_aEvaluateIter = m_pEvaluationKeySet->begin();
                                bOK = ExecuteRow(IResultSetHelper::BOOKMARK,*m_aEvaluateIter,sal_True, bRetrieveData);
                            }
                            else
                            {
                                ++m_aEvaluateIter;
                                bOK = ExecuteRow(IResultSetHelper::BOOKMARK,*m_aEvaluateIter,sal_True, bRetrieveData);
                            }
                        }
                        else
                            bOK = ExecuteRow(IResultSetHelper::NEXT,1,sal_True, sal_False);//bRetrieveData);
                    }

                    if (bOK)
                    {
                        // read the results again
                        m_pTable->fetchRow(m_aRow, *(m_pTable->getTableColumns()), sal_True,bRetrieveData);

                        // now set the bookmark for outside
                        *(*m_aRow->get().begin()) = sal_Int32(m_nRowPos + 1);

                        if ( (bRetrieveData || m_pSQLAnalyzer->hasRestriction()) && m_pSQLAnalyzer->hasFunctions() )
                        {
                            m_pSQLAnalyzer->setSelectionEvaluationResult(m_aSelectRow,m_aColMapping);
                        }
                    }
                    else if (!m_pFileSet->isFrozen())                   // no valid record found
                    {
                        m_pFileSet->setFrozen();
                        m_pEvaluationKeySet = NULL;
                        goto Error;
                    }
                }
            }
        }
    }
    else if (m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT && isCount())
    {
        // Fetch the COUNT(*)
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

        if ( m_nRowPos < 0 )
            goto Error;
        else if (m_nRowPos == 0)
        {
            // put COUNT(*) in result-row
            // (must be the first and only variable in the row)
            if (m_aRow->get().size() >= 2)
            {
                *(m_aRow->get())[1] = m_nRowCountResult;
                *(m_aRow->get())[0] = sal_Int32(1);
                (m_aRow->get())[1]->setBound(sal_True);
                (m_aSelectRow->get())[1] = (m_aRow->get())[1];
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
        // Fetch only possible at SELECT!
        return sal_False;

    return sal_True;

Error:
    // is the Cursor positioned before the first row
    // then the position will be maintained
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
                    m_nRowPos = m_pFileSet.is() ? (sal_Int32)m_pFileSet->get().size() : -1;
                else if (nOffset < 0)
                    m_nRowPos = -1;
                break;
            case IResultSetHelper::BOOKMARK:
                m_nRowPos = nTempPos;    // last Position
        }
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void OResultSet::sortRows()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::sortRows" );
    if (!m_pSQLAnalyzer->hasRestriction() && m_aOrderbyColumnNumber.size() == 1)
    {
        // is just one field given for sorting
        // and this field is indexed, then the Index will be used
        Reference<XIndexesSupplier> xIndexSup;
        m_pTable->queryInterface(::getCppuType((const Reference<XIndexesSupplier>*)0)) >>= xIndexSup;

        Reference<XIndexAccess> xIndexes;
        if(xIndexSup.is())
        {
            xIndexes.set(xIndexSup->getIndexes(),UNO_QUERY);
            Reference<XPropertySet> xColProp;
            if(m_aOrderbyColumnNumber[0] < xIndexes->getCount())
            {
                xColProp.set(xIndexes->getByIndex(m_aOrderbyColumnNumber[0]),UNO_QUERY);
                // iterate through the indexes to find the matching column
                const sal_Int32 nCount = xIndexes->getCount();
                for(sal_Int32 i=0; i < nCount;++i)
                {
                    Reference<XColumnsSupplier> xIndex(xIndexes->getByIndex(i),UNO_QUERY);
                    Reference<XNameAccess> xIndexCols = xIndex->getColumns();
                    if(xIndexCols->hasByName(comphelper::getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)))))
                    {
                        m_pFileSet = new OKeySet();

                        if(fillIndexValues(xIndex))
                            return;
                    }
                }
            }
        }
    }

    OSortIndex::TKeyTypeVector eKeyType(m_aOrderbyColumnNumber.size());
    ::std::vector<sal_Int32>::iterator aOrderByIter = m_aOrderbyColumnNumber.begin();
    for (::std::vector<sal_Int16>::size_type i=0;aOrderByIter != m_aOrderbyColumnNumber.end(); ++aOrderByIter,++i)
    {
        OSL_ENSURE((sal_Int32)m_aSelectRow->get().size() > *aOrderByIter,"Invalid Index");
        switch ((*(m_aSelectRow->get().begin()+*aOrderByIter))->getValue().getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
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

        // Other types aren't implemented (so they are always FALSE)
            default:
                eKeyType[i] = SQL_ORDERBYKEY_NONE;
                OSL_FAIL("OFILECursor::Execute: Datentyp nicht implementiert");
                break;
        }
        (m_aSelectRow->get())[*aOrderByIter]->setBound(sal_True);
    }

    m_pSortIndex = new OSortIndex(eKeyType,m_aOrderbyAscending);

    if (m_pEvaluationKeySet)
    {
        m_aEvaluateIter = m_pEvaluationKeySet->begin();

        while (m_aEvaluateIter != m_pEvaluationKeySet->end())
        {
            ExecuteRow(IResultSetHelper::BOOKMARK,(*m_aEvaluateIter),sal_True);
            ++m_aEvaluateIter;
        }
    }
    else
    {
        while ( ExecuteRow( IResultSetHelper::NEXT, 1, sal_False, sal_True ) )
        {
            m_aSelectRow->get()[0]->setValue( m_aRow->get()[0]->getValue() );
            if ( m_pSQLAnalyzer->hasFunctions() )
                m_pSQLAnalyzer->setSelectionEvaluationResult( m_aSelectRow, m_aColMapping );
            const sal_Int32 nBookmark = (*m_aRow->get().begin())->getValue();
            ExecuteRow( IResultSetHelper::BOOKMARK, nBookmark, sal_True, sal_False );
        }
    }

    // create sorted Keyset
    m_pEvaluationKeySet = NULL;
    m_pFileSet = NULL;
    m_pFileSet = m_pSortIndex->CreateKeySet();
    DELETEZ(m_pSortIndex);
    // now access to a sorted set is possible via Index
}


// -------------------------------------------------------------------------
sal_Bool OResultSet::OpenImpl()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::OpenImpl" );
    OSL_ENSURE(m_pSQLAnalyzer,"No analyzer set with setSqlAnalyzer!");
    if(!m_pTable)
    {
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        if ((xTabs.begin() == xTabs.end()) || !xTabs.begin()->second.is())
            lcl_throwError(STR_QUERY_TOO_COMPLEX,*this);

        if ( xTabs.size() > 1 || m_aSQLIterator.hasErrors() )
            lcl_throwError(STR_QUERY_MORE_TABLES,*this);

        OSQLTable xTable = xTabs.begin()->second;
        m_xColumns = m_aSQLIterator.getSelectColumns();

        m_xColNames = xTable->getColumns();
        m_xColsIdx.set(m_xColNames,UNO_QUERY);
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

    // create new Index:
    m_pFileSet = NULL;

    // position at the beginning
    m_nRowPos = -1;
    m_nFilePos  = 0;
    m_nRowCountResult = -1;

    m_nLastVisitedPos = m_pTable->getCurrentLastPos();

    switch(m_aSQLIterator.getStatementType())
    {
        case SQL_STATEMENT_SELECT:
        {
            if(isCount())
            {
                if(m_xColumns->get().size() > 1)
                    lcl_throwError(STR_QUERY_COMPLEX_COUNT,*this);

                m_nRowCountResult = 0;
                // for now simply iterate over all rows and
                // do all actions (or just count)
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
                            ExecuteRow(IResultSetHelper::BOOKMARK,(*m_aEvaluateIter),sal_True);
                        else
                            bOK = ExecuteRow(IResultSetHelper::NEXT,1,sal_True);

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

                    // save result of COUNT(*) in m_nRowCountResult.
                    // nRowCount (number of Rows in the result) = 1 for this request!
                    m_pEvaluationKeySet = NULL;
                }
            }
            else
            {
                sal_Bool bDistinct = sal_False;
                OSQLParseNode *pDistinct = m_pParseTree->getChild(1);

                assert(m_aOrderbyColumnNumber.size() ==
                       m_aOrderbyAscending.size());
                if (pDistinct && pDistinct->getTokenID() == SQL_TOKEN_DISTINCT )
                {
                    // To eliminate duplicates we need to sort on all columns.
                    // This is not a problem because the SQL spec says that the
                    // order of columns that are not specified in ORDER BY
                    // clause is undefined, so it doesn't hurt to sort on
                    // these; pad the vectors to include them.
                    for (sal_Int32 i = 1; // 0: bookmark (see setBoundedColumns)
                         static_cast<size_t>(i) < m_aColMapping.size(); ++i)
                    {
                        if (::std::find(m_aOrderbyColumnNumber.begin(),
                                        m_aOrderbyColumnNumber.end(), i)
                                == m_aOrderbyColumnNumber.end())
                        {
                            m_aOrderbyColumnNumber.push_back(i);
                            // ASC or DESC doesn't matter
                            m_aOrderbyAscending.push_back(SQL_ASC);
                        }
                    }
                    bDistinct = sal_True;
                }

                if (IsSorted())
                    sortRows();

                if (!m_pFileSet.is())
                {
                    m_pFileSet = new OKeySet();

                    if (!m_pSQLAnalyzer->hasRestriction())
                    // now the Keyset can be filled!
                    // But be careful: It is assumed, that the FilePositions will be stored as sequence 1..n
                    {
                        if ( m_nLastVisitedPos > 0)
                            m_pFileSet->get().reserve( m_nLastVisitedPos );
                        for (sal_Int32 i = 0; i < m_nLastVisitedPos; i++)
                            m_pFileSet->get().push_back(i + 1);
                    }
                }
                OSL_ENSURE(m_pFileSet.is(),"Kein KeySet vorhanden! :-(");

                if(bDistinct && m_pFileSet.is())
                {
                    OValueRow aSearchRow = new OValueVector(m_aRow->get().size());
                    OValueRefVector::Vector::iterator aRowIter = m_aRow->get().begin();
                    OValueVector::Vector::iterator aSearchIter = aSearchRow->get().begin();
                    for (   ++aRowIter,++aSearchIter;   // the first column is the bookmark column
                            aRowIter != m_aRow->get().end();
                            ++aRowIter,++aSearchIter)
                        aSearchIter->setBound((*aRowIter)->isBound());

                    size_t nMaxRow = m_pFileSet->get().size();

                    if (nMaxRow)
                    {
    #if OSL_DEBUG_LEVEL > 1
                        sal_Int32 nFound=0;
    #endif
                        sal_Int32 nPos;
                        sal_Int32 nKey;

                        for( size_t j = nMaxRow-1; j > 0; --j)
                        {
                            nPos = (m_pFileSet->get())[j];
                            ExecuteRow(IResultSetHelper::BOOKMARK,nPos,sal_False);
                            m_pSQLAnalyzer->setSelectionEvaluationResult(m_aSelectRow,m_aColMapping);
                            { // copy row values
                                OValueRefVector::Vector::iterator copyFrom = m_aSelectRow->get().begin();
                                OValueVector::Vector::iterator copyTo = aSearchRow->get().begin();
                                for (   ++copyFrom,++copyTo;    // the first column is the bookmark column
                                        copyFrom != m_aSelectRow->get().end();
                                        ++copyFrom,++copyTo)
                                            *copyTo = *(*copyFrom);
                            }

                            // compare with next row
                            nKey = (m_pFileSet->get())[j-1];
                            ExecuteRow(IResultSetHelper::BOOKMARK,nKey,sal_False);
                            m_pSQLAnalyzer->setSelectionEvaluationResult(m_aSelectRow,m_aColMapping);
                            OValueRefVector::Vector::iterator loopInRow = m_aSelectRow->get().begin();
                            OValueVector::Vector::iterator existentInSearchRow = aSearchRow->get().begin();
                            for (   ++loopInRow,++existentInSearchRow;  // the first column is the bookmark column
                                    loopInRow != m_aSelectRow->get().end();
                                    ++loopInRow,++existentInSearchRow)
                            {
                                if ( (*loopInRow)->isBound() && !( *(*loopInRow) == *existentInSearchRow) )
                                    break;
                            }

                            if(loopInRow == m_aSelectRow->get().end())
                                (m_pFileSet->get())[j] = 0; // Rows match -- Mark for deletion by setting key to 0
    #if OSL_DEBUG_LEVEL > 1
                            else
                                nFound++;
    #endif
                        }

                        m_pFileSet->get().erase(::std::remove_if(m_pFileSet->get().begin(),m_pFileSet->get().end(),
                                                            ::std::bind2nd(::std::equal_to<sal_Int32>(),0))
                                          ,m_pFileSet->get().end());
                    }
                }
            }
        }   break;

        case SQL_STATEMENT_UPDATE:
        case SQL_STATEMENT_DELETE:
            // during processing count the number of processed Rows
            m_nRowCountResult = 0;
            // for now simply iterate over all rows and
            // run the actions (or simply count):
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
                        ExecuteRow(IResultSetHelper::BOOKMARK,(*m_aEvaluateIter),sal_True);
                    else
                        bOK = ExecuteRow(IResultSetHelper::NEXT,1,sal_True);

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

                // save result of COUNT(*) in nRowCountResult.
                // nRowCount (number of rows in the result-set) = 1 for this request!
                m_pEvaluationKeySet = NULL;
            }
            break;
        case SQL_STATEMENT_INSERT:
            m_nRowCountResult = 0;

            OSL_ENSURE(m_aAssignValues.is(),"No assign values set!");
            if(!m_pTable->InsertRow(*m_aAssignValues, sal_True,m_xColsIdx))
            {
                m_nFilePos  = 0;
                return sal_False;
            }

            m_nRowCountResult = 1;
            break;
        default:
            OSL_FAIL( "OResultSet::OpenImpl: unsupported statement type!" );
            break;
    }

    // reset FilePos
    m_nFilePos  = 0;

    return sal_True;
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OResultSet::getUnoTunnelImplementationId()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getUnoTunnelImplementationId" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getSomething" );
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : 0;
}
// -----------------------------------------------------------------------------
void OResultSet::setBoundedColumns(const OValueRefRow& _rRow,
                                   const OValueRefRow& _rSelectRow,
                                   const ::rtl::Reference<connectivity::OSQLColumns>& _rxColumns,
                                   const Reference<XIndexAccess>& _xNames,
                                   sal_Bool _bSetColumnMapping,
                                   const Reference<XDatabaseMetaData>& _xMetaData,
                                   ::std::vector<sal_Int32>& _rColMapping)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::setBoundedColumns" );
    ::comphelper::UStringMixEqual aCase(_xMetaData->supportsMixedCaseQuotedIdentifiers());

    Reference<XPropertySet> xTableColumn;
    ::rtl::OUString sTableColumnName, sSelectColumnRealName;

    const ::rtl::OUString sName     = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    const ::rtl::OUString sRealName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME);
    const ::rtl::OUString sType     = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE);

    typedef ::std::map<OSQLColumns::Vector::iterator,sal_Bool> IterMap;
    IterMap aSelectIters;
    OValueRefVector::Vector::iterator aRowIter = _rRow->get().begin()+1;
    for (sal_Int32 i=0; // the first column is the bookmark column
         aRowIter != _rRow->get().end();
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
            for (   OSQLColumns::Vector::iterator aIter = _rxColumns->get().begin();
                    aIter != _rxColumns->get().end();
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
                        sal_Int32 nSelectColumnPos = aIter - _rxColumns->get().begin() + 1;
                            // the getXXX methods are 1-based ...
                        sal_Int32 nTableColumnPos = i + 1;
                            // get first table column is the bookmark column ...
                        _rColMapping[nSelectColumnPos] = nTableColumnPos;
                        (_rSelectRow->get())[nSelectColumnPos] = *aRowIter;
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
            OSL_FAIL("OResultSet::setBoundedColumns: caught an Exception!");
        }
    }
    // in this case we got more select columns as columns exist in the table
    if ( _bSetColumnMapping && aSelectIters.size() != _rColMapping.size() )
    {
        Reference<XNameAccess> xNameAccess(_xNames,UNO_QUERY);
        Sequence< ::rtl::OUString > aSelectColumns = xNameAccess->getElementNames();

        for (   OSQLColumns::Vector::iterator aIter = _rxColumns->get().begin();
                aIter != _rxColumns->get().end();
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
                    sal_Int32 nSelectColumnPos = aIter - _rxColumns->get().begin() + 1;
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
                            (_rSelectRow->get())[nSelectColumnPos] = (_rRow->get())[nTableColumnPos];
                            break;
                        }
                    }
                }
            }
        }
    }
}
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
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getPropertySetInfo" );
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
void OResultSet::doTableSpecials(const OSQLTable& _xTable)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::doTableSpecials" );
    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(_xTable,UNO_QUERY);
    if(xTunnel.is())
    {
        m_pTable = reinterpret_cast< OFileTable* >( xTunnel->getSomething(OFileTable::getUnoTunnelImplementationId()) );
        if(m_pTable)
            m_pTable->acquire();
    }
}
// -----------------------------------------------------------------------------
void OResultSet::clearInsertRow()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::clearInsertRow" );
    m_aRow->setDeleted(sal_False); // set to false here because this is the new row
    OValueRefVector::Vector::iterator aIter = m_aInsertRow->get().begin();
    const OValueRefVector::Vector::iterator aEnd = m_aInsertRow->get().end();
    for(sal_Int32 nPos = 0;aIter != aEnd;++aIter,++nPos)
    {
        ORowSetValueDecoratorRef& rValue = (*aIter);
        if ( rValue->isBound() )
        {
            (m_aRow->get())[nPos]->setValue( (*aIter)->getValue() );
        }
        rValue->setBound(nPos == 0);
        rValue->setModified(sal_False);
        rValue->setNull();
    }
}
// -----------------------------------------------------------------------------
void OResultSet::initializeRow(OValueRefRow& _rRow,sal_Int32 _nColumnCount)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::initializeRow" );
    if(!_rRow.is())
    {
        _rRow   = new OValueRefVector(_nColumnCount);
        (_rRow->get())[0]->setBound(sal_True);
        ::std::for_each(_rRow->get().begin()+1,_rRow->get().end(),TSetRefBound(sal_False));
    }
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::fillIndexValues(const Reference< XColumnsSupplier> &/*_xIndex*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::fillIndexValues" );
    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::move(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, sal_Bool _bRetrieveData)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::move" );
    return Move(_eCursorPosition,_nOffset,_bRetrieveData);
}
// -----------------------------------------------------------------------------
sal_Int32 OResultSet::getDriverPos() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::getDriverPos" );
    return (m_aRow->get())[0]->getValue();
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::deletedVisible() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::deletedVisible" );
    return m_bShowDeleted;
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::isRowDeleted() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::isRowDeleted" );
    return m_aRow->isDeleted();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::disposing( const EventObject& Source ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OResultSet::disposing" );
    Reference<XPropertySet> xProp = m_pTable;
    if(m_pTable && Source.Source == xProp)
    {
        m_pTable->release();
        m_pTable = NULL;
    }
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
