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

#include <ado/AStatement.hxx>
#include <ado/AConnection.hxx>
#include <ado/AResultSet.hxx>
#include <comphelper/property.hxx>
#include <osl/thread.h>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <connectivity/dbexception.hxx>
#include <comphelper/types.hxx>

#undef max

#include <algorithm>
#include <numeric>

using namespace ::comphelper;

#define CHECK_RETURN(x)                                                 \
    if(!x)                                                              \
        ADOS::ThrowException(*m_pConnection->getConnection(),*this);


using namespace connectivity::ado;


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace ::std;

OStatement_Base::OStatement_Base(OConnection* _pConnection ) :  OStatement_BASE(m_aMutex)
                                                        ,OPropertySetHelper(OStatement_BASE::rBHelper)
                                                        ,m_pConnection(_pConnection)
                                                        ,m_nMaxRows(0)
                                                        ,m_nFetchSize(1)
                                                        ,m_eLockType(adLockReadOnly)
                                                        ,m_eCursorType(adOpenForwardOnly)
{
    osl_atomic_increment( &m_refCount );

    m_Command.Create();
    if(m_Command.IsValid())
        m_Command.putref_ActiveConnection(m_pConnection->getConnection());
    else
        ADOS::ThrowException(*m_pConnection->getConnection(),*this);

    m_RecordsAffected.setNoArg();
    m_Parameters.setNoArg();

    m_pConnection->acquire();

    osl_atomic_decrement( &m_refCount );
}

void OStatement_Base::disposeResultSet()
{
    // free the cursor if alive
    Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_xResultSet.clear();
}


void OStatement_Base::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);


    disposeResultSet();

    if ( m_Command.IsValid() )
        m_Command.putref_ActiveConnection( nullptr );
    m_Command.clear();

    if ( m_RecordSet.IsValid() )
        m_RecordSet.PutRefDataSource( nullptr );
    m_RecordSet.clear();

    if (m_pConnection)
        m_pConnection->release();

    OStatement_BASE::disposing();
}

void SAL_CALL OStatement_Base::release() throw()
{
    OStatement_BASE::release();
}

Any SAL_CALL OStatement_Base::queryInterface( const Type & rType )
{
    Any aRet = OStatement_BASE::queryInterface(rType);
    return aRet.hasValue() ? aRet : OPropertySetHelper::queryInterface(rType);
}

css::uno::Sequence< css::uno::Type > SAL_CALL OStatement_Base::getTypes(  )
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<css::beans::XMultiPropertySet>::get(),
                                    cppu::UnoType<css::beans::XFastPropertySet>::get(),
                                    cppu::UnoType<css::beans::XPropertySet>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE::getTypes());
}


void SAL_CALL OStatement_Base::cancel(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    CHECK_RETURN(m_Command.Cancel())
}


void SAL_CALL OStatement_Base::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    }
    dispose();
}


void SAL_CALL OStatement::clearBatch(  )
{

}


void OStatement_Base::reset()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    clearWarnings ();

    if (m_xResultSet.get().is())
        clearMyResultSet();
}

// clearMyResultSet
// If a ResultSet was created for this Statement, close it


void OStatement_Base::clearMyResultSet ()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    try
    {
        Reference<XCloseable> xCloseable(
            m_xResultSet.get(), css::uno::UNO_QUERY);
        if ( xCloseable.is() )
            xCloseable->close();
    }
    catch( const DisposedException& ) { }

    m_xResultSet.clear();
}

sal_Int32 OStatement_Base::getRowCount ()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return m_RecordsAffected.getInt32();
}

// getPrecision
// Given a SQL type, return the maximum precision for the column.
// Returns -1 if not known


sal_Int32 OStatement_Base::getPrecision ( sal_Int32 sqlType)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    sal_Int32 prec = -1;
    OTypeInfo aInfo;
    aInfo.nType = static_cast<sal_Int16>(sqlType);
    if (!m_aTypeInfo.empty())
    {
        std::vector<OTypeInfo>::const_iterator aIter = std::find(m_aTypeInfo.begin(),m_aTypeInfo.end(),aInfo);
        for(;aIter != m_aTypeInfo.end();++aIter)
        {
            prec = std::max(prec,(*aIter).nPrecision);
        }
    }

    return prec;
}

// setWarning
// Sets the warning


void OStatement_Base::setWarning (const SQLWarning &ex)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aLastWarning = ex;
}

void OStatement_Base::assignRecordSet( ADORecordset* _pRS )
{
    WpADORecordset aOldRS( m_RecordSet );
    m_RecordSet = WpADORecordset( _pRS );

    if ( aOldRS.IsValid() )
        aOldRS.PutRefDataSource( nullptr );

    if ( m_RecordSet.IsValid() )
        m_RecordSet.PutRefDataSource( static_cast<IDispatch*>(m_Command) );
}

sal_Bool SAL_CALL OStatement_Base::execute( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    // Reset the statement handle and warning

    reset();

    try
    {
        ADORecordset* pSet = nullptr;
        CHECK_RETURN(m_Command.put_CommandText(sql))
        CHECK_RETURN(m_Command.Execute(m_RecordsAffected,m_Parameters,adCmdText,&pSet))

        assignRecordSet( pSet );
    }
    catch (SQLWarning& ex)
    {

        // Save pointer to warning and save with ResultSet
        // object once it is created.

        m_aLastWarning = ex;
    }

    return m_RecordSet.IsValid();
}

Reference< XResultSet > SAL_CALL OStatement_Base::executeQuery( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    reset();

    m_xResultSet = WeakReference<XResultSet>(nullptr);

    WpADORecordset aSet;
    aSet.Create();
    CHECK_RETURN(m_Command.put_CommandText(sql))
    OLEVariant aCmd;
    aCmd.setIDispatch(m_Command);
    OLEVariant aCon;
    aCon.setNoArg();
    CHECK_RETURN(aSet.put_CacheSize(m_nFetchSize))
    CHECK_RETURN(aSet.put_MaxRecords(m_nMaxRows))
    CHECK_RETURN(aSet.Open(aCmd,aCon,m_eCursorType,m_eLockType,adOpenUnspecified))


    CHECK_RETURN(aSet.get_CacheSize(m_nFetchSize))
    CHECK_RETURN(aSet.get_MaxRecords(m_nMaxRows))
    CHECK_RETURN(aSet.get_CursorType(m_eCursorType))
    CHECK_RETURN(aSet.get_LockType(m_eLockType))

    OResultSet* pSet = new OResultSet(aSet,this);
    Reference< XResultSet > xRs = pSet;
    pSet->construct();

    m_xResultSet = WeakReference<XResultSet>(xRs);

    return xRs;
}


Reference< XConnection > SAL_CALL OStatement_Base::getConnection(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return static_cast<Reference< XConnection >>(m_pConnection);
}


Any SAL_CALL OStatement::queryInterface( const Type & rType )
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    return aRet.hasValue() ? aRet : OStatement_Base::queryInterface(rType);
}


void SAL_CALL OStatement::addBatch( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aBatchVector.push_back(sql);
}

Sequence< sal_Int32 > SAL_CALL OStatement::executeBatch(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    reset();

    OUString aBatchSql = std::accumulate(m_aBatchVector.begin(), m_aBatchVector.end(), OUString(),
        [](const OUString& rRes, const OUString& rStr) { return rRes + rStr + ";"; });
    sal_Int32 nLen = m_aBatchVector.size();


    if ( m_RecordSet.IsValid() )
        m_RecordSet.PutRefDataSource( nullptr );
    m_RecordSet.clear();
    m_RecordSet.Create();

    CHECK_RETURN(m_Command.put_CommandText(aBatchSql))
    if ( m_RecordSet.IsValid() )
        m_RecordSet.PutRefDataSource(static_cast<IDispatch*>(m_Command));

    CHECK_RETURN(m_RecordSet.UpdateBatch(adAffectAll))

    ADORecordset* pSet=nullptr;
    Sequence< sal_Int32 > aRet(nLen);
    sal_Int32* pArray = aRet.getArray();
    for(sal_Int32 j=0;j<nLen;++j)
    {
        pSet = nullptr;
        OLEVariant aRecordsAffected;
        if(m_RecordSet.NextRecordset(aRecordsAffected,&pSet) && pSet)
        {
            assignRecordSet( pSet );

            ADO_LONGPTR nValue;
            if(m_RecordSet.get_RecordCount(nValue))
                pArray[j] = nValue;
        }
    }
    return aRet;
}


sal_Int32 SAL_CALL OStatement_Base::executeUpdate( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    reset();

    try {
        ADORecordset* pSet = nullptr;
        CHECK_RETURN(m_Command.put_CommandText(sql))
        CHECK_RETURN(m_Command.Execute(m_RecordsAffected,m_Parameters,adCmdText|adExecuteNoRecords,&pSet))
    }
    catch (SQLWarning& ex) {

        // Save pointer to warning and save with ResultSet
        // object once it is created.

        m_aLastWarning = ex;
    }
    if(!m_RecordsAffected.isEmpty() && !m_RecordsAffected.isNull() && m_RecordsAffected.getType() != VT_ERROR)
        return m_RecordsAffected.getInt32();

    return 0;
}


Reference< XResultSet > SAL_CALL OStatement_Base::getResultSet(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return m_xResultSet;
}


sal_Int32 SAL_CALL OStatement_Base::getUpdateCount(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    ADO_LONGPTR nRet;
    if(m_RecordSet.IsValid() && m_RecordSet.get_RecordCount(nRet))
        return nRet;
    return -1;
}


sal_Bool SAL_CALL OStatement_Base::getMoreResults(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    SQLWarning  warning;

    // clear previous warnings

    clearWarnings ();

    // Call SQLMoreResults

    try
    {
        ADORecordset* pSet=nullptr;
        OLEVariant aRecordsAffected;
        if(m_RecordSet.IsValid() && m_RecordSet.NextRecordset(aRecordsAffected,&pSet) && pSet)
            assignRecordSet( pSet );
    }
    catch (SQLWarning &ex)
    {

        // Save pointer to warning and save with ResultSet
        // object once it is created.

        warning = ex;
    }
    return m_RecordSet.IsValid();
}


Any SAL_CALL OStatement_Base::getWarnings(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return makeAny(m_aLastWarning);
}


void SAL_CALL OStatement_Base::clearWarnings(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aLastWarning = SQLWarning();
}


sal_Int32 OStatement_Base::getQueryTimeOut() const
{
    return m_Command.get_CommandTimeout();
}

sal_Int32 OStatement_Base::getMaxRows() const
{
    ADO_LONGPTR nRet=-1;
    if(!(m_RecordSet.IsValid() && m_RecordSet.get_MaxRecords(nRet)))
        ::dbtools::throwFunctionSequenceException(nullptr);
    return nRet;
}

sal_Int32 OStatement_Base::getResultSetConcurrency() const
{
    sal_Int32 nValue;

    switch(m_eLockType)
    {
        case adLockReadOnly:
            nValue = ResultSetConcurrency::READ_ONLY;
            break;
        default:
            nValue = ResultSetConcurrency::UPDATABLE;
            break;
    }

    return nValue;
}

sal_Int32 OStatement_Base::getResultSetType() const
{
    sal_Int32 nValue=0;
    switch(m_eCursorType)
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
    return nValue;
}

sal_Int32 OStatement_Base::getFetchDirection()
{
    return FetchDirection::FORWARD;
}

sal_Int32 OStatement_Base::getFetchSize() const
{
    return m_nFetchSize;
}

sal_Int32 OStatement_Base::getMaxFieldSize()
{
    return 0;
}

OUString OStatement_Base::getCursorName() const
{
    return m_Command.GetName();
}

void OStatement_Base::setQueryTimeOut(sal_Int32 seconds)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_Command.put_CommandTimeout(seconds);
}

void OStatement_Base::setMaxRows(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    m_nMaxRows = _par0;
}

void OStatement_Base::setResultSetConcurrency(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    switch(_par0)
    {
        case ResultSetConcurrency::READ_ONLY:
            m_eLockType = adLockReadOnly;
            break;
        default:
            m_eLockType = adLockOptimistic;
            break;
    }
}

void OStatement_Base::setResultSetType(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    switch(_par0)
    {
        case ResultSetType::FORWARD_ONLY:
            m_eCursorType = adOpenForwardOnly;
            break;
        case ResultSetType::SCROLL_INSENSITIVE:
            m_eCursorType = adOpenKeyset;
            break;
        case ResultSetType::SCROLL_SENSITIVE:
            m_eCursorType = adOpenDynamic;
            break;
    }
}

void OStatement_Base::setFetchDirection(sal_Int32 /*_par0*/)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedSQLException( "Statement::FetchDirection", *this );
}

void OStatement_Base::setFetchSize(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_nFetchSize = _par0;
}

void OStatement_Base::setMaxFieldSize(sal_Int32 /*_par0*/)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedSQLException( "Statement::MaxFieldSize", *this );
}

void OStatement_Base::setCursorName(const OUString &_par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    m_Command.put_Name(_par0);
}


::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
{
    Sequence< css::beans::Property > aProps(10);
    css::beans::Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),
        PROPERTY_ID_CURSORNAME, cppu::UnoType<OUString>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ESCAPEPROCESSING),
        PROPERTY_ID_ESCAPEPROCESSING, cppu::UnoType<bool>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
        PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
        PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXFIELDSIZE),
        PROPERTY_ID_MAXFIELDSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXROWS),
        PROPERTY_ID_MAXROWS, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_QUERYTIMEOUT),
        PROPERTY_ID_QUERYTIMEOUT, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
        PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
        PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_USEBOOKMARKS),
        PROPERTY_ID_USEBOOKMARKS, cppu::UnoType<bool>::get(), 0);

    return new ::cppu::OPropertyArrayHelper(aProps);
}


::cppu::IPropertyArrayHelper & OStatement_Base::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool OStatement_Base::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
{
    bool bModified = false;

    bool bValidAdoRS = m_RecordSet.IsValid();
        // some of the properties below, when set, are remembered in a member, and applied in the next execute
        // For these properties, the record set does not need to be valid to allow setting them.
        // For all others (where the values are forwarded to the ADO RS directly), the recordset must be valid.

    try
    {
        switch(nHandle)
        {
            case PROPERTY_ID_MAXROWS:
                bModified = ::comphelper::tryPropertyValue( rConvertedValue, rOldValue, rValue, bValidAdoRS ? getMaxRows() : m_nMaxRows );
                break;

            case PROPERTY_ID_RESULTSETTYPE:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getResultSetType());
                break;
            case PROPERTY_ID_FETCHSIZE:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
                break;
            case PROPERTY_ID_RESULTSETCONCURRENCY:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getResultSetConcurrency());
                break;
            case PROPERTY_ID_QUERYTIMEOUT:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getQueryTimeOut());
                break;
            case PROPERTY_ID_MAXFIELDSIZE:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getMaxFieldSize());
                break;
            case PROPERTY_ID_CURSORNAME:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getCursorName());
                break;
            case PROPERTY_ID_FETCHDIRECTION:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
                break;
        }
    }
    catch( const Exception& )
    {
        bModified = true;   // will ensure that the property is set
        OSL_FAIL( "OStatement_Base::convertFastPropertyValue: caught something strange!" );
    }
    return bModified;
}

void OStatement_Base::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
{
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
            setQueryTimeOut(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_MAXFIELDSIZE:
            setMaxFieldSize(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_MAXROWS:
            setMaxRows(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_CURSORNAME:
            setCursorName(comphelper::getString(rValue));
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            setResultSetConcurrency(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            setResultSetType(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            setFetchDirection(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHSIZE:
            setFetchSize(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_ESCAPEPROCESSING:
            //  return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAsLink);
        case PROPERTY_ID_USEBOOKMARKS:
            //  return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAsLink);
        default:
            ;
    }
}

void OStatement_Base::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
            rValue <<= getQueryTimeOut();
            break;
        case PROPERTY_ID_MAXFIELDSIZE:
            rValue <<= getMaxFieldSize();
            break;
        case PROPERTY_ID_MAXROWS:
            rValue <<= getMaxRows();
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
        case PROPERTY_ID_ESCAPEPROCESSING:
            rValue <<= true;
            break;
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}

OStatement::~OStatement()
{
}
IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.AStatement","com.sun.star.sdbc.Statement");

void SAL_CALL OStatement_Base::acquire() throw()
{
    OStatement_BASE::acquire();
}

void SAL_CALL OStatement::acquire() throw()
{
    OStatement_Base::acquire();
}

void SAL_CALL OStatement::release() throw()
{
    OStatement_Base::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OStatement_Base::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
