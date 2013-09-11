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

#include <comphelper/sequence.hxx>
#include "ado/ADatabaseMetaDataResultSet.hxx"
#include "ado/ADatabaseMetaDataResultSetMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/ProcedureResult.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <comphelper/property.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/seqstream.hxx>
#include "connectivity/dbexception.hxx"


#include <oledb.h>

using namespace dbtools;
using namespace connectivity::ado;
using namespace cppu;
using namespace ::comphelper;
//------------------------------------------------------------------------------
using namespace ::com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet(ADORecordset* _pRecordSet)
    :ODatabaseMetaDataResultSet_BASE(m_aMutex)
    ,OPropertySetHelper(ODatabaseMetaDataResultSet_BASE::rBHelper)
    ,m_pRecordSet(_pRecordSet)
    ,m_aStatement(NULL)
    ,m_xMetaData(NULL)
    ,m_nRowPos(0)
    ,m_bWasNull(sal_False)
    ,m_bEOF(sal_False)
    ,m_bOnFirstAfterOpen(sal_False)
{
    osl_atomic_increment( &m_refCount );
    m_aColMapping.push_back(-1);
    if(_pRecordSet)
    {
        m_pRecordSet->AddRef();
        VARIANT_BOOL bIsAtBOF;
        m_pRecordSet->get_BOF(&bIsAtBOF);
        m_bOnFirstAfterOpen = bIsAtBOF != VARIANT_TRUE;
    }
    else
        m_bOnFirstAfterOpen = sal_False;
    osl_atomic_decrement( &m_refCount );
    //  allocBuffer();
}

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::~ODatabaseMetaDataResultSet()
{
    if(m_pRecordSet)
        m_pRecordSet->Release();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_pRecordSet)
        m_pRecordSet->Close();
    m_aStatement    = NULL;
m_xMetaData.clear();
}
// -------------------------------------------------------------------------
Any SAL_CALL ODatabaseMetaDataResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : ODatabaseMetaDataResultSet_BASE::queryInterface(rType);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL ODatabaseMetaDataResultSet::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),ODatabaseMetaDataResultSet_BASE::getTypes());
}
// -----------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::checkRecordSet() throw(SQLException)
{
    if(!m_pRecordSet)
        throwFunctionSequenceException(*this);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::findColumn( const OUString& columnName ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );


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
#define BLOCK_SIZE 256
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    columnIndex = mapColumn(columnIndex);
    WpADOField aField = ADOS::getField(m_pRecordSet,columnIndex);
    if((aField.GetAttributes() & adFldLong) == adFldLong)
    {
        //Copy the data only upto the Actual Size of Field.
        sal_Int32 nSize = aField.GetActualSize();
        Sequence<sal_Int8> aData(nSize);
        long index = 0;
        while(index < nSize)
        {
            m_aValue = aField.GetChunk(BLOCK_SIZE);
            if(m_aValue.isNull())
                break;
            UCHAR chData;
            for(long index2 = 0;index2 < BLOCK_SIZE;++index2)
            {
                HRESULT hr = ::SafeArrayGetElement(m_aValue.parray,&index2,&chData);
                if(SUCCEEDED(hr))
                {
                    //Take BYTE by BYTE and advance Memory Location
                    aData.getArray()[index++] = chData;
                }
                else
                    break;
            }
        }
        return index ? Reference< ::com::sun::star::io::XInputStream >(new SequenceInputStream(aData)) : Reference< ::com::sun::star::io::XInputStream >();
    }
    // else we ask for a bytesequence
    aField.get_Value(m_aValue);
    if(m_aValue.isNull())
        return NULL;
    return new SequenceInputStream(m_aValue);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getCharacterStream", *this );
    return NULL;
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_aValueRange.empty()  && columnIndex == 11 && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end() )
    {
        getValue(2);
        if ( static_cast<sal_Int16>(m_aValue) != adCurrency )
            return sal_False;
    }
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL ODatabaseMetaDataResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    getValue(columnIndex);

    columnIndex = mapColumn(columnIndex);

    if(m_aValue.isNull())
        return 0;
    if ( !m_aValueRange.empty() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return (sal_Int8)(*m_aValueRangeIter).second[(sal_Int32)m_aValue];
    else if(m_aStrValueRange.size() && (m_aStrValueRangeIter = m_aStrValueRange.find(columnIndex)) != m_aStrValueRange.end())
        return (sal_Int8)(*m_aStrValueRangeIter).second[m_aValue];

    return m_aValue;
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL ODatabaseMetaDataResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

::com::sun::star::util::Date SAL_CALL ODatabaseMetaDataResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

double SAL_CALL ODatabaseMetaDataResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

float SAL_CALL ODatabaseMetaDataResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );


    getValue(columnIndex);

    columnIndex = mapColumn(columnIndex);
    if(m_aValue.isNull())
        return 0;

    if(m_aValueRange.size() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return (*m_aValueRangeIter).second[(sal_Int32)m_aValue];
    else if(m_aStrValueRange.size() && (m_aStrValueRangeIter = m_aStrValueRange.find(columnIndex)) != m_aStrValueRange.end())
        return (*m_aStrValueRangeIter).second[m_aValue];

    return m_aValue;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XResultSet::getRow", *this );
    return 0;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL ODatabaseMetaDataResultSet::getLong( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getLong", *this );
    return sal_Int64(0);
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL ODatabaseMetaDataResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    if(!m_xMetaData.is())
        m_xMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);

    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ODatabaseMetaDataResultSet::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getRow", *this );
    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL ODatabaseMetaDataResultSet::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getRow", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ODatabaseMetaDataResultSet::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getRow", *this );
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL ODatabaseMetaDataResultSet::getRef( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getRow", *this );
    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL ODatabaseMetaDataResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    columnIndex = mapColumn(columnIndex);
    return Any();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL ODatabaseMetaDataResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    getValue(columnIndex);

    columnIndex = mapColumn(columnIndex);
    if(m_aValue.isNull())
        return 0;

    if(m_aValueRange.size() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return (sal_Int16)(*m_aValueRangeIter).second[(sal_Int32)m_aValue];
    else if(m_aStrValueRange.size() && (m_aStrValueRangeIter = m_aStrValueRange.find(columnIndex)) != m_aStrValueRange.end())
        return (sal_Int16)(*m_aStrValueRangeIter).second[m_aValue];

    return m_aValue;
}
// -------------------------------------------------------------------------

OUString SAL_CALL ODatabaseMetaDataResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    getValue(columnIndex);


    columnIndex = mapColumn(columnIndex);
    if(m_aValue.isNull())
        return OUString();
    if(m_aIntValueRange.size() && (m_aIntValueRangeIter = m_aIntValueRange.find(columnIndex)) != m_aIntValueRange.end())
        return (*m_aIntValueRangeIter).second[m_aValue];

    return m_aValue;
}

// -------------------------------------------------------------------------


::com::sun::star::util::Time SAL_CALL ODatabaseMetaDataResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------


::com::sun::star::util::DateTime SAL_CALL ODatabaseMetaDataResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    VARIANT_BOOL bIsAtEOF;
    m_pRecordSet->get_EOF(&bIsAtEOF);
    return bIsAtEOF == VARIANT_TRUE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    return m_nRowPos == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    return sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    if(first())
        previous();
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    if(last())
        next();
    m_bEOF = sal_True;
}
// -------------------------------------------------------------------------

void SAL_CALL ODatabaseMetaDataResultSet::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::first(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(!m_pRecordSet)
        return sal_False;

    sal_Bool bRet = SUCCEEDED(m_pRecordSet->MoveFirst());
    if ( bRet )
        m_nRowPos = 1;
    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );


    return m_pRecordSet && SUCCEEDED(m_pRecordSet->MoveLast()) ? sal_True : sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(first())
    {
        OLEVariant aEmpty;
        aEmpty.setNoArg();
        sal_Bool bRet = SUCCEEDED(m_pRecordSet->Move(row,aEmpty));
        if(bRet)
            m_nRowPos = row;
        return bRet;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(!m_pRecordSet)
        return sal_False;

    OLEVariant aEmpty;
    aEmpty.setNoArg();
    sal_Bool bRet = SUCCEEDED(m_pRecordSet->Move(row,aEmpty));
    if(bRet)
        m_nRowPos += row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(!m_pRecordSet)
        return sal_False;

    sal_Bool bRet = SUCCEEDED(m_pRecordSet->MovePrevious());
    if(bRet)
        --m_nRowPos;
    return bRet;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseMetaDataResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    return m_aStatement.get();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    return (eRec & adRecDeleted) == adRecDeleted;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{   ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    return (eRec & adRecNew) == adRecNew;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    return (eRec & adRecModified) == adRecModified;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(!m_pRecordSet)
        return sal_True;

    VARIANT_BOOL bIsAtBOF;
    m_pRecordSet->get_BOF(&bIsAtBOF);
    return bIsAtBOF == VARIANT_TRUE;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(!m_pRecordSet)
        return sal_False;

    if(m_bOnFirstAfterOpen)
    {
        m_bOnFirstAfterOpen = sal_False;
        return sal_True;
    }
    else
        return SUCCEEDED(m_pRecordSet->MoveNext());
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    return m_aValue.isNull();
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    m_pRecordSet->Resync(adAffectCurrent,adResyncAllValues);
}
// -------------------------------------------------------------------------

void SAL_CALL ODatabaseMetaDataResultSet::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    m_pRecordSet->Cancel();
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Any SAL_CALL ODatabaseMetaDataResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getResultSetConcurrency() const
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return ResultSetConcurrency::READ_ONLY;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getResultSetType() const
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return ResultSetType::FORWARD_ONLY;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getFetchDirection() const
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return FetchDirection::FORWARD;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getFetchSize() const
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nValue=-1;
    if(m_pRecordSet)
        m_pRecordSet->get_CacheSize(&nValue);
    return nValue;
}
//------------------------------------------------------------------------------
OUString ODatabaseMetaDataResultSet::getCursorName() const
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return OUString();
}

//------------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setFetchDirection(sal_Int32 /*_par0*/)
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "ResultSet::FetchDirection", *this );
}
//------------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setFetchSize(sal_Int32 _par0)
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    if(m_pRecordSet)
        m_pRecordSet->put_CacheSize(_par0);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODatabaseMetaDataResultSet::createArrayHelper( ) const
{

    Sequence< com::sun::star::beans::Property > aProps(5);
    com::sun::star::beans::Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,          OUString);
    DECL_PROP0(FETCHDIRECTION,      sal_Int32);
    DECL_PROP0(FETCHSIZE,           sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,       sal_Int32);

    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & ODatabaseMetaDataResultSet::getInfoHelper()
{
    return *const_cast<ODatabaseMetaDataResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaDataResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
        case PROPERTY_ID_FETCHSIZE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        default:
            ;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& /*rValue*/
                                                 )
                                                 throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
            throw Exception();
        default:
            OSL_FAIL("setFastPropertyValue_NoBroadcast: Illegal handle value!");
    }
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::getFastPropertyValue(
                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
            rValue <<= getCursorName();
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= getResultSetConcurrency();
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= getResultSetType();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= getFetchDirection();
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= getFetchSize();
            break;
    }
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setProceduresMap()
{

    for(sal_Int32 i=1;i<4;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(5);
    m_aColMapping.push_back(7);
    m_aColMapping.push_back(8);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(4);

    TInt2IntMap aMap;
    aMap[DB_PT_UNKNOWN]     = ProcedureResult::UNKNOWN;
    aMap[DB_PT_PROCEDURE]   = ProcedureResult::NONE;
    aMap[DB_PT_FUNCTION]    = ProcedureResult::RETURN;
    m_aValueRange[4] = aMap;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setProceduresMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setCatalogsMap()
{
    m_aColMapping.push_back(1);

    m_xMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setSchemasMap()
{
    m_aColMapping.push_back(2);

    m_xMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setColumnPrivilegesMap()
{

    m_aColMapping.push_back(3);
    m_aColMapping.push_back(4);
    m_aColMapping.push_back(5);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(2);
    m_aColMapping.push_back(9);
    m_aColMapping.push_back(10);

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setColumnPrivilegesMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setColumnsMap()
{

    for(sal_Int32 i=1;i<5;++i)
        m_aColMapping.push_back(i);

    m_aColMapping.push_back(12);
    m_aColMapping.push_back(12); // is used as TYPE_NAME

    m_aColMapping.push_back(14);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(17);
    m_aColMapping.push_back(18);

    m_aColMapping.push_back(11);
    m_aColMapping.push_back(29);
    m_aColMapping.push_back(9);
    m_aColMapping.push_back(18);
    m_aColMapping.push_back(18);

    m_aColMapping.push_back(15);
    m_aColMapping.push_back(7);
    m_aColMapping.push_back(11);

    TInt2IntMap aMap;
    aMap[adEmpty]           = ADOS::MapADOType2Jdbc(adEmpty);
    aMap[adTinyInt]         = ADOS::MapADOType2Jdbc(adTinyInt);
    aMap[adSmallInt]        = ADOS::MapADOType2Jdbc(adSmallInt);
    aMap[adInteger]         = ADOS::MapADOType2Jdbc(adInteger);
    aMap[adBigInt]          = ADOS::MapADOType2Jdbc(adBigInt);
    aMap[adUnsignedTinyInt] = ADOS::MapADOType2Jdbc(adUnsignedTinyInt);
    aMap[adUnsignedSmallInt]= ADOS::MapADOType2Jdbc(adUnsignedSmallInt);
    aMap[adUnsignedInt]     = ADOS::MapADOType2Jdbc(adUnsignedInt);
    aMap[adUnsignedBigInt]  = ADOS::MapADOType2Jdbc(adUnsignedBigInt);
    aMap[adSingle]          = ADOS::MapADOType2Jdbc(adSingle);
    aMap[adDouble]          = ADOS::MapADOType2Jdbc(adDouble);
    aMap[adCurrency]        = ADOS::MapADOType2Jdbc(adCurrency);
    aMap[adDecimal]         = ADOS::MapADOType2Jdbc(adDecimal);
    aMap[adNumeric]         = ADOS::MapADOType2Jdbc(adNumeric);
    aMap[adBoolean]         = ADOS::MapADOType2Jdbc(adBoolean);
    aMap[adError]           = ADOS::MapADOType2Jdbc(adError);
    aMap[adUserDefined]     = ADOS::MapADOType2Jdbc(adUserDefined);
    aMap[adVariant]         = ADOS::MapADOType2Jdbc(adVariant);
    aMap[adIDispatch]       = ADOS::MapADOType2Jdbc(adIDispatch);
    aMap[adIUnknown]        = ADOS::MapADOType2Jdbc(adIUnknown);
    aMap[adGUID]            = ADOS::MapADOType2Jdbc(adGUID);
    aMap[adDate]            = ADOS::MapADOType2Jdbc(adDate);
    aMap[adDBDate]          = ADOS::MapADOType2Jdbc(adDBDate);
    aMap[adDBTime]          = ADOS::MapADOType2Jdbc(adDBTime);
    aMap[adDBTimeStamp]     = ADOS::MapADOType2Jdbc(adDBTimeStamp);
    aMap[adBSTR]            = ADOS::MapADOType2Jdbc(adBSTR);
    aMap[adChar]            = ADOS::MapADOType2Jdbc(adChar);
    aMap[adVarChar]         = ADOS::MapADOType2Jdbc(adVarChar);
    aMap[adLongVarChar]     = ADOS::MapADOType2Jdbc(adLongVarChar);
    aMap[adWChar]           = ADOS::MapADOType2Jdbc(adWChar);
    aMap[adVarWChar]        = ADOS::MapADOType2Jdbc(adVarWChar);
    aMap[adLongVarWChar]    = ADOS::MapADOType2Jdbc(adLongVarWChar);
    aMap[adBinary]          = ADOS::MapADOType2Jdbc(adBinary);
    aMap[adVarBinary]       = ADOS::MapADOType2Jdbc(adVarBinary);
    aMap[adLongVarBinary]   = ADOS::MapADOType2Jdbc(adLongVarBinary);
    aMap[adChapter]         = ADOS::MapADOType2Jdbc(adChapter);
    aMap[adFileTime]        = ADOS::MapADOType2Jdbc(adFileTime);
    aMap[adPropVariant]     = ADOS::MapADOType2Jdbc(adPropVariant);
    aMap[adVarNumeric]      = ADOS::MapADOType2Jdbc(adVarNumeric);

    m_aValueRange[12] = aMap;

    ::std::map< sal_Int32,OUString> aMap2;
    aMap2[0] = OUString("YES");
    aMap2[1] = OUString("NO");
    m_aIntValueRange[18] = aMap2;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setColumnsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setTablesMap()
{

    for(sal_Int32 i=1;i<5;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(6);

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setTablesMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setProcedureColumnsMap()
{

    for(sal_Int32 i=1;i<5;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(10);
    m_aColMapping.push_back(16);
    m_aColMapping.push_back(13);
    m_aColMapping.push_back(11);
    m_aColMapping.push_back(12);

    m_aColMapping.push_back(9);
    m_aColMapping.push_back(14);

    TInt2IntMap aMap;
    aMap[DBTYPE_EMPTY] = DataType::SQLNULL;
    aMap[DBTYPE_NULL] = DataType::SQLNULL;
    aMap[DBTYPE_I2] = DataType::SMALLINT;
    aMap[DBTYPE_I4] = DataType::INTEGER;
    aMap[DBTYPE_R4] = DataType::FLOAT;
    aMap[DBTYPE_R8] = DataType::DOUBLE;
    aMap[DBTYPE_CY] = DataType::BIGINT;
    aMap[DBTYPE_DATE] = DataType::DATE;
    aMap[DBTYPE_BSTR] = DataType::VARCHAR;
    aMap[DBTYPE_IDISPATCH] = DataType::OBJECT;
    aMap[DBTYPE_ERROR] = DataType::OTHER;
    aMap[DBTYPE_BOOL] = DataType::BIT;
    aMap[DBTYPE_VARIANT] = DataType::STRUCT;
    aMap[DBTYPE_IUNKNOWN] = DataType::OTHER;
    aMap[DBTYPE_DECIMAL] = DataType::DECIMAL;
    aMap[DBTYPE_UI1] = DataType::TINYINT;
    aMap[DBTYPE_ARRAY] = DataType::ARRAY;
    aMap[DBTYPE_BYREF] = DataType::REF;
    aMap[DBTYPE_I1] = DataType::CHAR;
    aMap[DBTYPE_UI2] = DataType::SMALLINT;
    aMap[DBTYPE_UI4] = DataType::INTEGER;

    // aMap[The] = ;
    // aMap[in] = ;
    aMap[DBTYPE_I8] = DataType::BIGINT;
    aMap[DBTYPE_UI8] = DataType::BIGINT;
    aMap[DBTYPE_GUID] = DataType::OTHER;
    aMap[DBTYPE_VECTOR] = DataType::OTHER;
    aMap[DBTYPE_FILETIME] = DataType::OTHER;
    aMap[DBTYPE_RESERVED] = DataType::OTHER;

    // aMap[The] = ;
    aMap[DBTYPE_BYTES] = DataType::VARBINARY;
    aMap[DBTYPE_STR] = DataType::LONGVARCHAR;
    aMap[DBTYPE_WSTR] = DataType::LONGVARCHAR;
    aMap[DBTYPE_NUMERIC] = DataType::NUMERIC;
    aMap[DBTYPE_UDT] = DataType::OTHER;
    aMap[DBTYPE_DBDATE] = DataType::DATE;
    aMap[DBTYPE_DBTIME] = DataType::TIME;
    aMap[DBTYPE_DBTIMESTAMP] = DataType::TIMESTAMP;
    aMap[DBTYPE_HCHAPTER] = DataType::OTHER;
    aMap[DBTYPE_PROPVARIANT] = DataType::OTHER;
    aMap[DBTYPE_VARNUMERIC] = DataType::NUMERIC;

    m_aValueRange[10] = aMap;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setProcedureColumnsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setPrimaryKeysMap()
{

    sal_Int32 i=1;
    for(;i<5;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(7);
    m_aColMapping.push_back(8);

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setProcedureColumnsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setIndexInfoMap()
{

    sal_Int32 i=1;
    for(;i<4;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(8);
    m_aColMapping.push_back(4);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(10);
    m_aColMapping.push_back(17);
    m_aColMapping.push_back(18);
    m_aColMapping.push_back(21);
    m_aColMapping.push_back(22);
    m_aColMapping.push_back(23);
    m_aColMapping.push_back(24);

    TInt2IntMap aMap;
    aMap[DBPROPVAL_IT_HASH] = IndexType::HASHED;
    aMap[DBPROPVAL_IT_CONTENT] = IndexType::OTHER;
    aMap[DBPROPVAL_IT_OTHER] = IndexType::OTHER;
    aMap[DBPROPVAL_IT_BTREE] = IndexType::OTHER;

    m_aValueRange[10] = aMap;

    TInt2IntMap aMap2;
    aMap[0] = 1;
    aMap[1] = 0;
    m_aValueRange[8] = aMap2;

    ::std::map< sal_Int32,OUString> aMap3;
    aMap3[0]                    = OUString();
    aMap3[DB_COLLATION_ASC]     = OUString("A");
    aMap3[DB_COLLATION_DESC]    = OUString("D");

    m_aIntValueRange[21] = aMap3;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setIndexInfoMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setTablePrivilegesMap()
{

    sal_Int32 i=3;
    for(;i<6;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(1);
    m_aColMapping.push_back(2);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(7);

    ::std::map< sal_Int32,OUString> aMap;
    aMap[0] = OUString("YES");
    aMap[1] = OUString("NO");
    m_aIntValueRange[7] = aMap;


    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setTablePrivilegesMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setCrossReferenceMap()
{

    sal_Int32 i=1;
    for(;i<5;i++)
        m_aColMapping.push_back(i);
    for(i=7;i<11;i++)
        m_aColMapping.push_back(i);

    m_aColMapping.push_back(13);
    m_aColMapping.push_back(14);
    m_aColMapping.push_back(15);
    m_aColMapping.push_back(17);
    m_aColMapping.push_back(16);
    m_aColMapping.push_back(18);

    ::std::map< OUString,sal_Int32> aMap;
    aMap[ OUString("CASCADE")] = KeyRule::CASCADE;
    aMap[ OUString("RESTRICT")] = KeyRule::RESTRICT;
    aMap[ OUString("SET NULL")] = KeyRule::SET_NULL;
    aMap[ OUString("SET DEFAULT")] = KeyRule::SET_DEFAULT;
    aMap[ OUString("NO ACTION")] = KeyRule::NO_ACTION;

    m_aStrValueRange[14] = aMap;
    m_aStrValueRange[15] = aMap;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setCrossReferenceMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setTypeInfoMap(sal_Bool _bJetEngine)
{
    sal_Int32 i=1;
    for(;i<19;i++)
        m_aColMapping.push_back(i);

    ::std::map< OUString,sal_Int32> aMap1;
    aMap1[ OUString()] = 10;

    m_aStrValueRange[18] = aMap1;

    TInt2IntMap aMap;
    aMap[adEmpty]           = ADOS::MapADOType2Jdbc(adEmpty);
    aMap[adTinyInt]         = ADOS::MapADOType2Jdbc(adTinyInt);
    aMap[adSmallInt]        = ADOS::MapADOType2Jdbc(adSmallInt);
    aMap[adInteger]         = ADOS::MapADOType2Jdbc(adInteger);
    aMap[adBigInt]          = ADOS::MapADOType2Jdbc(adBigInt);
    aMap[adUnsignedTinyInt] = ADOS::MapADOType2Jdbc(adUnsignedTinyInt);
    aMap[adUnsignedSmallInt]= ADOS::MapADOType2Jdbc(adUnsignedSmallInt);
    aMap[adUnsignedInt]     = ADOS::MapADOType2Jdbc(adUnsignedInt);
    aMap[adUnsignedBigInt]  = ADOS::MapADOType2Jdbc(adUnsignedBigInt);
    aMap[adSingle]          = ADOS::MapADOType2Jdbc(adSingle);
    aMap[adDouble]          = ADOS::MapADOType2Jdbc(adDouble);
    aMap[adCurrency]        = ADOS::MapADOType2Jdbc(adCurrency);
    aMap[adDecimal]         = ADOS::MapADOType2Jdbc(adDecimal);
    aMap[adNumeric]         = ADOS::MapADOType2Jdbc(adNumeric);
    aMap[adBoolean]         = ADOS::MapADOType2Jdbc(adBoolean);
    aMap[adError]           = ADOS::MapADOType2Jdbc(adError);
    aMap[adUserDefined]     = ADOS::MapADOType2Jdbc(adUserDefined);
    aMap[adVariant]         = ADOS::MapADOType2Jdbc(adVariant);
    aMap[adIDispatch]       = ADOS::MapADOType2Jdbc(adIDispatch);
    aMap[adIUnknown]        = ADOS::MapADOType2Jdbc(adIUnknown);
    aMap[adGUID]            = ADOS::MapADOType2Jdbc(adGUID);
    aMap[adDate]            = _bJetEngine ? ADOS::MapADOType2Jdbc(adDBTimeStamp) : ADOS::MapADOType2Jdbc(adDate);
    aMap[adDBDate]          = ADOS::MapADOType2Jdbc(adDBDate);
    aMap[adDBTime]          = ADOS::MapADOType2Jdbc(adDBTime);
    aMap[adDBTimeStamp]     = ADOS::MapADOType2Jdbc(adDBTimeStamp);
    aMap[adBSTR]            = ADOS::MapADOType2Jdbc(adBSTR);
    aMap[adChar]            = ADOS::MapADOType2Jdbc(adChar);
    aMap[adVarChar]         = ADOS::MapADOType2Jdbc(adVarChar);
    aMap[adLongVarChar]     = ADOS::MapADOType2Jdbc(adLongVarChar);
    aMap[adWChar]           = ADOS::MapADOType2Jdbc(adWChar);
    aMap[adVarWChar]        = ADOS::MapADOType2Jdbc(adVarWChar);
    aMap[adLongVarWChar]    = ADOS::MapADOType2Jdbc(adLongVarWChar);
    aMap[adBinary]          = ADOS::MapADOType2Jdbc(adBinary);
    aMap[adVarBinary]       = ADOS::MapADOType2Jdbc(adVarBinary);
    aMap[adLongVarBinary]   = ADOS::MapADOType2Jdbc(adLongVarBinary);
    aMap[adChapter]         = ADOS::MapADOType2Jdbc(adChapter);
    aMap[adFileTime]        = ADOS::MapADOType2Jdbc(adFileTime);
    aMap[adPropVariant]     = ADOS::MapADOType2Jdbc(adPropVariant);
    aMap[adVarNumeric]      = ADOS::MapADOType2Jdbc(adVarNumeric);
//  aMap[adArray]           = ADOS::MapADOType2Jdbc(adArray);

    m_aValueRange[2] = aMap;

    TInt2IntMap aColumnValueMapping;
    aColumnValueMapping[VARIANT_FALSE]      = ColumnValue::NO_NULLS;
    aColumnValueMapping[VARIANT_TRUE]       = ColumnValue::NULLABLE;
    m_aValueRange[7] = aColumnValueMapping;

    // now adjust the column mapping
    // OJ 24.01.2002  96860
    TInt2IntMap aSerachMapping;
    aSerachMapping[DB_UNSEARCHABLE]     = ColumnSearch::NONE;
    aSerachMapping[DB_LIKE_ONLY]        = ColumnSearch::CHAR;
    aSerachMapping[DB_ALL_EXCEPT_LIKE]  = ColumnSearch::BASIC;
    aSerachMapping[DB_SEARCHABLE]       = ColumnSearch::FULL;

    m_aValueRange[9] = aSerachMapping;

    TInt2IntMap aCurrencyMapping;
    m_aValueRange[11] = aCurrencyMapping;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setTypeInfoMap();
    m_xMetaData = pMetaData;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::acquire() throw()
{
    ODatabaseMetaDataResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::release() throw()
{
    ODatabaseMetaDataResultSet_BASE::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL ODatabaseMetaDataResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
OLEVariant ODatabaseMetaDataResultSet::getValue(sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();

    WpADOField aField = ADOS::getField(m_pRecordSet,columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
