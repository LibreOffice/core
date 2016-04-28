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


#ifdef __GNUC__
#include <iostream>
#endif
#include <connectivity/sdbcx/VColumn.hxx>
#include "file/FResultSet.hxx"
#include "sqlbison.hxx"
#include "file/FResultSetMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/property.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/propshlp.hxx>
#include <iterator>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>

#include <algorithm>
#include <comphelper/extract.hxx>
#include <connectivity/dbexception.hxx>
#include <comphelper/types.hxx>
#include "resource/file_res.hrc"
#include "resource/sharedresources.hxx"


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

namespace
{
    void lcl_throwError(sal_uInt16 _nErrorId,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xContext)
    {
        ::connectivity::SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(_nErrorId);
        ::dbtools::throwGenericSQLException(sMessage ,_xContext);
    }
}

IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.drivers.file.ResultSet","com.sun.star.sdbc.ResultSet");

OResultSet::OResultSet(OStatement_Base* pStmt,OSQLParseTreeIterator&    _aSQLIterator) :    OResultSet_BASE(m_aMutex)
                        ,::comphelper::OPropertyContainer(OResultSet_BASE::rBHelper)
                        ,m_aAssignValues(nullptr)
                        ,m_aSkipDeletedSet(this)
                        ,m_pFileSet(nullptr)
                        ,m_pSortIndex(nullptr)
                        ,m_pTable(nullptr)
                        ,m_pParseTree(pStmt->getParseTree())
                        ,m_pSQLAnalyzer(nullptr)
                        ,m_aSQLIterator(_aSQLIterator)
                        ,m_nFetchSize(0)
                        ,m_nResultSetType(ResultSetType::SCROLL_INSENSITIVE)
                        ,m_nFetchDirection(FetchDirection::FORWARD)
                        ,m_nResultSetConcurrency(ResultSetConcurrency::UPDATABLE)
                        ,m_xStatement(*pStmt)
                        ,m_xMetaData(nullptr)
                        ,m_nRowPos(-1)
                        ,m_nFilePos(0)
                        ,m_nLastVisitedPos(-1)
                        ,m_nRowCountResult(-1)
                        ,m_nColumnCount(0)
                        ,m_bWasNull(false)
                        ,m_bEOF(false)
                        ,m_bInserted(false)
                        ,m_bRowUpdated(false)
                        ,m_bRowInserted(false)
                        ,m_bRowDeleted(false)
                        ,m_bShowDeleted(pStmt->getOwnConnection()->showDeleted())
                        ,m_bIsCount(false)
{
    osl_atomic_increment( &m_refCount );
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
    osl_atomic_decrement( &m_refCount );
}


OResultSet::~OResultSet()
{
    osl_atomic_increment( &m_refCount );
    disposing();
}

void OResultSet::construct()
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),           PROPERTY_ID_FETCHSIZE,          0,&m_nFetchSize,        ::cppu::UnoType<sal_Int32>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),        PROPERTY_ID_RESULTSETTYPE,      PropertyAttribute::READONLY,&m_nResultSetType,       ::cppu::UnoType<sal_Int32>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),      PROPERTY_ID_FETCHDIRECTION,     0,&m_nFetchDirection,   ::cppu::UnoType<sal_Int32>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY), PROPERTY_ID_RESULTSETCONCURRENCY,PropertyAttribute::READONLY,&m_nResultSetConcurrency,                ::cppu::UnoType<sal_Int32>::get());
}

void OResultSet::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    m_xStatement.clear();
    m_xMetaData.clear();
    m_pParseTree    = nullptr;
    m_xColNames.clear();
    m_xColumns = nullptr;
    m_xParamColumns = nullptr;
    m_xColsIdx.clear();

    Reference<XComponent> xComp = m_pTable;
    if ( xComp.is() )
        xComp->removeEventListener(this);
    if(m_pTable)
    {
        m_pTable->release();
        m_pTable = nullptr;
    }

    m_pFileSet = nullptr;
    DELETEZ(m_pSortIndex);

    if(m_aInsertRow.is())
        m_aInsertRow->get().clear();

    m_aSkipDeletedSet.clear();
}

Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet_BASE::queryInterface(rType);
}

Sequence< Type > SAL_CALL OResultSet::getTypes(  ) throw(RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OTypeCollection aTypes( cppu::UnoType<com::sun::star::beans::XMultiPropertySet>::get(),
                            cppu::UnoType<com::sun::star::beans::XPropertySet>::get(),
                            cppu::UnoType<com::sun::star::beans::XPropertySet>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}


sal_Int32 SAL_CALL OResultSet::findColumn( const OUString& columnName ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
    {
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
            return i;
    }

    ::dbtools::throwInvalidColumnException( columnName, *this );
    assert(false);
    return 0; // Never reached
}

const ORowSetValue& OResultSet::getValue(sal_Int32 columnIndex)
    throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkIndex(columnIndex );


    m_bWasNull = (m_aSelectRow->get())[columnIndex]->getValue().isNull();
    return *(m_aSelectRow->get())[columnIndex];
}

void OResultSet::checkIndex(sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException)
{
    if (   columnIndex <= 0
            || columnIndex >= m_nColumnCount )
        ::dbtools::throwInvalidIndexException(*this);
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}


sal_Bool SAL_CALL OResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


::com::sun::star::util::Date SAL_CALL OResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


double SAL_CALL OResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


float SAL_CALL OResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


sal_Int32 SAL_CALL OResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


sal_Int32 SAL_CALL OResultSet::getRow(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_ENSURE((m_bShowDeleted || !m_aRow->isDeleted()),"getRow called for deleted row");

    return m_aSkipDeletedSet.getMappedPosition((m_aRow->get())[0]->getValue());
}


sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_xColumns,m_aSQLIterator.getTables().begin()->first,m_pTable);
    return m_xMetaData;
}

Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}


Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}

Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}


Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}


Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex).makeAny();
}


sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}

OUString SAL_CALL OResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}

::com::sun::star::util::Time SAL_CALL OResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}

::com::sun::star::util::DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


sal_Bool SAL_CALL OResultSet::isAfterLast(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == sal_Int32(m_pFileSet->get().size());
}

sal_Bool SAL_CALL OResultSet::isFirst(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == 0;
}

sal_Bool SAL_CALL OResultSet::isLast(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == sal_Int32(m_pFileSet->get().size() - 1);
}

void SAL_CALL OResultSet::beforeFirst(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(first())
        previous();
}

void SAL_CALL OResultSet::afterLast(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(last())
        next();
    m_bEOF = true;
}


void SAL_CALL OResultSet::close(  ) throw(SQLException, RuntimeException, std::exception)
{
    dispose();
}


sal_Bool SAL_CALL OResultSet::first(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable && m_aSkipDeletedSet.skipDeleted(IResultSetHelper::FIRST,1,true);
}


sal_Bool SAL_CALL OResultSet::last(  ) throw(SQLException, RuntimeException, std::exception)
{
    // here I know definitely that I stand on the last record
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable && m_aSkipDeletedSet.skipDeleted(IResultSetHelper::LAST,1,true);
}

sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable && m_aSkipDeletedSet.skipDeleted(IResultSetHelper::ABSOLUTE1,row,true);
}

sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable && m_aSkipDeletedSet.skipDeleted(IResultSetHelper::RELATIVE1,row,true);
}

sal_Bool SAL_CALL OResultSet::previous(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_pTable && m_aSkipDeletedSet.skipDeleted(IResultSetHelper::PRIOR,0,true);
}

Reference< XInterface > SAL_CALL OResultSet::getStatement(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_xStatement;
}


sal_Bool SAL_CALL OResultSet::rowDeleted(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bRowDeleted;
}

sal_Bool SAL_CALL OResultSet::rowInserted(  ) throw(SQLException, RuntimeException, std::exception)
{   ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bRowInserted;
}

sal_Bool SAL_CALL OResultSet::rowUpdated(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bRowUpdated;
}


sal_Bool SAL_CALL OResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == -1;
}

sal_Bool SAL_CALL OResultSet::next(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_pTable && m_aSkipDeletedSet.skipDeleted(IResultSetHelper::NEXT,1,true);
}


sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}


void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException, std::exception)
{
}

void SAL_CALL OResultSet::clearWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
}

Any SAL_CALL OResultSet::getWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
    return Any();
}

void SAL_CALL OResultSet::insertRow(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_bInserted || !m_pTable)
        throwFunctionSequenceException(*this);

    // we know that we append new rows at the end
    // so we have to know where the end is
    (void)m_aSkipDeletedSet.skipDeleted(IResultSetHelper::LAST,1,false);
    m_bRowInserted = m_pTable->InsertRow(*m_aInsertRow, m_xColsIdx);
    if(m_bRowInserted && m_pFileSet.is())
    {
        sal_Int32 nPos = (m_aInsertRow->get())[0]->getValue();
        m_pFileSet->get().push_back(nPos);
        *(m_aInsertRow->get())[0] = sal_Int32(m_pFileSet->get().size());
        clearInsertRow();

        m_aSkipDeletedSet.insertNewPosition((m_aRow->get())[0]->getValue());
    }
}

void SAL_CALL OResultSet::updateRow(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!m_pTable || m_pTable->isReadOnly())
        lcl_throwError(STR_TABLE_READONLY,*this);

    m_bRowUpdated = m_pTable->UpdateRow(*m_aInsertRow, m_aRow,m_xColsIdx);
    *(m_aInsertRow->get())[0] = (sal_Int32)(m_aRow->get())[0]->getValue();

    clearInsertRow();
}

void SAL_CALL OResultSet::deleteRow() throw(SQLException, RuntimeException, std::exception)
{
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
        m_aRow->setDeleted(true);
        // don't touch the m_pFileSet member here
        m_aSkipDeletedSet.deletePosition(nPos);
    }
}

void SAL_CALL OResultSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bInserted     = false;
    m_bRowUpdated   = false;
    m_bRowInserted  = false;
    m_bRowDeleted   = false;

    if(m_aInsertRow.is())
    {
        OValueRefVector::Vector::iterator aIter = m_aInsertRow->get().begin()+1;
        for(;aIter != m_aInsertRow->get().end();++aIter)
        {
            (*aIter)->setBound(false);
            (*aIter)->setNull();
        }
    }
}


void SAL_CALL OResultSet::moveToInsertRow(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!m_pTable || m_pTable->isReadOnly())
        lcl_throwError(STR_TABLE_READONLY,*this);

    m_bInserted     = true;

    OValueRefVector::Vector::iterator aIter = m_aInsertRow->get().begin()+1;
    for(;aIter != m_aInsertRow->get().end();++aIter)
    {
        (*aIter)->setBound(false);
        (*aIter)->setNull();
    }
}


void SAL_CALL OResultSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException, std::exception)
{
}

void OResultSet::updateValue(sal_Int32 columnIndex ,const ORowSetValue& x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkIndex(columnIndex );
    columnIndex = mapColumn(columnIndex);

    (m_aInsertRow->get())[columnIndex]->setBound(true);
    *(m_aInsertRow->get())[columnIndex] = x;
}


void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    ORowSetValue aEmpty;
    updateValue(columnIndex,aEmpty);
}


void SAL_CALL OResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex, static_cast<bool>(x));
}

void SAL_CALL OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex,x);
}


void SAL_CALL OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex,x);
}

void SAL_CALL OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex,x);
}

void SAL_CALL OResultSet::updateLong( sal_Int32 /*columnIndex*/, sal_Int64 /*x*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRowUpdate::updateLong", *this );
}

void SAL_CALL OResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex,x);
}


void SAL_CALL OResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex,x);
}

void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const OUString& x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex,x);
}

void SAL_CALL OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex,x);
}

void SAL_CALL OResultSet::updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex,x);
}


void SAL_CALL OResultSet::updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex,x);
}


void SAL_CALL OResultSet::updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(SQLException, RuntimeException, std::exception)
{
    updateValue(columnIndex,x);
}


void SAL_CALL OResultSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    updateValue(columnIndex,aSeq);
}

void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException, std::exception)
{
    updateBinaryStream(columnIndex,x,length);
}

void SAL_CALL OResultSet::refreshRow(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException, std::exception)
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}


void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 /*scale*/ ) throw(SQLException, RuntimeException, std::exception)
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}

IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *getArrayHelper();
}


bool OResultSet::ExecuteRow(IResultSetHelper::Movement eFirstCursorPosition,
                               sal_Int32 nFirstOffset,
                               bool bEvaluate,
                               bool bRetrieveData)
{
    OSL_ENSURE(m_pSQLAnalyzer,"OResultSet::ExecuteRow: Analyzer isn't set!");

    // For further Fetch-Operations this information may possibly be changed ...
    IResultSetHelper::Movement eCursorPosition = eFirstCursorPosition;
    sal_Int32  nOffset = nFirstOffset;

    if (!m_pTable)
        return false;

    const OSQLColumns & rTableCols = *(m_pTable->getTableColumns());
    bool bHasRestriction = m_pSQLAnalyzer->hasRestriction();
again:

    // protect from reading over the end when somebody is inserting while we are reading
    // this method works only for dBase at the moment!!!
    if (eCursorPosition == IResultSetHelper::NEXT && m_nFilePos == m_nLastVisitedPos)
    {
        return false;
    }

    if (!m_pTable || !m_pTable->seekRow(eCursorPosition, nOffset, m_nFilePos))
    {
        return false;
    }

    if (!bEvaluate) // If no evaluation runs, then just fill the results-row
    {
        m_pTable->fetchRow(m_aRow,rTableCols, bRetrieveData);
    }
    else
    {
        m_pTable->fetchRow(m_aEvaluateRow, rTableCols, bRetrieveData || bHasRestriction);

        if  (   (   !m_bShowDeleted
                &&  m_aEvaluateRow->isDeleted()
                )
            ||  (   bHasRestriction
                &&  !m_pSQLAnalyzer->evaluateRestriction()
                )
            )
        {                                                // Evaluate the next record
            // delete current row in Keyset
            if (m_pFileSet.is())
            {
                OSL_ENSURE(eCursorPosition == IResultSetHelper::NEXT, "Falsche CursorPosition!");
                eCursorPosition = IResultSetHelper::NEXT;
                nOffset = 1;
            }
            else if (eCursorPosition == IResultSetHelper::FIRST ||
                     eCursorPosition == IResultSetHelper::NEXT ||
                     eCursorPosition == IResultSetHelper::ABSOLUTE1)
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
            else if (eCursorPosition == IResultSetHelper::RELATIVE1)
            {
                eCursorPosition = (nOffset >= 0) ? IResultSetHelper::NEXT : IResultSetHelper::PRIOR;
            }
            else
            {
                return false;
            }
            // Try again ...
            goto again;
        }
    }

    // Evaluate may only be set,
    // if the Keyset will be constructed further
    if  (   ( m_aSQLIterator.getStatementType() == OSQLStatementType::Select )
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
            sal_uInt32 nBookmarkValue = std::abs((sal_Int32)(m_aEvaluateRow->get())[0]->getValue());
            m_pFileSet->get().push_back(nBookmarkValue);
        }
    }
    else if (m_aSQLIterator.getStatementType() == OSQLStatementType::Update)
    {
        bool bOK = true;
        if (bEvaluate)
        {
            // read the actual result-row
            bOK = m_pTable->fetchRow(m_aEvaluateRow, *(m_pTable->getTableColumns()), true);
        }

        if (bOK)
        {
            // just give the values to be changed:
            if(!m_pTable->UpdateRow(*m_aAssignValues,m_aEvaluateRow, m_xColsIdx))
                return false;
        }
    }
    else if (m_aSQLIterator.getStatementType() == OSQLStatementType::Delete)
    {
        bool bOK = true;
        if (bEvaluate)
        {
            bOK = m_pTable->fetchRow(m_aEvaluateRow, *(m_pTable->getTableColumns()), true);
        }
        if (bOK)
        {
            if(!m_pTable->DeleteRow(*m_xColumns))
                return false;
        }
    }
    return true;
}


bool OResultSet::Move(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, bool bRetrieveData)
{
    sal_Int32 nTempPos = m_nRowPos;

    if (m_aSQLIterator.getStatementType() == OSQLStatementType::Select &&
        !isCount())
    {
        if (!m_pFileSet.is()) //no Index available
        {
            // Normal FETCH
            ExecuteRow(eCursorPosition,nOffset,false,bRetrieveData);

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
                case IResultSetHelper::RELATIVE1:
                    m_nRowPos += nOffset;
                    break;
                case IResultSetHelper::ABSOLUTE1:
                case IResultSetHelper::BOOKMARK:
                    if ( m_nRowPos == (nOffset -1) )
                        return true;
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
                    bool bOK = ExecuteRow(IResultSetHelper::BOOKMARK,(m_pFileSet->get())[m_nRowPos],false,bRetrieveData);
                    if (!bOK)
                        goto Error;

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
                    if (m_pFileSet->get().empty())
                    {
                        m_pTable->seekRow(IResultSetHelper::ABSOLUTE1, 0, m_nFilePos);
                    }
                    else
                    {
                        m_aFileSetIter = m_pFileSet->get().end()-1;
                        m_pTable->seekRow(IResultSetHelper::BOOKMARK, *m_aFileSetIter, m_nFilePos);
                    }
                    bool bOK = true;
                    // Determine the number of further Fetches
                    while (bOK && m_nRowPos >= (sal_Int32)m_pFileSet->get().size())
                    {
                        bOK = ExecuteRow(IResultSetHelper::NEXT,1,true, false);//bRetrieveData);
                    }

                    if (bOK)
                    {
                        // read the results again
                        m_pTable->fetchRow(m_aRow, *(m_pTable->getTableColumns()), bRetrieveData);

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
                        goto Error;
                    }
                }
            }
        }
    }
    else if (m_aSQLIterator.getStatementType() == OSQLStatementType::Select && isCount())
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
            case IResultSetHelper::RELATIVE1:
                m_nRowPos += nOffset;
                break;
            case IResultSetHelper::ABSOLUTE1:
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
                (m_aRow->get())[1]->setBound(true);
                (m_aSelectRow->get())[1] = (m_aRow->get())[1];
            }
        }
        else
        {
            m_bEOF = true;
            m_nRowPos = 1;
            return false;
        }
    }
    else
        // Fetch only possible at SELECT!
        return false;

    return true;

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
            case IResultSetHelper::ABSOLUTE1:
            case IResultSetHelper::RELATIVE1:
                if (nOffset > 0)
                    m_nRowPos = m_pFileSet.is() ? (sal_Int32)m_pFileSet->get().size() : -1;
                else if (nOffset < 0)
                    m_nRowPos = -1;
                break;
            case IResultSetHelper::BOOKMARK:
                m_nRowPos = nTempPos;    // last Position
        }
    }
    return false;
}

void OResultSet::sortRows()
{
    if (!m_pSQLAnalyzer->hasRestriction() && m_aOrderbyColumnNumber.size() == 1)
    {
        // is just one field given for sorting
        // and this field is indexed, then the Index will be used
        Reference<XIndexesSupplier> xIndexSup;
        m_pTable->queryInterface(cppu::UnoType<XIndexesSupplier>::get()) >>= xIndexSup;

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
    ::std::vector<sal_Int32>::const_iterator aOrderByIter = m_aOrderbyColumnNumber.begin();
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
                SAL_WARN( "connectivity.drivers","OFILECursor::Execute: Datentyp nicht implementiert");
                break;
        }
        (m_aSelectRow->get())[*aOrderByIter]->setBound(true);
    }

    m_pSortIndex = new OSortIndex(eKeyType,m_aOrderbyAscending);

    while ( ExecuteRow( IResultSetHelper::NEXT, 1, false ) )
    {
        m_aSelectRow->get()[0]->setValue( m_aRow->get()[0]->getValue() );
        if ( m_pSQLAnalyzer->hasFunctions() )
            m_pSQLAnalyzer->setSelectionEvaluationResult( m_aSelectRow, m_aColMapping );
        const sal_Int32 nBookmark = (*m_aRow->get().begin())->getValue();
        ExecuteRow( IResultSetHelper::BOOKMARK, nBookmark, true, false );
    }

    // create sorted Keyset
    m_pFileSet = nullptr;
    m_pFileSet = m_pSortIndex->CreateKeySet();
    DELETEZ(m_pSortIndex);
    // now access to a sorted set is possible via Index
}


bool OResultSet::OpenImpl()
{
    OSL_ENSURE(m_pSQLAnalyzer,"No analyzer set with setSqlAnalyzer!");
    if(!m_pTable)
    {
        const OSQLTables& rTabs = m_aSQLIterator.getTables();
        if (rTabs.empty() || !rTabs.begin()->second.is())
            lcl_throwError(STR_QUERY_TOO_COMPLEX,*this);

        if ( rTabs.size() > 1 || m_aSQLIterator.hasErrors() )
            lcl_throwError(STR_QUERY_MORE_TABLES,*this);

        OSQLTable xTable = rTabs.begin()->second;
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
    m_pFileSet = nullptr;

    // position at the beginning
    m_nRowPos = -1;
    m_nFilePos  = 0;
    m_nRowCountResult = -1;
    m_pTable->seekRow(IResultSetHelper::ABSOLUTE1, 0, m_nFilePos);

    m_nLastVisitedPos = m_pTable->getCurrentLastPos();

    switch(m_aSQLIterator.getStatementType())
    {
        case OSQLStatementType::Select:
        {
            if(isCount())
            {
                if(m_xColumns->get().size() > 1)
                    lcl_throwError(STR_QUERY_COMPLEX_COUNT,*this);

                m_nRowCountResult = 0;
                // for now simply iterate over all rows and
                // do all actions (or just count)
                {
                    bool bOK = true;
                    while (bOK)
                    {
                        bOK = ExecuteRow(IResultSetHelper::NEXT);

                        if (bOK)
                        {
                            m_nRowCountResult++;
                        }
                    }

                    // save result of COUNT(*) in m_nRowCountResult.
                    // nRowCount (number of Rows in the result) = 1 for this request!
                }
            }
            else
            {
                bool bDistinct = false;
                assert(m_pParseTree != nullptr);
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
                    for (size_t i = 1; // 0: bookmark (see setBoundedColumns)
                         i < m_aColMapping.size(); ++i)
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
                    bDistinct = true;
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
                            ExecuteRow(IResultSetHelper::BOOKMARK,nPos,false);
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
                            ExecuteRow(IResultSetHelper::BOOKMARK,nKey,false);
                            m_pSQLAnalyzer->setSelectionEvaluationResult(m_aSelectRow,m_aColMapping);
                            OValueRefVector::Vector::const_iterator loopInRow = m_aSelectRow->get().begin();
                            OValueVector::Vector::const_iterator existentInSearchRow = aSearchRow->get().begin();
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

        case OSQLStatementType::Update:
        case OSQLStatementType::Delete:
            // during processing count the number of processed Rows
            m_nRowCountResult = 0;
            // for now simply iterate over all rows and
            // run the actions (or simply count):
            {

                bool bOK = true;
                while (bOK)
                {
                    bOK = ExecuteRow(IResultSetHelper::NEXT);

                    if (bOK)
                    {
                        m_nRowCountResult++;
                    }
                }

                // save result of COUNT(*) in nRowCountResult.
                // nRowCount (number of rows in the result-set) = 1 for this request!
            }
            break;
        case OSQLStatementType::Insert:
            m_nRowCountResult = 0;

            OSL_ENSURE(m_aAssignValues.is(),"No assign values set!");
            if(!m_pTable->InsertRow(*m_aAssignValues, m_xColsIdx))
            {
                m_nFilePos  = 0;
                return false;
            }

            m_nRowCountResult = 1;
            break;
        default:
            SAL_WARN( "connectivity.drivers", "OResultSet::OpenImpl: unsupported statement type!" );
            break;
    }

    // reset FilePos
    m_nFilePos  = 0;

    return true;
}

Sequence< sal_Int8 > OResultSet::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = nullptr;
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

sal_Int64 OResultSet::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException, std::exception)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : 0;
}

void OResultSet::setBoundedColumns(const OValueRefRow& _rRow,
                                   const OValueRefRow& _rSelectRow,
                                   const ::rtl::Reference<connectivity::OSQLColumns>& _rxColumns,
                                   const Reference<XIndexAccess>& _xNames,
                                   bool _bSetColumnMapping,
                                   const Reference<XDatabaseMetaData>& _xMetaData,
                                   ::std::vector<sal_Int32>& _rColMapping)
{
    ::comphelper::UStringMixEqual aCase(_xMetaData->supportsMixedCaseQuotedIdentifiers());

    Reference<XPropertySet> xTableColumn;
    OUString sTableColumnName, sSelectColumnRealName;

    const OUString sName     = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    const OUString sRealName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME);
    const OUString sType     = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE);

    typedef ::std::map<OSQLColumns::Vector::iterator,sal_Bool> IterMap;
    IterMap aSelectIters;
    OValueRefVector::Vector::const_iterator aRowIter = _rRow->get().begin()+1;
    for (sal_Int32 i=0; // the first column is the bookmark column
         aRowIter != _rRow->get().end();
            ++i, ++aRowIter
        )
    {
        (*aRowIter)->setBound(false);
        try
        {
            // get the table column and its name
            _xNames->getByIndex(i) >>= xTableColumn;
            OSL_ENSURE(xTableColumn.is(), "OResultSet::setBoundedColumns: invalid table column!");
            if (xTableColumn.is())
                xTableColumn->getPropertyValue(sName) >>= sTableColumnName;
            else
                sTableColumnName.clear();

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
                    aSelectIters.insert(IterMap::value_type(aIter,true));
                    if(_bSetColumnMapping)
                    {
                        sal_Int32 nSelectColumnPos = aIter - _rxColumns->get().begin() + 1;
                            // the getXXX methods are 1-based ...
                        sal_Int32 nTableColumnPos = i + 1;
                            // get first table column is the bookmark column ...
                        _rColMapping[nSelectColumnPos] = nTableColumnPos;
                        (_rSelectRow->get())[nSelectColumnPos] = *aRowIter;
                    }

                    (*aRowIter)->setBound(true);
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
            SAL_WARN( "connectivity.drivers","OResultSet::setBoundedColumns: caught an Exception!");
        }
    }
    // in this case we got more select columns as columns exist in the table
    if ( _bSetColumnMapping && aSelectIters.size() != _rColMapping.size() )
    {
        Reference<XNameAccess> xNameAccess(_xNames,UNO_QUERY);
        Sequence< OUString > aSelectColumns = xNameAccess->getElementNames();

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
                    aSelectIters.insert(IterMap::value_type(aIter,true));
                    sal_Int32 nSelectColumnPos = aIter - _rxColumns->get().begin() + 1;
                    const OUString* pBegin = aSelectColumns.getConstArray();
                    const OUString* pEnd   = pBegin + aSelectColumns.getLength();
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

void SAL_CALL OResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}

void SAL_CALL OResultSet::release() throw()
{
    OResultSet_BASE::release();
}

Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

void OResultSet::doTableSpecials(const OSQLTable& _xTable)
{
    Reference<css::lang::XUnoTunnel> xTunnel(_xTable, UNO_QUERY_THROW);
    m_pTable = reinterpret_cast< OFileTable* >(xTunnel->getSomething(OFileTable::getUnoTunnelImplementationId()));
    assert(m_pTable);
    m_pTable->acquire();
}

void OResultSet::clearInsertRow()
{
    m_aRow->setDeleted(false); // set to false here because this is the new row
    OValueRefVector::Vector::iterator aIter = m_aInsertRow->get().begin();
    const OValueRefVector::Vector::const_iterator aEnd = m_aInsertRow->get().end();
    for(sal_Int32 nPos = 0;aIter != aEnd;++aIter,++nPos)
    {
        ORowSetValueDecoratorRef& rValue = (*aIter);
        if ( rValue->isBound() )
        {
            (m_aRow->get())[nPos]->setValue( (*aIter)->getValue() );
        }
        rValue->setBound(nPos == 0);
        rValue->setModified(false);
        rValue->setNull();
    }
}

void OResultSet::initializeRow(OValueRefRow& _rRow,sal_Int32 _nColumnCount)
{
    if(!_rRow.is())
    {
        _rRow   = new OValueRefVector(_nColumnCount);
        (_rRow->get())[0]->setBound(true);
        ::std::for_each(_rRow->get().begin()+1,_rRow->get().end(),TSetRefBound(false));
    }
}

bool OResultSet::fillIndexValues(const Reference< XColumnsSupplier> &/*_xIndex*/)
{
    return false;
}

bool OResultSet::move(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, bool _bRetrieveData)
{
    return Move(_eCursorPosition,_nOffset,_bRetrieveData);
}

sal_Int32 OResultSet::getDriverPos() const
{
    return (m_aRow->get())[0]->getValue();
}

bool OResultSet::isRowDeleted() const
{
    return m_aRow->isDeleted();
}

void SAL_CALL OResultSet::disposing( const EventObject& Source ) throw (RuntimeException, std::exception)
{
    Reference<XPropertySet> xProp = m_pTable;
    if(m_pTable && Source.Source == xProp)
    {
        m_pTable->release();
        m_pTable = nullptr;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
