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

#include "ado/AResultSet.hxx"
#include "ado/AResultSetMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <comphelper/property.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/seqstream.hxx>
#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"
#include <comphelper/types.hxx>

using namespace ::comphelper;


#include <oledb.h>

#define CHECK_RETURN(x)                                                 \
    if(!SUCCEEDED(x))                                                               \
        ADOS::ThrowException(*m_pStmt->m_pConnection->getConnection(),*this);

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

//------------------------------------------------------------------------------
//  IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.AResultSet","com.sun.star.sdbc.ResultSet");
OUString SAL_CALL OResultSet::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)  \
{
    return OUString("com.sun.star.sdbcx.ado.ResultSet");
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< OUString > SAL_CALL OResultSet::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< OUString > aSupported(2);
    aSupported[0] = OUString("com.sun.star.sdbc.ResultSet");
    aSupported[1] = OUString("com.sun.star.sdbcx.ResultSet");
    return aSupported;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::supportsService( const OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pSupported = aSupported.getConstArray();
    const OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
OResultSet::OResultSet(ADORecordset* _pRecordSet,OStatement_Base* pStmt) :  OResultSet_BASE(m_aMutex)
                        ,OPropertySetHelper(OResultSet_BASE::rBHelper)
                        ,m_pRecordSet(_pRecordSet)
                        ,m_pStmt(pStmt)
                        ,m_xStatement(*pStmt)
                        ,m_xMetaData(NULL)
                        ,m_nRowPos(0)
                        ,m_bWasNull(sal_False)
                        ,m_bEOF(sal_False)
                        ,m_bOnFirstAfterOpen(sal_False)
{
}
// -------------------------------------------------------------------------
OResultSet::OResultSet(ADORecordset* _pRecordSet) : OResultSet_BASE(m_aMutex)
                        ,OPropertySetHelper(OResultSet_BASE::rBHelper)
                        ,m_pRecordSet(_pRecordSet)
                        ,m_pStmt(NULL)
                        ,m_xStatement(NULL)
                        ,m_xMetaData(NULL)
                        ,m_nRowPos(0)
                        ,m_bWasNull(sal_False)
                        ,m_bEOF(sal_False)
                        ,m_bOnFirstAfterOpen(sal_False)
{
}
// -----------------------------------------------------------------------------
void OResultSet::construct()
{
    osl_atomic_increment( &m_refCount );
    if (!m_pRecordSet)
    {
        OSL_FAIL( "OResultSet::construct: no RecordSet!" );
        Reference< XInterface > xInt( *this );
        osl_atomic_decrement( &m_refCount );
        ::dbtools::throwFunctionSequenceException( xInt );
    }
    m_pRecordSet->AddRef();
    VARIANT_BOOL bIsAtBOF;
    CHECK_RETURN(m_pRecordSet->get_BOF(&bIsAtBOF))
    m_bOnFirstAfterOpen = bIsAtBOF != VARIANT_TRUE;
    osl_atomic_decrement( &m_refCount );
}
// -------------------------------------------------------------------------
OResultSet::~OResultSet()
{
    if(m_pRecordSet)
        m_pRecordSet->Release();
}
// -------------------------------------------------------------------------
void OResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_pRecordSet)
        m_pRecordSet->Close();
m_xStatement.clear();
m_xMetaData.clear();
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet_BASE::queryInterface(rType);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OResultSet::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}

// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::findColumn( const OUString& columnName ) throw(SQLException, RuntimeException)
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
    /* FIXME: should throw in case of not found ? */
    return i;
}
#define BLOCK_SIZE 256
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

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

        return new ::comphelper::SequenceInputStream(aData);
    }
    // else we ask for a bytesequence
    aField.get_Value(m_aValue);

    return m_aValue.isNull() ? NULL : new ::comphelper::SequenceInputStream(m_aValue);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getCharacterStream", *this );
    return NULL;
}
// -----------------------------------------------------------------------------
OLEVariant OResultSet::getValue(sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    WpADOField aField = ADOS::getField(m_pRecordSet,columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
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


    PositionEnum_Param aPos;
    m_pRecordSet->get_AbsolutePosition(&aPos);
    return  (aPos > 0) ? static_cast<sal_Int32>(aPos) : m_nRowPos;
    // return the rowcount from driver if the driver doesn't support this return our count
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getLong", *this );
    return sal_Int64(0);
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pRecordSet);
    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getArray", *this );
    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getClob", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getBlob", *this );
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getRef", *this );
    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex).makeAny();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------

OUString SAL_CALL OResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
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


    VARIANT_BOOL bIsAtEOF;
    CHECK_RETURN(m_pRecordSet->get_EOF(&bIsAtEOF))
    return bIsAtEOF == VARIANT_TRUE;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(first())
        m_bOnFirstAfterOpen = !previous();
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
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(SUCCEEDED(m_pRecordSet->MoveFirst()))
    {
        m_nRowPos = 1;
        m_bOnFirstAfterOpen = sal_False;
        return sal_True;
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Bool bRet = SUCCEEDED(m_pRecordSet->MoveLast());
    if(bRet)
    {
        m_pRecordSet->get_RecordCount(&m_nRowPos);
        m_bOnFirstAfterOpen = sal_False;
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!row)                 // absolute with zero not allowed
        ::dbtools::throwFunctionSequenceException(*this);

    sal_Bool bCheck = sal_True;
    if(row < 0)
    {
        bCheck = SUCCEEDED(m_pRecordSet->MoveLast());
        if ( bCheck )
        {
            while(++row < 0 && bCheck)
                bCheck = SUCCEEDED(m_pRecordSet->MovePrevious());
        }
    }
    else
    {
        first();
        OLEVariant aEmpty;
        aEmpty.setNoArg();
        bCheck = SUCCEEDED(m_pRecordSet->Move(row-1,aEmpty)); // move to row -1 because we stand already on the first
        if(bCheck)
            m_nRowPos = row;
    }
    if(bCheck)
        m_bOnFirstAfterOpen = sal_False;
    return bCheck;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OLEVariant aEmpty;
    aEmpty.setNoArg();
    sal_Int32 nNewPos = row;
    if ( m_bOnFirstAfterOpen && nNewPos > 0 )
        --nNewPos;
    sal_Bool bRet = SUCCEEDED(m_pRecordSet->Move(row,aEmpty));
    if(bRet)
    {
        m_nRowPos += row;
        m_bOnFirstAfterOpen = sal_False;
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    sal_Bool bRet = SUCCEEDED(m_pRecordSet->MovePrevious());
    if(bRet)
    {
        --m_nRowPos;
        m_bOnFirstAfterOpen = sal_False;
    }
    return bRet;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_xStatement;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    sal_Bool  bRet = (eRec & adRecDeleted) == adRecDeleted;
    if(bRet)
        --m_nRowPos;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{   ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    sal_Bool  bRet = (eRec & adRecNew) == adRecNew;
    if(bRet)
        ++m_nRowPos;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    return (eRec & adRecModified) == adRecModified;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OSL_ENSURE(!m_nRowPos,"OResultSet::isBeforeFirst: Error in setting m_nRowPos!");
    VARIANT_BOOL bIsAtBOF = VARIANT_TRUE;
    if(!m_bOnFirstAfterOpen)
    {
        OSL_ENSURE(!m_nRowPos,"OResultSet::isBeforeFirst: Error in setting m_nRowPos!");
        m_pRecordSet->get_BOF(&bIsAtBOF);
    }
    return bIsAtBOF == VARIANT_TRUE;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Bool bRet = sal_True;
    if(m_bOnFirstAfterOpen)
    {
        m_bOnFirstAfterOpen = sal_False;
        ++m_nRowPos;
    }
    else
    {
        bRet = SUCCEEDED(m_pRecordSet->MoveNext());

        if(bRet)
        {
            VARIANT_BOOL bIsAtEOF;
            CHECK_RETURN(m_pRecordSet->get_EOF(&bIsAtEOF))
            bRet = bIsAtEOF != VARIANT_TRUE;
            ++m_nRowPos;
        }
        else
            ADOS::ThrowException(*m_pStmt->m_pConnection->getConnection(),*this);
    }

    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_aValue.isNull();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_pRecordSet->Cancel();
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


    OLEVariant aEmpty;
    aEmpty.setNoArg();
    m_pRecordSet->AddNew(aEmpty,aEmpty);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OLEVariant aEmpty;
    aEmpty.setNoArg();
    m_pRecordSet->Update(aEmpty,aEmpty);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::deleteRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_pRecordSet->Delete(adAffectCurrent);
    m_pRecordSet->UpdateBatch(adAffectCurrent);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_pRecordSet->CancelUpdate();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
 //   ::osl::MutexGuard aGuard( m_aMutex );
    //checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
 //   if ( getResultSetConcurrency() == ResultSetConcurrency::READ_ONLY )
 //       throw SQLException();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
}
// -----------------------------------------------------------------------------
void OResultSet::updateValue(sal_Int32 columnIndex,const OLEVariant& x)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    WpADOField aField = ADOS::getField(m_pRecordSet,columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OLEVariant x;
    x.setNull();
    updateValue(columnIndex,x);
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
    updateValue(columnIndex,x);
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
void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const OUString& x ) throw(SQLException, RuntimeException)
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
    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    updateBytes(columnIndex,aSeq);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    updateBytes(columnIndex,aSeq);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_pRecordSet->Resync(adAffectCurrent,adResyncAllValues);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 /*scale*/ ) throw(SQLException, RuntimeException)
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}
//------------------------------------------------------------------------------
// XRowLocate
Any SAL_CALL OResultSet::getBookmark(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(m_nRowPos < (sal_Int32)m_aBookmarks.size()) // this bookmark was already fetched
        return makeAny(sal_Int32(m_nRowPos-1));

    OLEVariant aVar;
    m_pRecordSet->get_Bookmark(&aVar);
    m_aBookmarks.push_back(aVar);
    return makeAny((sal_Int32)(m_aBookmarks.size()-1));

}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int32 nPos = 0;
    bookmark >>= nPos;
    OSL_ENSURE(nPos >= 0 && nPos < (sal_Int32)m_aBookmarks.size(),"Invalid Index for vector");
    if(nPos < 0 || nPos >= (sal_Int32)m_aBookmarks.size())
        ::dbtools::throwFunctionSequenceException(*this);

    return SUCCEEDED(m_pRecordSet->Move(0,m_aBookmarks[nPos]));
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int32 nPos = 0;
    bookmark >>= nPos;
    nPos += rows;
    OSL_ENSURE(nPos >= 0 && nPos < (sal_Int32)m_aBookmarks.size(),"Invalid Index for vector");
    if(nPos < 0 || nPos >= (sal_Int32)m_aBookmarks.size())
        ::dbtools::throwFunctionSequenceException(*this);
    return SUCCEEDED(m_pRecordSet->Move(rows,m_aBookmarks[nPos]));
}
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::compareBookmarks( const Any& bookmark1, const Any& bookmark2 ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nPos1 = 0;
    bookmark1 >>= nPos1;
    sal_Int32 nPos2 = 0;
    bookmark2 >>= nPos2;
    if(nPos1 == nPos2)  // they should be equal
        return sal_True;

    OSL_ENSURE((nPos1 >= 0 && nPos1 < (sal_Int32)m_aBookmarks.size()) || (nPos1 >= 0 && nPos2 < (sal_Int32)m_aBookmarks.size()),"Invalid Index for vector");

    CompareEnum eNum;
    m_pRecordSet->CompareBookmarks(m_aBookmarks[nPos1],m_aBookmarks[nPos2],&eNum);
    return ((sal_Int32)eNum) +1;
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    ADOProperties* pProps = NULL;
    m_pRecordSet->get_Properties(&pProps);
    WpADOProperties aProps;
    aProps.setWithOutAddRef(pProps);
    ADOS::ThrowException(*((OConnection*)m_pStmt->getConnection().get())->getConnection(),*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");

    WpADOProperty aProp(aProps.GetItem(OUString("Bookmarks Ordered")));
    OLEVariant aVar;
    if(aProp.IsValid())
        aVar = aProp.GetValue();
    else
        ADOS::ThrowException(*((OConnection*)m_pStmt->getConnection().get())->getConnection(),*this);

    sal_Bool bValue(sal_False);
    if(!aVar.isNull() && !aVar.isEmpty())
        bValue = aVar;
    return bValue;
}
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int32 nPos = 0;
    bookmark >>= nPos;
    return nPos;
}
//------------------------------------------------------------------------------
// XDeleteRows
Sequence< sal_Int32 > SAL_CALL OResultSet::deleteRows( const Sequence< Any >& rows ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OLEVariant aVar;
    sal_Int32 nPos = 0;

    // Create SafeArray Bounds and initialize the array
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = rows.getLength();
    SAFEARRAY *psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    const Any* pBegin = rows.getConstArray();
    const Any* pEnd = pBegin + rows.getLength();
    for(sal_Int32 i=0;pBegin != pEnd ;++pBegin,++i)
    {
        *pBegin >>= nPos;
        SafeArrayPutElement(psa,&i,&m_aBookmarks[nPos]);
    }

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    m_pRecordSet->put_Filter(vsa);
    m_pRecordSet->Delete(adAffectGroup);
    m_pRecordSet->UpdateBatch(adAffectGroup);

    Sequence< sal_Int32 > aSeq(rows.getLength());
    if(first())
    {
        sal_Int32* pSeq = aSeq.getArray();
        sal_Int32 i=0;
        do
        {
            OSL_ENSURE(i<aSeq.getLength(),"Index greater than length of sequence");
            m_pRecordSet->get_Status(&pSeq[i]);
            if(pSeq[i++] == adRecDeleted)
                --m_nRowPos;
        }
        while(next());
    }
    return aSeq;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getResultSetConcurrency() const
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nValue=ResultSetConcurrency::READ_ONLY;
    LockTypeEnum eRet;
    if(!SUCCEEDED(m_pRecordSet->get_LockType(&eRet)))
    {
        switch(eRet)
        {
            case adLockReadOnly:
                nValue = ResultSetConcurrency::READ_ONLY;
                break;
            default:
                nValue = ResultSetConcurrency::UPDATABLE;
                break;
        }
    }
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getResultSetType() const
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nValue=0;
    CursorTypeEnum eRet;
    if(!SUCCEEDED(m_pRecordSet->get_CursorType(&eRet)))
    {
        switch(eRet)
        {
            case adOpenUnspecified:
            case adOpenForwardOnly:
                nValue = ResultSetType::FORWARD_ONLY;
                break;
            case adOpenStatic:
            case adOpenKeyset:
                nValue = ResultSetType::SCROLL_INSENSITIVE;
                break;
            case adOpenDynamic:
                nValue = ResultSetType::SCROLL_SENSITIVE;
                break;
        }
    }
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getFetchDirection() const
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return FetchDirection::FORWARD;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getFetchSize() const
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nValue=-1;
    m_pRecordSet->get_CacheSize(&nValue);
    return nValue;
}
//------------------------------------------------------------------------------
OUString OResultSet::getCursorName() const
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return OUString();
}

//------------------------------------------------------------------------------
void OResultSet::setFetchDirection(sal_Int32 /*_par0*/)
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "ResultSet::FetchDirection", *this );
}
//------------------------------------------------------------------------------
void OResultSet::setFetchSize(sal_Int32 _par0)
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_pRecordSet->put_CacheSize(_par0);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< com::sun::star::beans::Property > aProps(5);
    com::sun::star::beans::Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;

    //  DECL_PROP1IMPL(CURSORNAME,          OUString) PropertyAttribute::READONLY);
    DECL_PROP0(FETCHDIRECTION,          sal_Int32);
    DECL_PROP0(FETCHSIZE,               sal_Int32);
    DECL_BOOL_PROP1IMPL(ISBOOKMARKABLE) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETCONCURRENCY,sal_Int32) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETTYPE,       sal_Int32) PropertyAttribute::READONLY);

    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *const_cast<OResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
            break;
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
void OResultSet::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            setFetchDirection(getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHSIZE:
            setFetchSize(getINT32(rValue));
            break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OResultSet::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
            {
                VARIANT_BOOL bBool;
                m_pRecordSet->Supports(adBookmark,&bBool);
                sal_Bool bRet = bBool == VARIANT_TRUE;
                rValue.setValue(&bRet, ::getCppuBooleanType() );
            }
            break;
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
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
