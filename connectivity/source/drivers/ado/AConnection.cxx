/*************************************************************************
 *
 *  $RCSfile: AConnection.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-24 16:11:26 $
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

#ifndef _CONNECTIVITY_ADO_ACONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ADATABASEMETADATA_HXX_
#include "ado/ADatabaseMetaData.hxx"
#endif

#ifndef _CONNECTIVITY_RESOURCE_HRC_
#include "Resource.hrc"
#endif
#ifndef _CONNECTIVITY_MODULECONTEXT_HXX_
#include "ModuleContext.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ADRIVER_HXX_
#include "ado/ADriver.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ASTATEMENT_HXX_
#include "ado/AStatement.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ACALLABLESTATEMENT_HXX_
#include "ado/ACallableStatement.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_APREPAREDSTATEMENT_HXX_
#include "ado/APreparedStatement.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_TRANSACTIONISOLATION_HPP_
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

using namespace connectivity::ado;
using namespace connectivity::dbtools;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OConnection,"com.sun.star.sdbcx.AConnection","com.sun.star.sdbc.Connection");
// --------------------------------------------------------------------------------
OConnection::OConnection(const ::rtl::OUString& url, const Sequence< PropertyValue >& info,
                         ODriver*   _pDriver) throw(SQLException, RuntimeException)
                         : OConnection_BASE(m_aMutex),
                         OSubComponent<OConnection>((::cppu::OWeakObject*)_pDriver, this),
                         m_bClosed(sal_False),
                         m_xMetaData(NULL),
                         m_xCatalog(NULL),
                         m_pDriver(_pDriver),
                         m_pAdoConnection(NULL),
                         m_bAutocommit(sal_True)
{
    ModuleContext::AddRef();

    osl_incrementInterlockedCount( &m_refCount );

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
        pIUnknown->AddRef();

        ADOConnection *pCon         = NULL;
        hr = pIUnknown->CreateInstanceLic(  pOuter,
                                            NULL,
                                            ADOS::IID_ADOCONNECTION_21,
                                            ADOS::GetKeyStr(),
                                            (void**) &pCon);

        if( !FAILED( hr ) )
        {
            m_pAdoConnection = new WpADOConnection(pCon);
            // Class Factory is no longer needed


        }
        pIUnknown->Release();
    }

    osl_decrementInterlockedCount( &m_refCount );
}
//-----------------------------------------------------------------------------
OConnection::~OConnection()
{
    delete m_pAdoConnection;
    ModuleContext::ReleaseRef();
}
//-----------------------------------------------------------------------------
void OConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)
{
    osl_incrementInterlockedCount( &m_refCount );

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aDSN(url.copy(nLen+1)),aUID,aPWD;

    sal_Int32 nTimeout = 20;
    sal_Bool bSilent = sal_True;
    const PropertyValue *pBegin = info.getConstArray();
    const PropertyValue *pEnd   = pBegin + info.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!pBegin->Name.compareToAscii("Timeout"))
            pBegin->Value >>= nTimeout;
        else if(!pBegin->Name.compareToAscii("Silent"))
            pBegin->Value >>= bSilent;
        else if(!pBegin->Name.compareToAscii("user"))
            pBegin->Value >>= aUID;
        else if(!pBegin->Name.compareToAscii("password"))
            pBegin->Value >>= aPWD;
    }

    if(m_pAdoConnection)
    {
        if(m_pAdoConnection->Open(aDSN,aUID,aPWD,adConnectUnspecified))
            m_pAdoConnection->PutCommandTimeout(nTimeout);
        else
            ADOS::ThrowException(*m_pAdoConnection,*this);
        if(m_pAdoConnection->get_State() != adStateOpen)
            throw SQLException();

        ADOProperties* pProps=m_pAdoConnection->get_Properties();
        if(pProps)
        {
            pProps->AddRef();
            ADOProperty* pProp = NULL;
            pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Jet OLEDB:ODBC Parsing")),&pProp);
            WpADOProperty aProp(pProp);
            if(pProp)
            {
                aProp.PutValue(OLEVariant(VARIANT_TRUE));
                OLEVariant aVar = aProp.GetValue();
            }
            pProps->Release();
        }
        //bErg = TRUE;
    }
    else
        throw SQLException();

    osl_decrementInterlockedCount( &m_refCount );
}
//-----------------------------------------------------------------------------
void SAL_CALL OConnection::release() throw(RuntimeException)
{
    relase_ChildImpl();
}
// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();
    OStatement* pStmt = new OStatement(this);
    Reference< XStatement > xStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OPreparedStatement* pStmt = new OPreparedStatement(this,m_aTypeInfo,sql);
    Reference< XPreparedStatement > xPStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xPStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OCallableStatement* pStmt = new OCallableStatement(this,m_aTypeInfo,sql);
    Reference< XPreparedStatement > xPStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xPStmt;
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::nativeSQL( const ::rtl::OUString& _sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ::rtl::OUString sql = _sql;
    ADOProperties* pProps=m_pAdoConnection->get_Properties();
    if(pProps)
    {
        pProps->AddRef();
        ADOProperty* pProp = NULL;
        pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Jet OLEDB:ODBC Parsing")),&pProp);
        WpADOProperty aProp(pProp);
        if(pProp)
        {
            pProp->put_Value(OLEVariant(VARIANT_TRUE));
            WpADOCommand aCommand;
            aCommand.Create();
            aCommand.put_ActiveConnection((IDispatch*)*m_pAdoConnection);
            aCommand.put_CommandText(sql);
            sql = aCommand.get_CommandText();
        }
        pProps->Release();
    }

    return sql;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

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
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_bAutocommit;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_pAdoConnection->CommitTrans();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

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
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_xMetaData.is())
        m_xMetaData = new ODatabaseMetaData(this);

    return m_xMetaData;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_pAdoConnection->put_Mode(adModeRead);
    ADOS::ThrowException(*m_pAdoConnection,*this);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_pAdoConnection->get_Mode() == adModeRead;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setCatalog( const ::rtl::OUString& catalog ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_pAdoConnection->PutDefaultDatabase(catalog);
    ADOS::ThrowException(*m_pAdoConnection,*this);
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_pAdoConnection->GetDefaultDatabase();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

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
            OSL_ENSHURE(0,"OConnection::setTransactionIsolation invalid level");
            return;
    }
    m_pAdoConnection->put_IsolationLevel(eIso);
    ADOS::ThrowException(*m_pAdoConnection,*this);
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

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
            OSL_ENSHURE(0,"OConnection::setTransactionIsolation invalid level");
    }
    ADOS::ThrowException(*m_pAdoConnection,*this);
    return nRet;
}
// --------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return NULL;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
}
// --------------------------------------------------------------------------------
// XCloseable
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (OConnection_BASE::rBHelper.bDisposed)
            throw DisposedException();
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

    Reference< XResultSet> xRs = getMetaData ()->getTypeInfo ();
    Reference< XRow> xRow(xRs,UNO_QUERY);
    // Information for a single SQL type

    // Loop on the result set until we reach end of file

    sal_Bool more = xRs->next ();
    while (more)
    {
        OTypeInfo aInfo;
        aInfo.aTypeName         = xRow->getString (1);
        aInfo.aLiteralPrefix    = xRow->getString (4);
        aInfo.aLiteralSuffix    = xRow->getString (5);
        aInfo.aCreateParams     = xRow->getString (6);
        aInfo.aLocalTypeName    = xRow->getString (13);

        aInfo.nPrecision        = xRow->getInt (3);
        aInfo.nMaximumScale     = xRow->getShort (15);
        aInfo.nMinimumScale     = xRow->getShort (14);
        aInfo.nType             = xRow->getShort (2);
        aInfo.nSearchType       = xRow->getShort (9);
        aInfo.nNumPrecRadix     = xRow->getInt (18);

        aInfo.bCurrency         = xRow->getBoolean (11);
        aInfo.bAutoIncrement    = xRow->getBoolean (12);
        aInfo.bNullable         = xRow->getBoolean (7) == ColumnValue::NULLABLE;
        aInfo.bCaseSensitive    = xRow->getBoolean (8);
        aInfo.bUnsigned         = xRow->getBoolean (10);

        // Now that we have the type info, save it
        // in the Hashtable if we don't already have an
        // entry for this SQL type.

        m_aTypeInfo.push_back(aInfo);
        more = xRs->next ();
    }

    // Close the result set/statement.

    Reference< XCloseable>(xRs,UNO_QUERY)->close();
}
//------------------------------------------------------------------------------
void OConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    //  m_aTables.disposing();
    for (OWeakRefArray::iterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aStatements.clear();

    Reference< XComponent > xComp2(m_xCatalog.get(), UNO_QUERY);
    if(xComp2.is())
        xComp2->dispose();

    m_bClosed = sal_True;
    m_xMetaData = NULL;

    m_pAdoConnection->Close();

    dispose_ChildImpl();
    OConnection_BASE::disposing();
}


