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
#include <ado/ADatabaseMetaDataResultSet.hxx>
#include <ado/ADatabaseMetaDataResultSetMetaData.hxx>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
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
#include <connectivity/dbexception.hxx>
#include <rtl/ref.hxx>

#include <oledb.h>

using namespace dbtools;
using namespace connectivity::ado;
using namespace cppu;
using namespace ::comphelper;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;


ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet(ADORecordset* _pRecordSet)
    :ODatabaseMetaDataResultSet_BASE(m_aMutex)
    ,OPropertySetHelper(ODatabaseMetaDataResultSet_BASE::rBHelper)
    ,m_pRecordSet(_pRecordSet)
    ,m_nRowPos(0)
    ,m_bWasNull(false)
    ,m_bEOF(false)
    ,m_bOnFirstAfterOpen(false)
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
        m_bOnFirstAfterOpen = false;
    osl_atomic_decrement( &m_refCount );
    //  allocBuffer();
}


ODatabaseMetaDataResultSet::~ODatabaseMetaDataResultSet()
{
    if(m_pRecordSet)
        m_pRecordSet->Release();
}

void ODatabaseMetaDataResultSet::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_pRecordSet)
        m_pRecordSet->Close();
    m_aStatement.clear();
    m_xMetaData.clear();
}

Any SAL_CALL ODatabaseMetaDataResultSet::queryInterface( const Type & rType )
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : ODatabaseMetaDataResultSet_BASE::queryInterface(rType);
}

css::uno::Sequence< css::uno::Type > SAL_CALL ODatabaseMetaDataResultSet::getTypes(  )
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<css::beans::XMultiPropertySet>::get(),
                                    cppu::UnoType<css::beans::XFastPropertySet>::get(),
                                    cppu::UnoType<css::beans::XPropertySet>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),ODatabaseMetaDataResultSet_BASE::getTypes());
}

void ODatabaseMetaDataResultSet::checkRecordSet()
{
    if(!m_pRecordSet)
        throwFunctionSequenceException(*this);
}

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::findColumn( const OUString& columnName )
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
}

#define BLOCK_SIZE 256

Reference< css::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getBinaryStream( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    columnIndex = mapColumn(columnIndex);
    WpADOField aField = ADOS::getField(m_pRecordSet,columnIndex);
    if((aField.GetAttributes() & adFldLong) == adFldLong)
    {
        //Copy the data only up to the Actual Size of Field.
        sal_Int32 nSize = aField.GetActualSize();
        Sequence<sal_Int8> aData(nSize);
        sal_Int32 index = 0;
        while(index < nSize)
        {
            m_aValue = aField.GetChunk(BLOCK_SIZE);
            if(m_aValue.isNull())
                break;
            UCHAR chData;
            for(LONG index2 = 0;index2 < BLOCK_SIZE;++index2)
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
        return index ? Reference< css::io::XInputStream >(new SequenceInputStream(aData)) : Reference< css::io::XInputStream >();
    }
    // else we ask for a bytesequence
    aField.get_Value(m_aValue);
    if(m_aValue.isNull())
        return nullptr;
    return new SequenceInputStream(m_aValue.getByteSequence());
}

Reference< css::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getCharacterStream", *this );
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::getBoolean( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_aValueRange.empty()  && columnIndex == 11 && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end() )
    {
        getValue(2);
        if ( m_aValue.getInt16() != adCurrency )
            return false;
    }
    return getValue(columnIndex).getBool();
}


sal_Int8 SAL_CALL ODatabaseMetaDataResultSet::getByte( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    getValue(columnIndex);

    columnIndex = mapColumn(columnIndex);

    if(m_aValue.isNull())
        return 0;
    if ( !m_aValueRange.empty() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return static_cast<sal_Int8>((*m_aValueRangeIter).second[m_aValue.getInt32()]);
    else if(m_aStrValueRange.size() && (m_aStrValueRangeIter = m_aStrValueRange.find(columnIndex)) != m_aStrValueRange.end())
        return static_cast<sal_Int8>((*m_aStrValueRangeIter).second[m_aValue.getString()]);

    return m_aValue.getInt8();
}


Sequence< sal_Int8 > SAL_CALL ODatabaseMetaDataResultSet::getBytes( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getByteSequence();
}


css::util::Date SAL_CALL ODatabaseMetaDataResultSet::getDate( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getDate();
}


double SAL_CALL ODatabaseMetaDataResultSet::getDouble( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getDouble();
}


float SAL_CALL ODatabaseMetaDataResultSet::getFloat( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getFloat();
}


sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getInt( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );


    getValue(columnIndex);

    columnIndex = mapColumn(columnIndex);
    if(m_aValue.isNull())
        return 0;

    if(m_aValueRange.size() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return (*m_aValueRangeIter).second[m_aValue.getInt32()];
    else if(m_aStrValueRange.size() && (m_aStrValueRangeIter = m_aStrValueRange.find(columnIndex)) != m_aStrValueRange.end())
        return (*m_aStrValueRangeIter).second[m_aValue.getString()];

    return m_aValue.getInt32();
}

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getRow(  )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XResultSet::getRow", *this );
}

sal_Int64 SAL_CALL ODatabaseMetaDataResultSet::getLong( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getLong", *this );
}

Reference< XResultSetMetaData > SAL_CALL ODatabaseMetaDataResultSet::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    if(!m_xMetaData.is())
        m_xMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);

    return m_xMetaData;
}

Reference< XArray > SAL_CALL ODatabaseMetaDataResultSet::getArray( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getRow", *this );
}

Reference< XClob > SAL_CALL ODatabaseMetaDataResultSet::getClob( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getRow", *this );
}

Reference< XBlob > SAL_CALL ODatabaseMetaDataResultSet::getBlob( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getRow", *this );
}

Reference< XRef > SAL_CALL ODatabaseMetaDataResultSet::getRef( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getRow", *this );
}

Any SAL_CALL ODatabaseMetaDataResultSet::getObject( sal_Int32 columnIndex, const Reference< css::container::XNameAccess >& /*typeMap*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    columnIndex = mapColumn(columnIndex);
    return Any();
}


sal_Int16 SAL_CALL ODatabaseMetaDataResultSet::getShort( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    getValue(columnIndex);

    columnIndex = mapColumn(columnIndex);
    if(m_aValue.isNull())
        return 0;

    if(m_aValueRange.size() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return static_cast<sal_Int16>((*m_aValueRangeIter).second[m_aValue.getInt32()]);
    else if(m_aStrValueRange.size() && (m_aStrValueRangeIter = m_aStrValueRange.find(columnIndex)) != m_aStrValueRange.end())
        return static_cast<sal_Int16>((*m_aStrValueRangeIter).second[m_aValue.getString()]);

    return m_aValue.getInt16();
}


OUString SAL_CALL ODatabaseMetaDataResultSet::getString( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    getValue(columnIndex);


    columnIndex = mapColumn(columnIndex);
    if(m_aValue.isNull())
        return OUString();
    if(m_aIntValueRange.size() && (m_aIntValueRangeIter = m_aIntValueRange.find(columnIndex)) != m_aIntValueRange.end())
        return (*m_aIntValueRangeIter).second[m_aValue.getInt32()];

    return m_aValue.getString();
}


css::util::Time SAL_CALL ODatabaseMetaDataResultSet::getTime( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getTime();
}


css::util::DateTime SAL_CALL ODatabaseMetaDataResultSet::getTimestamp( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getDateTime();
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isAfterLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    VARIANT_BOOL bIsAtEOF;
    m_pRecordSet->get_EOF(&bIsAtEOF);
    return bIsAtEOF == VARIANT_TRUE;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    return m_nRowPos == 1;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    return true;
}

void SAL_CALL ODatabaseMetaDataResultSet::beforeFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    if(first())
        previous();
}

void SAL_CALL ODatabaseMetaDataResultSet::afterLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    if(last())
        next();
    m_bEOF = true;
}


void SAL_CALL ODatabaseMetaDataResultSet::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    }
    dispose();
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::first(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(!m_pRecordSet)
        return false;

    bool bRet = SUCCEEDED(m_pRecordSet->MoveFirst());
    if ( bRet )
        m_nRowPos = 1;
    return bRet;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::last(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );


    return m_pRecordSet && SUCCEEDED(m_pRecordSet->MoveLast());
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::absolute( sal_Int32 row )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(first())
    {
        OLEVariant aEmpty;
        aEmpty.setNoArg();
        bool bRet = SUCCEEDED(m_pRecordSet->Move(row,aEmpty));
        if(bRet)
            m_nRowPos = row;
        return bRet;
    }
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::relative( sal_Int32 row )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(!m_pRecordSet)
        return false;

    OLEVariant aEmpty;
    aEmpty.setNoArg();
    bool bRet = SUCCEEDED(m_pRecordSet->Move(row,aEmpty));
    if(bRet)
        m_nRowPos += row;
    return bRet;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::previous(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(!m_pRecordSet)
        return false;

    bool bRet = SUCCEEDED(m_pRecordSet->MovePrevious());
    if(bRet)
        --m_nRowPos;
    return bRet;
}

Reference< XInterface > SAL_CALL ODatabaseMetaDataResultSet::getStatement(  )
{
    return m_aStatement.get();
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowDeleted(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    sal_Int32 eRec;
    m_pRecordSet->get_Status(&eRec);
    return (RecordStatusEnum(eRec) & adRecDeleted) == adRecDeleted;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowInserted(  )
{   ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    sal_Int32 eRec;
    m_pRecordSet->get_Status(&eRec);
    return (RecordStatusEnum(eRec) & adRecNew) == adRecNew;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowUpdated(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    sal_Int32 eRec;
    m_pRecordSet->get_Status(&eRec);
    return (RecordStatusEnum(eRec) & adRecModified) == adRecModified;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isBeforeFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(!m_pRecordSet)
        return true;

    VARIANT_BOOL bIsAtBOF;
    m_pRecordSet->get_BOF(&bIsAtBOF);
    return bIsAtBOF == VARIANT_TRUE;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::next(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);


    if(!m_pRecordSet)
        return false;

    if(m_bOnFirstAfterOpen)
    {
        m_bOnFirstAfterOpen = false;
        return true;
    }
    else
        return SUCCEEDED(m_pRecordSet->MoveNext());
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::wasNull(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    return m_aValue.isNull();
}

void SAL_CALL ODatabaseMetaDataResultSet::refreshRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    m_pRecordSet->Resync(adAffectCurrent);
}


void SAL_CALL ODatabaseMetaDataResultSet::cancel(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();


    m_pRecordSet->Cancel();
}

void SAL_CALL ODatabaseMetaDataResultSet::clearWarnings(  )
{
}

Any SAL_CALL ODatabaseMetaDataResultSet::getWarnings(  )
{
    return Any();
}

sal_Int32 ODatabaseMetaDataResultSet::getResultSetConcurrency()
{
    return ResultSetConcurrency::READ_ONLY;
}

sal_Int32 ODatabaseMetaDataResultSet::getResultSetType()
{
    return ResultSetType::FORWARD_ONLY;
}

sal_Int32 ODatabaseMetaDataResultSet::getFetchDirection()
{
    return FetchDirection::FORWARD;
}

sal_Int32 ODatabaseMetaDataResultSet::getFetchSize() const
{
    sal_Int32 nValue=-1;
    if(m_pRecordSet)
        m_pRecordSet->get_CacheSize(&nValue);
    return nValue;
}

OUString ODatabaseMetaDataResultSet::getCursorName()
{
    return OUString();
}


void ODatabaseMetaDataResultSet::setFetchDirection(sal_Int32 /*_par0*/)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "ResultSet::FetchDirection", *this );
}

void ODatabaseMetaDataResultSet::setFetchSize(sal_Int32 _par0)
{
    if(m_pRecordSet)
        m_pRecordSet->put_CacheSize(_par0);
}

::cppu::IPropertyArrayHelper* ODatabaseMetaDataResultSet::createArrayHelper( ) const
{

    return new ::cppu::OPropertyArrayHelper
    {
        {
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),
                PROPERTY_ID_CURSORNAME,
                cppu::UnoType<OUString>::get(),
                0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
                PROPERTY_ID_FETCHDIRECTION,
                cppu::UnoType<sal_Int32>::get(),
                0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
                PROPERTY_ID_FETCHSIZE,
                cppu::UnoType<sal_Int32>::get(),
                0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
                PROPERTY_ID_RESULTSETCONCURRENCY,
                cppu::UnoType<sal_Int32>::get(),
                0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
                PROPERTY_ID_RESULTSETTYPE,
                cppu::UnoType<sal_Int32>::get(),
                0
            }
        }
    };
}

::cppu::IPropertyArrayHelper & ODatabaseMetaDataResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool ODatabaseMetaDataResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw css::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
        case PROPERTY_ID_FETCHSIZE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        default:
            ;
    }
    return false;
}

void ODatabaseMetaDataResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& /*rValue*/
                                                 )
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
            throw Exception("cannot set prop " + OUString::number(nHandle), nullptr);
        default:
            OSL_FAIL("setFastPropertyValue_NoBroadcast: Illegal handle value!");
    }
}

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

void ODatabaseMetaDataResultSet::setProceduresMap()
{
    for(sal_Int32 i=1;i<4;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(5);
    m_aColMapping.push_back(7);
    m_aColMapping.push_back(8);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(4);

    ::std::map<sal_Int32,sal_Int32> aMap
    {
        {DB_PT_UNKNOWN, ProcedureResult::UNKNOWN},
        {DB_PT_PROCEDURE, ProcedureResult::NONE},
        {DB_PT_FUNCTION, ProcedureResult::RETURN}
    };
    m_aValueRange[4] = aMap;

    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setProceduresMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setCatalogsMap()
{
    m_aColMapping.push_back(1);

    m_xMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
}

void ODatabaseMetaDataResultSet::setSchemasMap()
{
    m_aColMapping.push_back(2);

    m_xMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
}

void ODatabaseMetaDataResultSet::setColumnPrivilegesMap()
{

    m_aColMapping.push_back(3);
    m_aColMapping.push_back(4);
    m_aColMapping.push_back(5);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(2);
    m_aColMapping.push_back(9);
    m_aColMapping.push_back(10);

    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setColumnPrivilegesMap();
    m_xMetaData = pMetaData;
}

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

    ::std::map<sal_Int32,sal_Int32> aMap {
        {adEmpty,            ADOS::MapADOType2Jdbc(adEmpty)},
        {adTinyInt,          ADOS::MapADOType2Jdbc(adTinyInt)},
        {adSmallInt,         ADOS::MapADOType2Jdbc(adSmallInt)},
        {adInteger,          ADOS::MapADOType2Jdbc(adInteger)},
        {adBigInt,           ADOS::MapADOType2Jdbc(adBigInt)},
        {adUnsignedTinyInt,  ADOS::MapADOType2Jdbc(adUnsignedTinyInt)},
        {adUnsignedSmallInt, ADOS::MapADOType2Jdbc(adUnsignedSmallInt)},
        {adUnsignedInt,      ADOS::MapADOType2Jdbc(adUnsignedInt)},
        {adUnsignedBigInt,   ADOS::MapADOType2Jdbc(adUnsignedBigInt)},
        {adSingle,           ADOS::MapADOType2Jdbc(adSingle)},
        {adDouble,           ADOS::MapADOType2Jdbc(adDouble)},
        {adCurrency,         ADOS::MapADOType2Jdbc(adCurrency)},
        {adDecimal,          ADOS::MapADOType2Jdbc(adDecimal)},
        {adNumeric,          ADOS::MapADOType2Jdbc(adNumeric)},
        {adBoolean,          ADOS::MapADOType2Jdbc(adBoolean)},
        {adError,            ADOS::MapADOType2Jdbc(adError)},
        {adUserDefined,      ADOS::MapADOType2Jdbc(adUserDefined)},
        {adVariant,          ADOS::MapADOType2Jdbc(adVariant)},
        {adIDispatch,        ADOS::MapADOType2Jdbc(adIDispatch)},
        {adIUnknown,         ADOS::MapADOType2Jdbc(adIUnknown)},
        {adGUID,             ADOS::MapADOType2Jdbc(adGUID)},
        {adDate,             ADOS::MapADOType2Jdbc(adDate)},
        {adDBDate,           ADOS::MapADOType2Jdbc(adDBDate)},
        {adDBTime,           ADOS::MapADOType2Jdbc(adDBTime)},
        {adDBTimeStamp,      ADOS::MapADOType2Jdbc(adDBTimeStamp)},
        {adBSTR,             ADOS::MapADOType2Jdbc(adBSTR)},
        {adChar,             ADOS::MapADOType2Jdbc(adChar)},
        {adVarChar,          ADOS::MapADOType2Jdbc(adVarChar)},
        {adLongVarChar,      ADOS::MapADOType2Jdbc(adLongVarChar)},
        {adWChar,            ADOS::MapADOType2Jdbc(adWChar)},
        {adVarWChar,         ADOS::MapADOType2Jdbc(adVarWChar)},
        {adLongVarWChar,     ADOS::MapADOType2Jdbc(adLongVarWChar)},
        {adBinary,           ADOS::MapADOType2Jdbc(adBinary)},
        {adVarBinary,        ADOS::MapADOType2Jdbc(adVarBinary)},
        {adLongVarBinary,    ADOS::MapADOType2Jdbc(adLongVarBinary)},
        {adChapter,          ADOS::MapADOType2Jdbc(adChapter)},
        {adFileTime,         ADOS::MapADOType2Jdbc(adFileTime)},
        {adPropVariant,      ADOS::MapADOType2Jdbc(adPropVariant)},
        {adVarNumeric,       ADOS::MapADOType2Jdbc(adVarNumeric)}
    };
    m_aValueRange[12] = aMap;

    std::map< sal_Int32,OUString> aMap2 {
        {0, "YES"},
        {1, "NO"}
    };
    m_aIntValueRange[18] = aMap2;

    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setColumnsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTablesMap()
{

    for(sal_Int32 i=1;i<5;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(6);

    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setTablesMap();
    m_xMetaData = pMetaData;
}

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

    ::std::map<sal_Int32,sal_Int32> aMap
    {
        {DBTYPE_EMPTY,  DataType::SQLNULL},
        {DBTYPE_NULL,  DataType::SQLNULL},
        {DBTYPE_I2,  DataType::SMALLINT},
        {DBTYPE_I4,  DataType::INTEGER},
        {DBTYPE_R4,  DataType::FLOAT},
        {DBTYPE_R8,  DataType::DOUBLE},
        {DBTYPE_CY,  DataType::BIGINT},
        {DBTYPE_DATE,  DataType::DATE},
        {DBTYPE_BSTR,  DataType::VARCHAR},
        {DBTYPE_IDISPATCH,  DataType::OBJECT},
        {DBTYPE_ERROR,  DataType::OTHER},
        {DBTYPE_BOOL,  DataType::BIT},
        {DBTYPE_VARIANT,  DataType::STRUCT},
        {DBTYPE_IUNKNOWN,  DataType::OTHER},
        {DBTYPE_DECIMAL,  DataType::DECIMAL},
        {DBTYPE_UI1,  DataType::TINYINT},
        {DBTYPE_ARRAY,  DataType::ARRAY},
        {DBTYPE_BYREF,  DataType::REF},
        {DBTYPE_I1,  DataType::CHAR},
        {DBTYPE_UI2,  DataType::SMALLINT},
        {DBTYPE_UI4,  DataType::INTEGER},

        // aMap[The] = ;
        // aMap[in] = ;
        {DBTYPE_I8,  DataType::BIGINT},
        {DBTYPE_UI8,  DataType::BIGINT},
        {DBTYPE_GUID,  DataType::OTHER},
        {DBTYPE_VECTOR,  DataType::OTHER},
        {DBTYPE_FILETIME,  DataType::OTHER},
        {DBTYPE_RESERVED,  DataType::OTHER},

        // aMap[The] = ;
        {DBTYPE_BYTES,  DataType::VARBINARY},
        {DBTYPE_STR,  DataType::LONGVARCHAR},
        {DBTYPE_WSTR,  DataType::LONGVARCHAR},
        {DBTYPE_NUMERIC,  DataType::NUMERIC},
        {DBTYPE_UDT,  DataType::OTHER},
        {DBTYPE_DBDATE,  DataType::DATE},
        {DBTYPE_DBTIME,  DataType::TIME},
        {DBTYPE_DBTIMESTAMP,  DataType::TIMESTAMP},
        {DBTYPE_HCHAPTER,  DataType::OTHER},
        {DBTYPE_PROPVARIANT,  DataType::OTHER},
        {DBTYPE_VARNUMERIC,  DataType::NUMERIC}
    };

    m_aValueRange[10] = aMap;

    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setProcedureColumnsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setPrimaryKeysMap()
{

    sal_Int32 i=1;
    for(;i<5;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(7);
    m_aColMapping.push_back(8);

    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setProcedureColumnsMap();
    m_xMetaData = pMetaData;
}

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

    ::std::map<sal_Int32,sal_Int32> aMap
    {
        { DBPROPVAL_IT_HASH, IndexType::HASHED },
        { DBPROPVAL_IT_CONTENT, IndexType::OTHER },
        { DBPROPVAL_IT_OTHER, IndexType::OTHER },
        { DBPROPVAL_IT_BTREE, IndexType::OTHER }
    };

    m_aValueRange[10] = aMap;

    ::std::map<sal_Int32,sal_Int32> aMap2
    {
        { 0, 1 },
        { 1, 0 }
    };
    m_aValueRange[8] = aMap2;

    std::map< sal_Int32,OUString> aMap3
    {
        { 0, "" },
        { DB_COLLATION_ASC, "A" },
        { DB_COLLATION_DESC, "D" }
    };

    m_aIntValueRange[21] = aMap3;

    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setIndexInfoMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTablePrivilegesMap()
{

    sal_Int32 i=3;
    for(;i<6;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(1);
    m_aColMapping.push_back(2);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(7);

    std::map< sal_Int32,OUString> aMap
    {
        { 0, "YES" },
        { 1, "NO" }
    };
    m_aIntValueRange[7] = aMap;


    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setTablePrivilegesMap();
    m_xMetaData = pMetaData;
}

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

    std::map< OUString,sal_Int32> aMap
    {
        { "CASCADE", KeyRule::CASCADE },
        { "RESTRICT", KeyRule::RESTRICT },
        { "SET NULL", KeyRule::SET_NULL },
        { "SET DEFAULT", KeyRule::SET_DEFAULT },
        { "NO ACTION", KeyRule::NO_ACTION }
    };

    m_aStrValueRange[14] = aMap;
    m_aStrValueRange[15] = aMap;

    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setCrossReferenceMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTypeInfoMap(bool _bJetEngine)
{
    sal_Int32 i=1;
    for(;i<19;i++)
        m_aColMapping.push_back(i);

    std::map< OUString,sal_Int32> aMap1 { { OUString(), 10 } };

    m_aStrValueRange[18] = aMap1;

    ::std::map<sal_Int32,sal_Int32> aMap
    {
        {adEmpty,            ADOS::MapADOType2Jdbc(adEmpty)},
        {adTinyInt,          ADOS::MapADOType2Jdbc(adTinyInt)},
        {adSmallInt,         ADOS::MapADOType2Jdbc(adSmallInt)},
        {adInteger,          ADOS::MapADOType2Jdbc(adInteger)},
        {adBigInt,           ADOS::MapADOType2Jdbc(adBigInt)},
        {adUnsignedTinyInt,  ADOS::MapADOType2Jdbc(adUnsignedTinyInt)},
        {adUnsignedSmallInt, ADOS::MapADOType2Jdbc(adUnsignedSmallInt)},
        {adUnsignedInt,      ADOS::MapADOType2Jdbc(adUnsignedInt)},
        {adUnsignedBigInt,   ADOS::MapADOType2Jdbc(adUnsignedBigInt)},
        {adSingle,           ADOS::MapADOType2Jdbc(adSingle)},
        {adDouble,           ADOS::MapADOType2Jdbc(adDouble)},
        {adCurrency,         ADOS::MapADOType2Jdbc(adCurrency)},
        {adDecimal,          ADOS::MapADOType2Jdbc(adDecimal)},
        {adNumeric,          ADOS::MapADOType2Jdbc(adNumeric)},
        {adBoolean,          ADOS::MapADOType2Jdbc(adBoolean)},
        {adError,            ADOS::MapADOType2Jdbc(adError)},
        {adUserDefined,      ADOS::MapADOType2Jdbc(adUserDefined)},
        {adVariant,          ADOS::MapADOType2Jdbc(adVariant)},
        {adIDispatch,        ADOS::MapADOType2Jdbc(adIDispatch)},
        {adIUnknown,         ADOS::MapADOType2Jdbc(adIUnknown)},
        {adGUID,             ADOS::MapADOType2Jdbc(adGUID)},
        {adDate,             ADOS::MapADOType2Jdbc(_bJetEngine?adDBTimeStamp:adDate)},
        {adDBDate,           ADOS::MapADOType2Jdbc(adDBDate)},
        {adDBTime,           ADOS::MapADOType2Jdbc(adDBTime)},
        {adDBTimeStamp,      ADOS::MapADOType2Jdbc(adDBTimeStamp)},
        {adBSTR,             ADOS::MapADOType2Jdbc(adBSTR)},
        {adChar,             ADOS::MapADOType2Jdbc(adChar)},
        {adVarChar,          ADOS::MapADOType2Jdbc(adVarChar)},
        {adLongVarChar,      ADOS::MapADOType2Jdbc(adLongVarChar)},
        {adWChar,            ADOS::MapADOType2Jdbc(adWChar)},
        {adVarWChar,         ADOS::MapADOType2Jdbc(adVarWChar)},
        {adLongVarWChar,     ADOS::MapADOType2Jdbc(adLongVarWChar)},
        {adBinary,           ADOS::MapADOType2Jdbc(adBinary)},
        {adVarBinary,        ADOS::MapADOType2Jdbc(adVarBinary)},
        {adLongVarBinary,    ADOS::MapADOType2Jdbc(adLongVarBinary)},
        {adChapter,          ADOS::MapADOType2Jdbc(adChapter)},
        {adFileTime,         ADOS::MapADOType2Jdbc(adFileTime)},
        {adPropVariant,      ADOS::MapADOType2Jdbc(adPropVariant)},
        {adVarNumeric,       ADOS::MapADOType2Jdbc(adVarNumeric)}
//      {adArray,            ADOS::MapADOType2Jdbc(adArray)}
    };

    m_aValueRange[2] = aMap;

    ::std::map<sal_Int32,sal_Int32> aColumnValueMapping
    {
        { VARIANT_FALSE, ColumnValue::NO_NULLS },
        { VARIANT_TRUE, ColumnValue::NULLABLE }
    };
    m_aValueRange[7] = aColumnValueMapping;

    // now adjust the column mapping
    // OJ 24.01.2002  96860
    ::std::map<sal_Int32,sal_Int32> aSearchMapping
    {
        { DB_UNSEARCHABLE,      ColumnSearch::NONE },
        { DB_LIKE_ONLY,         ColumnSearch::CHAR },
        { DB_ALL_EXCEPT_LIKE,   ColumnSearch::BASIC },
        { DB_SEARCHABLE,        ColumnSearch::FULL }
    };

    m_aValueRange[9] = aSearchMapping;

    ::std::map<sal_Int32,sal_Int32> aCurrencyMapping;
    m_aValueRange[11] = aCurrencyMapping;

    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setTypeInfoMap();
    m_xMetaData = pMetaData;
}

void SAL_CALL ODatabaseMetaDataResultSet::acquire() noexcept
{
    ODatabaseMetaDataResultSet_BASE::acquire();
}

void SAL_CALL ODatabaseMetaDataResultSet::release() noexcept
{
    ODatabaseMetaDataResultSet_BASE::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL ODatabaseMetaDataResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

OLEVariant ODatabaseMetaDataResultSet::getValue(sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);

    checkRecordSet();

    WpADOField aField = ADOS::getField(m_pRecordSet,columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
