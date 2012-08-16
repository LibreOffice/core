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

#include "ado/AStatement.hxx"
#include "ado/AConnection.hxx"
#include "ado/AResultSet.hxx"
#include <comphelper/property.hxx>
#include <comphelper/uno3.hxx>
#include <osl/thread.h>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include "connectivity/dbexception.hxx"
#include <comphelper/types.hxx>

#undef max

#include <algorithm>

using namespace ::comphelper;

#define CHECK_RETURN(x)                                                 \
    if(!x)                                                              \
        ADOS::ThrowException(*m_pConnection->getConnection(),*this);



using namespace connectivity::ado;

//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace ::std;
//------------------------------------------------------------------------------
OStatement_Base::OStatement_Base(OConnection* _pConnection ) :  OStatement_BASE(m_aMutex)
                                                        ,OPropertySetHelper(OStatement_BASE::rBHelper)
                                                        ,OSubComponent<OStatement_Base, OStatement_BASE>((::cppu::OWeakObject*)_pConnection, this)
                                                        ,m_pConnection(_pConnection)
                                                        ,m_nMaxRows(0)
                                                        ,m_nFetchSize(1)
                                                        ,m_eLockType(adLockReadOnly)
                                                        ,m_eCursorType(adOpenForwardOnly)
{
    osl_incrementInterlockedCount( &m_refCount );

    m_Command.Create();
    if(m_Command.IsValid())
        m_Command.putref_ActiveConnection(m_pConnection->getConnection());
    else
        ADOS::ThrowException(*m_pConnection->getConnection(),*this);

    m_RecordsAffected.setNoArg();
    m_Parameters.setNoArg();

    m_pConnection->acquire();

    osl_decrementInterlockedCount( &m_refCount );
}
//------------------------------------------------------------------------------
void OStatement_Base::disposeResultSet()
{
    // free the cursor if alive
    Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_xResultSet = Reference< XResultSet>();
}

//------------------------------------------------------------------------------
void OStatement_Base::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);


    disposeResultSet();

    if ( m_Command.IsValid() )
        m_Command.putref_ActiveConnection( NULL );
    m_Command.clear();

    if ( m_RecordSet.IsValid() )
        m_RecordSet.PutRefDataSource( NULL );
    m_RecordSet.clear();

    if (m_pConnection)
        m_pConnection->release();

    dispose_ChildImpl();
    OStatement_BASE::disposing();
}
//-----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::release() throw()
{
    relase_ChildImpl();
}
//-----------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE::queryInterface(rType);
    return aRet.hasValue() ? aRet : OPropertySetHelper::queryInterface(rType);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OStatement_Base::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE::getTypes());
}

// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    CHECK_RETURN(m_Command.Cancel())
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement::clearBatch(  ) throw(SQLException, RuntimeException)
{

}
// -------------------------------------------------------------------------

void OStatement_Base::reset() throw (SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    clearWarnings ();

    if (m_xResultSet.get().is())
        clearMyResultSet();
}
//--------------------------------------------------------------------
// clearMyResultSet
// If a ResultSet was created for this Statement, close it
//--------------------------------------------------------------------

void OStatement_Base::clearMyResultSet () throw (SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    try
    {
        Reference<XCloseable> xCloseable;
        if ( ::comphelper::query_interface( m_xResultSet.get(), xCloseable ) )
            xCloseable->close();
    }
    catch( const DisposedException& ) { }

    m_xResultSet = Reference< XResultSet >();
}
//--------------------------------------------------------------------
sal_Int32 OStatement_Base::getRowCount () throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return m_RecordsAffected;
}
//--------------------------------------------------------------------
// getPrecision
// Given a SQL type, return the maximum precision for the column.
// Returns -1 if not known
//--------------------------------------------------------------------

sal_Int32 OStatement_Base::getPrecision ( sal_Int32 sqlType)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    sal_Int32 prec = -1;
    OTypeInfo aInfo;
    aInfo.nType = (sal_Int16)sqlType;
    if (!m_aTypeInfo.empty())
    {
        ::std::vector<OTypeInfo>::const_iterator aIter = ::std::find(m_aTypeInfo.begin(),m_aTypeInfo.end(),aInfo);
        for(;aIter != m_aTypeInfo.end();++aIter)
        {
            prec = ::std::max(prec,(*aIter).nPrecision);
        }
    }

    return prec;
}
//--------------------------------------------------------------------
// setWarning
// Sets the warning
//--------------------------------------------------------------------

void OStatement_Base::setWarning (const SQLWarning &ex) throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aLastWarning = ex;
}
// -------------------------------------------------------------------------
void OStatement_Base::assignRecordSet( ADORecordset* _pRS )
{
    WpADORecordset aOldRS( m_RecordSet );
    m_RecordSet = WpADORecordset( _pRS );

    if ( aOldRS.IsValid() )
        aOldRS.PutRefDataSource( NULL );

    if ( m_RecordSet.IsValid() )
        m_RecordSet.PutRefDataSource( (IDispatch*)m_Command );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStatement_Base::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    // Reset the statement handle and warning

    reset();

    try
    {
        ADORecordset* pSet = NULL;
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
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OStatement_Base::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    reset();

    m_xResultSet = WeakReference<XResultSet>(NULL);

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
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OStatement_Base::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Any SAL_CALL OStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    return aRet.hasValue() ? aRet : OStatement_Base::queryInterface(rType);
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement::addBatch( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aBatchList.push_back(sql);
}
// -------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL OStatement::executeBatch(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    reset();

    ::rtl::OUString aBatchSql;
    sal_Int32 nLen = 0;
    for(::std::list< ::rtl::OUString>::const_iterator i=m_aBatchList.begin();i != m_aBatchList.end();++i,++nLen)
        aBatchSql = aBatchSql + *i + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";"));


    if ( m_RecordSet.IsValid() )
        m_RecordSet.PutRefDataSource( NULL );
    m_RecordSet.clear();
    m_RecordSet.Create();

    CHECK_RETURN(m_Command.put_CommandText(aBatchSql))
    if ( m_RecordSet.IsValid() )
        m_RecordSet.PutRefDataSource((IDispatch*)m_Command);

    CHECK_RETURN(m_RecordSet.UpdateBatch(adAffectAll))

    ADORecordset* pSet=NULL;
    Sequence< sal_Int32 > aRet(nLen);
    sal_Int32* pArray = aRet.getArray();
    for(sal_Int32 j=0;j<nLen;++j)
    {
        pSet = NULL;
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
// -------------------------------------------------------------------------


sal_Int32 SAL_CALL OStatement_Base::executeUpdate( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    reset();

    try {
        ADORecordset* pSet = NULL;
        CHECK_RETURN(m_Command.put_CommandText(sql))
        CHECK_RETURN(m_Command.Execute(m_RecordsAffected,m_Parameters,adCmdText|adExecuteNoRecords,&pSet))
    }
    catch (SQLWarning& ex) {

        // Save pointer to warning and save with ResultSet
        // object once it is created.

        m_aLastWarning = ex;
    }
    if(!m_RecordsAffected.isEmpty() && !m_RecordsAffected.isNull() && m_RecordsAffected.getType() != VT_ERROR)
        return m_RecordsAffected;

    return 0;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement_Base::getResultSet(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return m_xResultSet;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OStatement_Base::getUpdateCount(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    ADO_LONGPTR nRet;
    if(m_RecordSet.IsValid() && m_RecordSet.get_RecordCount(nRet))
        return nRet;
    return -1;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OStatement_Base::getMoreResults(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    SQLWarning  warning;

    // clear previous warnings

    clearWarnings ();

    // Call SQLMoreResults

    try
    {
        ADORecordset* pSet=NULL;
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
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return makeAny(m_aLastWarning);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void SAL_CALL OStatement_Base::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aLastWarning = SQLWarning();
}
// -------------------------------------------------------------------------
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getQueryTimeOut() const  throw(SQLException, RuntimeException)
{
    return m_Command.get_CommandTimeout();
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getMaxRows() const throw(SQLException, RuntimeException)
{
    ADO_LONGPTR nRet=-1;
    if(!(m_RecordSet.IsValid() && m_RecordSet.get_MaxRecords(nRet)))
        ::dbtools::throwFunctionSequenceException(NULL);
    return nRet;
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getResultSetConcurrency() const throw(SQLException, RuntimeException)
{
    return m_eLockType;
#if 0 // Huh?
    sal_Int32 nValue=0;
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
#endif
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getResultSetType() const throw(SQLException, RuntimeException)
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
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getFetchDirection() const throw(SQLException, RuntimeException)
{
    return FetchDirection::FORWARD;
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getFetchSize() const throw(SQLException, RuntimeException)
{
    return m_nFetchSize;
}
//------------------------------------------------------------------------------
sal_Int32 OStatement_Base::getMaxFieldSize() const throw(SQLException, RuntimeException)
{
    return 0;
}
//------------------------------------------------------------------------------
::rtl::OUString OStatement_Base::getCursorName() const throw(SQLException, RuntimeException)
{
    return m_Command.GetName();
}
//------------------------------------------------------------------------------
void OStatement_Base::setQueryTimeOut(sal_Int32 seconds) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_Command.put_CommandTimeout(seconds);
}
//------------------------------------------------------------------------------
void OStatement_Base::setMaxRows(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    m_nMaxRows = _par0;
}
//------------------------------------------------------------------------------
void OStatement_Base::setResultSetConcurrency(sal_Int32 _par0) throw(SQLException, RuntimeException)
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
//------------------------------------------------------------------------------
void OStatement_Base::setResultSetType(sal_Int32 _par0) throw(SQLException, RuntimeException)
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
//------------------------------------------------------------------------------
void OStatement_Base::setFetchDirection(sal_Int32 /*_par0*/) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "Statement::FetchDirection", *this );
}
//------------------------------------------------------------------------------
void OStatement_Base::setFetchSize(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_nFetchSize = _par0;
    //  m_RecordSet.put_CacheSize(_par0);
}
//------------------------------------------------------------------------------
void OStatement_Base::setMaxFieldSize(sal_Int32 /*_par0*/) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "Statement::MaxFieldSize", *this );
}
//------------------------------------------------------------------------------
void OStatement_Base::setCursorName(const ::rtl::OUString &_par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    m_Command.put_Name(_par0);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
{
    Sequence< com::sun::star::beans::Property > aProps(10);
    com::sun::star::beans::Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,  ::rtl::OUString);
    DECL_BOOL_PROP0(ESCAPEPROCESSING);
    DECL_PROP0(FETCHDIRECTION,sal_Int32);
    DECL_PROP0(FETCHSIZE,   sal_Int32);
    DECL_PROP0(MAXFIELDSIZE,sal_Int32);
    DECL_PROP0(MAXROWS,     sal_Int32);
    DECL_PROP0(QUERYTIMEOUT,sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,sal_Int32);
    DECL_BOOL_PROP0(USEBOOKMARKS);


    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OStatement_Base::getInfoHelper()
{
    return *const_cast<OStatement_Base*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OStatement_Base::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bModified = sal_False;

    sal_Bool bValidAdoRS = m_RecordSet.IsValid();
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
    catch( const Exception& e )
    {
        bModified = sal_True;   // will ensure that the property is set
        OSL_FAIL( "OStatement_Base::convertFastPropertyValue: caught something strange!" );
        (void)e;
    }
    return bModified;
}
// -------------------------------------------------------------------------
void OStatement_Base::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
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
// -------------------------------------------------------------------------
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
            rValue <<= sal_True;
            break;
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}
// -------------------------------------------------------------------------
OStatement::~OStatement()
{
}
IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.AStatement","com.sun.star.sdbc.Statement");
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::acquire() throw()
{
    OStatement_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::acquire() throw()
{
    OStatement_Base::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::release() throw()
{
    OStatement_Base::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OStatement_Base::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
