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

#include "ado/AConnection.hxx"
#include "ado/ADatabaseMetaData.hxx"
#include "ado/ADriver.hxx"
#include "ado/AStatement.hxx"
#include "ado/ACallableStatement.hxx"
#include "ado/APreparedStatement.hxx"
#include "ado/ACatalog.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include "connectivity/dbexception.hxx"
#include <osl/file.hxx>
#include "resource/ado_res.hrc"

#include <o3tl/compat_functional.hxx>

using namespace dbtools;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OConnection,"com.sun.star.sdbcx.AConnection","com.sun.star.sdbc.Connection");
// --------------------------------------------------------------------------------
OConnection::OConnection(ODriver*   _pDriver) throw(SQLException, RuntimeException)
                         : OSubComponent<OConnection, OConnection_BASE>((::cppu::OWeakObject*)_pDriver, this),
                         m_xCatalog(NULL),
                         m_pDriver(_pDriver),
                         m_pAdoConnection(NULL),
                         m_pCatalog(NULL),
                         m_nEngineType(0),
                         m_bClosed(sal_False),
                         m_bAutocommit(sal_True)
{
    osl_atomic_increment( &m_refCount );

    IClassFactory2* pIUnknown   = NULL;
    IUnknown        *pOuter     = NULL;
    HRESULT         hr;
    hr = CoGetClassObject( ADOS::CLSID_ADOCONNECTION_21,
                          CLSCTX_INPROC_SERVER,
                          NULL,
                          IID_IClassFactory2,
                          (void**)&pIUnknown );

    if( !FAILED( hr ) )
    {
        ADOConnection *pCon         = NULL;
        hr = pIUnknown->CreateInstanceLic(  pOuter,
                                            NULL,
                                            ADOS::IID_ADOCONNECTION_21,
                                            ADOS::GetKeyStr(),
                                            (void**) &pCon);

        if( !FAILED( hr ) )
        {
            OSL_ENSURE( pCon, "OConnection::OConnection: invalid ADO object!" );

            m_pAdoConnection = new WpADOConnection( pCon );
            // CreateInstanceLic returned an object which was already acquired
            pCon->Release( );

        }

        // Class Factory is no longer needed
        pIUnknown->Release();
    }

    osl_atomic_decrement( &m_refCount );
}
//-----------------------------------------------------------------------------
OConnection::~OConnection()
{
}
//-----------------------------------------------------------------------------
void OConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)
{
    osl_atomic_increment( &m_refCount );

    setConnectionInfo(info);

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aDSN(url.copy(nLen+1)),aUID,aPWD;
    if ( aDSN.compareToAscii("access:",7) == 0 )
        aDSN = aDSN.copy(7);

    sal_Int32 nTimeout = 20;
    sal_Bool bSilent = sal_True;
    const PropertyValue *pIter  = info.getConstArray();
    const PropertyValue *pEnd   = pIter + info.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if(!pIter->Name.compareToAscii("Timeout"))
            pIter->Value >>= nTimeout;
        else if(!pIter->Name.compareToAscii("Silent"))
            pIter->Value >>= bSilent;
        else if(!pIter->Name.compareToAscii("user"))
            pIter->Value >>= aUID;
        else if(!pIter->Name.compareToAscii("password"))
            pIter->Value >>= aPWD;
    }
    try
    {
        if(m_pAdoConnection)
        {
            if(m_pAdoConnection->Open(aDSN,aUID,aPWD,adConnectUnspecified))
                m_pAdoConnection->PutCommandTimeout(nTimeout);
            else
                ADOS::ThrowException(*m_pAdoConnection,*this);
            if(m_pAdoConnection->get_State() != adStateOpen)
                throwGenericSQLException( STR_NO_CONNECTION,*this );

            WpADOProperties aProps = m_pAdoConnection->get_Properties();
            if(aProps.IsValid())
            {
                OTools::putValue(aProps,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Jet OLEDB:ODBC Parsing")),sal_True);
                OLEVariant aVar(OTools::getValue(aProps,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Jet OLEDB:Engine Type"))));
                if(!aVar.isNull() && !aVar.isEmpty())
                    m_nEngineType = aVar;
            }
            buildTypeInfo();
            //bErg = TRUE;
        }
        else
            ::dbtools::throwFunctionSequenceException(*this);

    }
    catch(const Exception& )
    {
        osl_atomic_decrement( &m_refCount );
        throw;
    }
    osl_atomic_decrement( &m_refCount );
}
//-----------------------------------------------------------------------------
void SAL_CALL OConnection::release() throw()
{
    relase_ChildImpl();
}
// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    OStatement* pStmt = new OStatement(this);
    Reference< XStatement > xStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OPreparedStatement* pStmt = new OPreparedStatement(this,m_aTypeInfo,sql);
    Reference< XPreparedStatement > xPStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xPStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OCallableStatement* pStmt = new OCallableStatement(this,m_aTypeInfo,sql);
    Reference< XPreparedStatement > xPStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xPStmt;
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::nativeSQL( const ::rtl::OUString& _sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    ::rtl::OUString sql = _sql;
    WpADOProperties aProps = m_pAdoConnection->get_Properties();
    if(aProps.IsValid())
    {
        OTools::putValue(aProps,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Jet OLEDB:ODBC Parsing")),sal_True);
        WpADOCommand aCommand;
        aCommand.Create();
        aCommand.put_ActiveConnection((IDispatch*)*m_pAdoConnection);
        aCommand.put_CommandText(sql);
        sql = aCommand.get_CommandText();
    }

    return sql;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    m_bAutocommit = autoCommit;
    if(!autoCommit)
        m_pAdoConnection->BeginTrans();
    else
        m_pAdoConnection->RollbackTrans();
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    return m_bAutocommit;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    m_pAdoConnection->CommitTrans();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    m_pAdoConnection->RollbackTrans();
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return OConnection_BASE::rBHelper.bDisposed && !m_pAdoConnection->get_State();
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new ODatabaseMetaData(this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);



    m_pAdoConnection->put_Mode(readOnly ? adModeRead : adModeReadWrite);
    ADOS::ThrowException(*m_pAdoConnection,*this);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    return m_pAdoConnection->get_Mode() == adModeRead;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setCatalog( const ::rtl::OUString& catalog ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_pAdoConnection->PutDefaultDatabase(catalog);
    ADOS::ThrowException(*m_pAdoConnection,*this);
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    return m_pAdoConnection->GetDefaultDatabase();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    IsolationLevelEnum eIso;
    switch(level)
    {
        case TransactionIsolation::NONE:
            eIso = adXactUnspecified;
            break;
        case TransactionIsolation::READ_UNCOMMITTED:
            eIso = adXactReadUncommitted;
            break;
        case TransactionIsolation::READ_COMMITTED:
            eIso = adXactReadCommitted;
            break;
        case TransactionIsolation::REPEATABLE_READ:
            eIso = adXactRepeatableRead;
            break;
        case TransactionIsolation::SERIALIZABLE:
            eIso = adXactSerializable;
            break;
        default:
            OSL_FAIL("OConnection::setTransactionIsolation invalid level");
            return;
    }
    m_pAdoConnection->put_IsolationLevel(eIso);
    ADOS::ThrowException(*m_pAdoConnection,*this);
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    sal_Int32 nRet = 0;
    switch(m_pAdoConnection->get_IsolationLevel())
    {
        case adXactUnspecified:
            nRet = TransactionIsolation::NONE;
            break;
        case adXactReadUncommitted:
            nRet = TransactionIsolation::READ_UNCOMMITTED;
            break;
        case adXactReadCommitted:
            nRet = TransactionIsolation::READ_COMMITTED;
            break;
        case adXactRepeatableRead:
            nRet = TransactionIsolation::REPEATABLE_READ;
            break;
        case adXactSerializable:
            nRet = TransactionIsolation::SERIALIZABLE;
            break;
        default:
            OSL_FAIL("OConnection::setTransactionIsolation invalid level");
    }
    ADOS::ThrowException(*m_pAdoConnection,*this);
    return nRet;
}
// --------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    return NULL;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setTypeMap", *this );
}
// --------------------------------------------------------------------------------
// XCloseable
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// --------------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
//--------------------------------------------------------------------
void OConnection::buildTypeInfo() throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ADORecordset *pRecordset = m_pAdoConnection->getTypeInfo();
    if ( pRecordset )
    {
        pRecordset->AddRef();
        VARIANT_BOOL bIsAtBOF;
        pRecordset->get_BOF(&bIsAtBOF);

        sal_Bool bOk = sal_True;
        if ( bIsAtBOF == VARIANT_TRUE )
            bOk = SUCCEEDED(pRecordset->MoveNext());

        if ( bOk )
        {
            // HACK for access
            static const ::rtl::OUString s_sVarChar(RTL_CONSTASCII_USTRINGPARAM("VarChar"));
            do
            {
                sal_Int32 nPos = 1;
                OExtendedTypeInfo* aInfo            = new OExtendedTypeInfo();
                aInfo->aSimpleType.aTypeName        = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->eType                        = (DataTypeEnum)(sal_Int32)ADOS::getField(pRecordset,nPos++).get_Value();
                if ( aInfo->eType == adWChar && aInfo->aSimpleType.aTypeName == s_sVarChar )
                    aInfo->eType = adVarWChar;
                aInfo->aSimpleType.nType            = (sal_Int16)ADOS::MapADOType2Jdbc(static_cast<DataTypeEnum>(aInfo->eType));
                aInfo->aSimpleType.nPrecision       = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.aLiteralPrefix   = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.aLiteralSuffix   = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.aCreateParams    = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.bNullable        = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.bCaseSensitive   = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.nSearchType      = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.bUnsigned        = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.bCurrency        = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.bAutoIncrement   = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.aLocalTypeName   = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.nMinimumScale    = ADOS::getField(pRecordset,nPos++).get_Value();
                aInfo->aSimpleType.nMaximumScale    = ADOS::getField(pRecordset,nPos++).get_Value();
                if ( adCurrency == aInfo->eType && !aInfo->aSimpleType.nMaximumScale)
                {
                    aInfo->aSimpleType.nMinimumScale = 4;
                    aInfo->aSimpleType.nMaximumScale = 4;
                }
                aInfo->aSimpleType.nNumPrecRadix    = ADOS::getField(pRecordset,nPos++).get_Value();
                // Now that we have the type info, save it
                // in the Hashtable if we don't already have an
                // entry for this SQL type.

                m_aTypeInfo.insert(OTypeInfoMap::value_type(aInfo->eType,aInfo));
            }
            while ( SUCCEEDED(pRecordset->MoveNext()) );
        }
        pRecordset->Release();
    }
}
//------------------------------------------------------------------------------
void OConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OConnection_BASE::disposing();

    m_bClosed   = sal_True;
    m_xMetaData = ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData>();
    m_xCatalog  = ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbcx::XTablesSupplier>();
    m_pDriver   = NULL;

    m_pAdoConnection->Close();

    OTypeInfoMap::iterator aIter = m_aTypeInfo.begin();
    for (; aIter != m_aTypeInfo.end(); ++aIter)
        delete aIter->second;

    m_aTypeInfo.clear();

    delete m_pAdoConnection;
    m_pAdoConnection = NULL;

    dispose_ChildImpl();
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL OConnection::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw (::com::sun::star::uno::RuntimeException)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ?
            reinterpret_cast< sal_Int64 >( this )
                :
            OConnection_BASE::getSomething(rId);
}
// -----------------------------------------------------------------------------
Sequence< sal_Int8 > OConnection::getUnoTunnelImplementationId()
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
// -----------------------------------------------------------------------------
const OExtendedTypeInfo* OConnection::getTypeInfoFromType(const OTypeInfoMap& _rTypeInfo,
                           DataTypeEnum _nType,
                           const ::rtl::OUString& _sTypeName,
                           sal_Int32 _nPrecision,
                           sal_Int32 _nScale,
                           sal_Bool& _brForceToType)
{
    const OExtendedTypeInfo* pTypeInfo = NULL;
    _brForceToType = sal_False;
    // search for type
    ::std::pair<OTypeInfoMap::const_iterator, OTypeInfoMap::const_iterator> aPair = _rTypeInfo.equal_range(_nType);
    OTypeInfoMap::const_iterator aIter = aPair.first;
    if(aIter != _rTypeInfo.end()) // compare with end is correct here
    {
        for(;aIter != aPair.second;++aIter)
        {
            // search the best matching type
            OExtendedTypeInfo* pInfo = aIter->second;
    #ifdef DBG_UTIL
            ::rtl::OUString sDBTypeName = pInfo->aSimpleType.aTypeName;
            sal_Int32       nDBTypePrecision = pInfo->aSimpleType.nPrecision;   (void)nDBTypePrecision;
            sal_Int32       nDBTypeScale = pInfo->aSimpleType.nMaximumScale;    (void)nDBTypeScale;
            sal_Int32       nAdoType = pInfo->eType;                            (void)nAdoType;
    #endif
            if  (   (   !_sTypeName.getLength()
                    ||  (pInfo->aSimpleType.aTypeName.equalsIgnoreAsciiCase(_sTypeName))
                    )
                &&  (pInfo->aSimpleType.nPrecision      >= _nPrecision)
                &&  (pInfo->aSimpleType.nMaximumScale   >= _nScale)

                )
                break;
        }

        if (aIter == aPair.second)
        {
            for(aIter = aPair.first; aIter != aPair.second; ++aIter)
            {
                // search the best matching type (now comparing the local names)
                if  (   (aIter->second->aSimpleType.aLocalTypeName.equalsIgnoreAsciiCase(_sTypeName))
                    &&  (aIter->second->aSimpleType.nPrecision      >= _nPrecision)
                    &&  (aIter->second->aSimpleType.nMaximumScale   >= _nScale)
                    )
                {
// we can not assert here because we could be in d&d
/*
                    OSL_FAIL((  ::rtl::OString("getTypeInfoFromType: assuming column type ")
                        +=  ::rtl::OString(aIter->second->aTypeName.getStr(), aIter->second->aTypeName.getLength(), osl_getThreadTextEncoding())
                        +=  ::rtl::OString("\" (expected type name ")
                        +=  ::rtl::OString(_sTypeName.getStr(), _sTypeName.getLength(), osl_getThreadTextEncoding())
                        +=  ::rtl::OString(" matches the type's local name).")).getStr());
*/
                    break;
                }
            }
        }

        if (aIter == aPair.second)
        {   // no match for the names, no match for the local names
            // -> drop the precision and the scale restriction, accept any type with the property
            // type id (nType)

            // we can not assert here because we could be in d&d
            pTypeInfo = aPair.first->second;
            _brForceToType = sal_True;
        }
        else
            pTypeInfo = aIter->second;
    }
    else if ( _sTypeName.getLength() )
    {
        ::comphelper::TStringMixEqualFunctor aCase(sal_False);
        // search for typeinfo where the typename is equal _sTypeName
        OTypeInfoMap::const_iterator aFind = ::std::find_if(_rTypeInfo.begin(),
                                                            _rTypeInfo.end(),
                                                            ::o3tl::compose1(
                                                                ::std::bind2nd(aCase, _sTypeName),
                                                                ::o3tl::compose1(
                                                                    ::std::mem_fun(&OExtendedTypeInfo::getDBName),
                                                                    ::o3tl::select2nd<OTypeInfoMap::value_type>())
                                                                )
                                                            );
        if(aFind != _rTypeInfo.end())
            pTypeInfo = aFind->second;
    }

// we can not assert here because we could be in d&d
//  OSL_ENSURE(pTypeInfo, "getTypeInfoFromType: no type info found for this type!");
    return pTypeInfo;
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
