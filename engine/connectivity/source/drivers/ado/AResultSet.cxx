/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <ado/AResultSet.hxx>
#include <ado/AResultSetMetaData.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <comphelper/property.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/seqstream.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>

using namespace ::comphelper;


#include <oledb.h>

#define CHECK_RETURN(x)                                                 \
    if(!SUCCEEDED(x))                                                               \
        ADOS::ThrowException(m_pStmt->m_pConnection->getConnection(),*this);

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace cpo::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;


//  IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.AResultSet","com.sun.star.sdbc.ResultSet");
OUString OResultSet::getImplementationName(  )
{
    return "com.sun.star.sdbcx.ado.ResultSet";
}

cpo::uno::Sequence< OUString > OResultSet::getSupportedServiceNames(  )
{
    return {"com.sun.star.sdbc.ResultSet","com.sun.star.sdbcx.ResultSet"};
}

bool OResultSet::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

OResultSet::OResultSet(ADORecordset* _pRecordSet,OStatement_Base* pStmt) :  OResultSet_BASE(m_aMutex)
                        ,OPropertySetHelper(OResultSet_BASE::rBHelper)
                        ,m_pRecordSet(_pRecordSet)
                        ,m_pStmt(pStmt)
                        ,m_xStatement(*pStmt)
                        ,m_nRowPos(0)
                        ,m_bEOF(false)
                        ,m_bOnFirstAfterOpen(false)
{
}

OResultSet::OResultSet(ADORecordset* _pRecordSet) : OResultSet_BASE(m_aMutex)
                        ,OPropertySetHelper(OResultSet_BASE::rBHelper)
                        ,m_pRecordSet(_pRecordSet)
                        ,m_pStmt(nullptr)
                        ,m_nRowPos(0)
                        ,m_bEOF(false)
                        ,m_bOnFirstAfterOpen(false)
{
}

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

OResultSet::~OResultSet()
{
    if(m_pRecordSet)
        m_pRecordSet->Release();
}

void OResultSet::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_pRecordSet)
        m_pRecordSet->Close();
    m_xStatement.clear();
    m_xMetaData.clear();
}

Any OResultSet::queryInterface( const Type & rType )
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet_BASE::queryInterface(rType);
}

cpo::uno::Sequence< cpo::uno::Type > OResultSet::getTypes(  )
{
    return comphelper::concatSequences(cppu::OPropertySetHelper::getTypes(),
                                       OResultSet_BASE::getTypes());
}

sal_Int32 OResultSet::findColumn( const OUString& columnName )
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
}

#define BLOCK_SIZE 256

Reference< css::io::XInputStream > OResultSet::getBinaryStream( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

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

        return new ::comphelper::SequenceInputStream(aData);
    }
    // else we ask for a bytesequence
    aField.get_Value(m_aValue);

    return m_aValue.isNull() ? nullptr : new ::comphelper::SequenceInputStream(m_aValue.getByteSequence());
}

Reference< css::io::XInputStream > OResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getCharacterStream", *this );
}

OLEVariant OResultSet::getValue(sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    WpADOField aField = ADOS::getField(m_pRecordSet,columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}

bool OResultSet::getBoolean( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getBool();
}


sal_Int8 OResultSet::getByte( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getInt8();
}


Sequence< sal_Int8 > OResultSet::getBytes( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getByteSequence();
}


css::util::Date OResultSet::getDate( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getDate();
}


double OResultSet::getDouble( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getDouble();
}


float OResultSet::getFloat( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getFloat();
}


sal_Int32 OResultSet::getInt( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getInt32();
}


sal_Int32 OResultSet::getRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    PositionEnum aPos;
    m_pRecordSet->get_AbsolutePosition(&aPos);
    return  (aPos > 0) ? static_cast<sal_Int32>(aPos) : m_nRowPos;
    // return the rowcount from driver if the driver doesn't support this return our count
}


sal_Int64 OResultSet::getLong( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getLong", *this );
}


Reference< XResultSetMetaData > OResultSet::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pRecordSet);
    return m_xMetaData;
}

Reference< XArray > OResultSet::getArray( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getArray", *this );
}

Reference< XClob > OResultSet::getClob( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getClob", *this );
}

Reference< XBlob > OResultSet::getBlob( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getBlob", *this );
}

Reference< XRef > OResultSet::getRef( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getRef", *this );
}

Any OResultSet::getObject( sal_Int32 columnIndex, const Reference< css::container::XNameAccess >& /*typeMap*/ )
{
    return getValue(columnIndex).makeAny();
}


sal_Int16 OResultSet::getShort( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getInt16();
}


OUString OResultSet::getString( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getString();
}


css::util::Time OResultSet::getTime( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getTime();
}


css::util::DateTime OResultSet::getTimestamp( sal_Int32 columnIndex )
{
    return getValue(columnIndex).getDateTime();
}


bool OResultSet::isAfterLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    VARIANT_BOOL bIsAtEOF;
    CHECK_RETURN(m_pRecordSet->get_EOF(&bIsAtEOF))
    return bIsAtEOF == VARIANT_TRUE;
}

bool OResultSet::isFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == 1;
}

bool OResultSet::isLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return true;
}

void OResultSet::beforeFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(first())
        m_bOnFirstAfterOpen = !previous();
}

void OResultSet::afterLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(last())
        next();
    m_bEOF = true;
}


void OResultSet::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    }
    dispose();
}


bool OResultSet::first(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(SUCCEEDED(m_pRecordSet->MoveFirst()))
    {
        m_nRowPos = 1;
        m_bOnFirstAfterOpen = false;
        return true;
    }
    return false;
}


bool OResultSet::last(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    bool bRet = SUCCEEDED(m_pRecordSet->MoveLast());
    if(bRet)
    {
        m_pRecordSet->get_RecordCount(&m_nRowPos);
        m_bOnFirstAfterOpen = false;
    }
    return bRet;
}

bool OResultSet::absolute( sal_Int32 row )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!row)                 // absolute with zero not allowed
        ::dbtools::throwFunctionSequenceException(*this);

    bool bCheck = true;
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
        m_bOnFirstAfterOpen = false;
    return bCheck;
}

bool OResultSet::relative( sal_Int32 row )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OLEVariant aEmpty;
    aEmpty.setNoArg();
    sal_Int32 nNewPos = row;
    if ( m_bOnFirstAfterOpen && nNewPos > 0 )
        --nNewPos;
    bool bRet = SUCCEEDED(m_pRecordSet->Move(row,aEmpty));
    if(bRet)
    {
        m_nRowPos += row;
        m_bOnFirstAfterOpen = false;
    }
    return bRet;
}

bool OResultSet::previous(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    bool bRet = SUCCEEDED(m_pRecordSet->MovePrevious());
    if(bRet)
    {
        --m_nRowPos;
        m_bOnFirstAfterOpen = false;
    }
    return bRet;
}

Reference< XInterface > OResultSet::getStatement(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_xStatement;
}


bool OResultSet::rowDeleted(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int32 eRec;
    m_pRecordSet->get_Status(&eRec);
    bool  bRet = (RecordStatusEnum(eRec) & adRecDeleted) == adRecDeleted;
    if(bRet)
        --m_nRowPos;
    return bRet;
}

bool OResultSet::rowInserted(  )
{   ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int32 eRec;
    m_pRecordSet->get_Status(&eRec);
    bool  bRet = (RecordStatusEnum(eRec) & adRecNew) == adRecNew;
    if(bRet)
        ++m_nRowPos;
    return bRet;
}

bool OResultSet::rowUpdated(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int32 eRec;
    m_pRecordSet->get_Status(&eRec);
    return (RecordStatusEnum(eRec) & adRecModified) == adRecModified;
}


bool OResultSet::isBeforeFirst(  )
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


bool OResultSet::next(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    bool bRet = true;
    if(m_bOnFirstAfterOpen)
    {
        m_bOnFirstAfterOpen = false;
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
            ADOS::ThrowException(m_pStmt->m_pConnection->getConnection(),*this);
    }

    return bRet;
}


bool OResultSet::wasNull(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_aValue.isNull();
}


void OResultSet::cancel(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_pRecordSet->Cancel();
}

void OResultSet::clearWarnings(  )
{
}

Any OResultSet::getWarnings(  )
{
    return Any();
}

void OResultSet::insertRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OLEVariant aEmpty;
    aEmpty.setNoArg();
    m_pRecordSet->AddNew(aEmpty,aEmpty);
}

void OResultSet::updateRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OLEVariant aEmpty;
    aEmpty.setNoArg();
    m_pRecordSet->Update(aEmpty,aEmpty);
}

void OResultSet::deleteRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_pRecordSet->Delete();
    m_pRecordSet->UpdateBatch(adAffectCurrent);
}


void OResultSet::cancelRowUpdates(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_pRecordSet->CancelUpdate();
}


void OResultSet::moveToInsertRow(  )
{
 //   ::osl::MutexGuard aGuard( m_aMutex );
    //checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
 //   if ( getResultSetConcurrency() == ResultSetConcurrency::READ_ONLY )
 //       throw SQLException();
}


void OResultSet::moveToCurrentRow(  )
{
}

void OResultSet::updateValue(sal_Int32 columnIndex,const OLEVariant& x)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    WpADOField aField = ADOS::getField(m_pRecordSet,columnIndex);
    aField.PutValue(x);
}

void OResultSet::updateNull( sal_Int32 columnIndex )
{
    OLEVariant x;
    x.setNull();
    updateValue(columnIndex,x);
}


void OResultSet::updateBoolean( sal_Int32 columnIndex, bool x )
{
    updateValue(columnIndex,bool(x));
}

void OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x )
{
    updateValue(columnIndex,x);
}


void OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x )
{
    updateValue(columnIndex,x);
}

void OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x )
{
    updateValue(columnIndex,x);
}

void OResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x )
{
    updateValue(columnIndex,x);
}

void OResultSet::updateFloat( sal_Int32 columnIndex, float x )
{
    updateValue(columnIndex,x);
}


void OResultSet::updateDouble( sal_Int32 columnIndex, double x )
{
    updateValue(columnIndex,x);
}

void OResultSet::updateString( sal_Int32 columnIndex, const OUString& x )
{
    updateValue(columnIndex,x);
}

void OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x )
{
    updateValue(columnIndex,x);
}

void OResultSet::updateDate( sal_Int32 columnIndex, const css::util::Date& x )
{
    updateValue(columnIndex,x);
}


void OResultSet::updateTime( sal_Int32 columnIndex, const css::util::Time& x )
{
    updateValue(columnIndex,x);
}


void OResultSet::updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x )
{
    updateValue(columnIndex,x);
}


void OResultSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    updateBytes(columnIndex,aSeq);
}

void OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    updateBytes(columnIndex,aSeq);
}

void OResultSet::refreshRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_pRecordSet->Resync(adAffectCurrent);
}

void OResultSet::updateObject( sal_Int32 columnIndex, const Any& x )
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}


void OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 /*scale*/ )
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}

// XRowLocate
Any OResultSet::getBookmark(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(m_nRowPos < static_cast<sal_Int32>(m_aBookmarks.size())) // this bookmark was already fetched
        return Any(sal_Int32(m_nRowPos-1));

    OLEVariant aVar;
    m_pRecordSet->get_Bookmark(&aVar);
    m_aBookmarks.push_back(aVar);
    return Any(static_cast<sal_Int32>(m_aBookmarks.size()-1));

}

bool OResultSet::moveToBookmark( const Any& bookmark )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int32 nPos = 0;
    bookmark >>= nPos;
    OSL_ENSURE(nPos >= 0 && nPos < static_cast<sal_Int32>(m_aBookmarks.size()),"Invalid Index for vector");
    if(nPos < 0 || nPos >= static_cast<sal_Int32>(m_aBookmarks.size()))
        ::dbtools::throwFunctionSequenceException(*this);

    return SUCCEEDED(m_pRecordSet->Move(0,m_aBookmarks[nPos]));
}

bool OResultSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int32 nPos = 0;
    bookmark >>= nPos;
    nPos += rows;
    OSL_ENSURE(nPos >= 0 && nPos < static_cast<sal_Int32>(m_aBookmarks.size()),"Invalid Index for vector");
    if(nPos < 0 || nPos >= static_cast<sal_Int32>(m_aBookmarks.size()))
        ::dbtools::throwFunctionSequenceException(*this);
    return SUCCEEDED(m_pRecordSet->Move(rows,m_aBookmarks[nPos]));
}

sal_Int32 OResultSet::compareBookmarks( const Any& bookmark1, const Any& bookmark2 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nPos1 = 0;
    bookmark1 >>= nPos1;
    sal_Int32 nPos2 = 0;
    bookmark2 >>= nPos2;
    if(nPos1 == nPos2)  // they should be equal
        return css::sdbcx::CompareBookmark::EQUAL;

    OSL_ENSURE((nPos1 >= 0 && nPos1 < static_cast<sal_Int32>(m_aBookmarks.size())) || (nPos1 >= 0 && nPos2 < static_cast<sal_Int32>(m_aBookmarks.size())),"Invalid Index for vector");

    CompareEnum eNum;
    m_pRecordSet->CompareBookmarks(m_aBookmarks[nPos1],m_aBookmarks[nPos2],&eNum);
    return static_cast<sal_Int32>(eNum) - 1;
}

bool OResultSet::hasOrderedBookmarks(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    WpADOProperties aProps;
    m_pRecordSet->get_Properties(&aProps);
    ADOS::ThrowException(static_cast<OConnection*>(m_pStmt->getConnection().get())->getConnection(),*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");

    WpADOProperty aProp(aProps.GetItem(OUString("Bookmarks Ordered")));
    OLEVariant aVar;
    if(aProp.IsValid())
        aVar = aProp.GetValue();
    else
        ADOS::ThrowException(static_cast<OConnection*>(m_pStmt->getConnection().get())->getConnection(),*this);

    bool bValue(false);
    if(!aVar.isNull() && !aVar.isEmpty())
        bValue = aVar.getBool();
    return bValue;
}

sal_Int32 OResultSet::hashBookmark( const Any& bookmark )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int32 nPos = 0;
    bookmark >>= nPos;
    return nPos;
}

// XDeleteRows
Sequence< sal_Int32 > OResultSet::deleteRows( const Sequence< Any >& rows )
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

    for (sal_Int32 i = 0; i < rows.getLength(); ++i)
    {
        rows[i] >>= nPos;
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

sal_Int32 OResultSet::getResultSetConcurrency() const
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

sal_Int32 OResultSet::getResultSetType() const
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

sal_Int32 OResultSet::getFetchDirection()
{
    return FetchDirection::FORWARD;
}

sal_Int32 OResultSet::getFetchSize() const
{
    sal_Int32 nValue=-1;
    m_pRecordSet->get_CacheSize(&nValue);
    return nValue;
}

OUString OResultSet::getCursorName()
{
    return OUString();
}


void OResultSet::setFetchDirection(sal_Int32 /*_par0*/)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "ResultSet::FetchDirection", *this );
}

void OResultSet::setFetchSize(sal_Int32 _par0)
{
    m_pRecordSet->put_CacheSize(_par0);
}

::cppu::IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    return new ::cppu::OPropertyArrayHelper
    {
        {
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
                PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
                PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),
                PROPERTY_ID_ISBOOKMARKABLE, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
                PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
                PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY
            }
        }
    };
}

::cppu::IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

bool OResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw css::lang::IllegalArgumentException();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
        case PROPERTY_ID_FETCHSIZE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        default:
            ;
    }
    return false;
}

void OResultSet::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception("cannot set prop " + OUString::number(nHandle), nullptr);
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

void OResultSet::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
            {
                VARIANT_BOOL bBool;
                m_pRecordSet->Supports(adBookmark,&bBool);
                rValue <<= (bBool == VARIANT_TRUE);
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

void OResultSet::acquire() noexcept
{
    OResultSet_BASE::acquire();
}

void OResultSet::release() noexcept
{
    OResultSet_BASE::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > OResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
