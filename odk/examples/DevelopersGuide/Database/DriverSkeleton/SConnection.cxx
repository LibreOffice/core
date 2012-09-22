/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include "SConnection.hxx"

#include "SDatabaseMetaData.hxx"
#include "SDriver.hxx"
#include "SStatement.hxx"
#include "SPreparedStatement.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

using namespace connectivity::skeleton;

//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
// --------------------------------------------------------------------------------
OConnection::OConnection(SkeletonDriver*    _pDriver)
                         : OSubComponent<OConnection, OConnection_BASE>((::cppu::OWeakObject*)_pDriver, this),
                         OMetaConnection_BASE(m_aMutex),
                         m_pDriver(_pDriver),
                         m_bClosed(sal_False),
                         m_xMetaData(NULL),
                         m_bUseCatalog(sal_False),
                         m_bUseOldDateFormat(sal_False)
{
    m_pDriver->acquire();
}
//-----------------------------------------------------------------------------
OConnection::~OConnection()
{
    if(!isClosed())
        close();
    m_pDriver->release();
    m_pDriver = NULL;
}
//-----------------------------------------------------------------------------
void SAL_CALL OConnection::release() throw()
{
    relase_ChildImpl();
}
// -----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void OConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException)
{
    osl_atomic_increment( &m_refCount );

    // some example code how to get the information out of the sequence

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aDSN("DSN="), aUID, aPWD, aSysDrvSettings;
    aDSN += url.copy(nLen+1);

    const char* pUser       = "user";
    const char* pTimeout    = "Timeout";
    const char* pSilent     = "Silent";
    const char* pPwd        = "password";
    const char* pUseCatalog = "UseCatalog";
    const char* pSysDrv     = "SystemDriverSettings";

    sal_Int32 nTimeout = 20;
    sal_Bool bSilent = sal_True;
    const PropertyValue *pBegin = info.getConstArray();
    const PropertyValue *pEnd   = pBegin + info.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!pBegin->Name.compareToAscii(pTimeout))
            pBegin->Value >>= nTimeout;
        else if(!pBegin->Name.compareToAscii(pSilent))
            pBegin->Value >>= bSilent;
        else if(!pBegin->Name.compareToAscii(pUser))
        {
            pBegin->Value >>= aUID;
            aDSN = aDSN + ::rtl::OUString(";UID=") + aUID;
        }
        else if(!pBegin->Name.compareToAscii(pPwd))
        {
            pBegin->Value >>= aPWD;
            aDSN = aDSN + ::rtl::OUString(";PWD=") + aPWD;
        }
        else if(!pBegin->Name.compareToAscii(pUseCatalog))
        {
            pBegin->Value >>= m_bUseCatalog;
        }
        else if(!pBegin->Name.compareToAscii(pSysDrv))
        {
            pBegin->Value >>= aSysDrvSettings;
            aDSN += ::rtl::OUString(";");
            aDSN += aSysDrvSettings;
        }
    }
    m_sUser = aUID;

    osl_atomic_decrement( &m_refCount );
}
// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.skeleton.OConnection", "com.sun.star.sdbc.Connection")

// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // create a statement
    // the statement can only be executed once
    Reference< XStatement > xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // the pre
    if(m_aTypeInfo.empty())
        buildTypeInfo();

    // create a statement
    // the statement can only be executed more than once
    Reference< XPreparedStatement > xReturn = new OPreparedStatement(this,m_aTypeInfo,_sSql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // not implemented yet :-) a task to do
    return NULL;
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::nativeSQL( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // when you need to transform SQL92 to you driver specific you can do it here

    return _sSql;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    // here you  have to set your commit mode please have a look at the jdbc documentation to get a clear explanation
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    // you have to distinguish which if you are in autocommit mode or not
    // at normal case true should be fine here

    return sal_True;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // when you database does support transactions you should commit here
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    // same as commit but for the other case
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // just simple -> we are close when we are disposed taht means someone called dispose(); (XComponent)
    return OConnection_BASE::rBHelper.bDisposed;
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // here we have to create the class with biggest interface
    // The answer is 42 :-)
    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new ODatabaseMetaData(this); // need the connection because it can return it
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // set you connection to readonly
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // return if your connection to readonly
    return sal_False;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setCatalog( const ::rtl::OUString& catalog ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // if your database doesn't work with catalogs you go to next method otherwise you kjnow what to do
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    // return your current catalog
    return ::rtl::OUString();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // set your isolation level
    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
    return TransactionIsolation::NONE;
}
// --------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // if your driver has special database types you can return it here

    return NULL;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    // the other way around
}
// --------------------------------------------------------------------------------
// XCloseable
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException)
{
    // we just dispose us
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
    // when you collected some warnings -> return it
    return Any();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    // you should clear your collected warnings here
}
//--------------------------------------------------------------------
void OConnection::buildTypeInfo() throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XResultSet> xRs = getMetaData ()->getTypeInfo ();
    Reference< XRow> xRow(xRs,UNO_QUERY);
    // Information for a single SQL type

    // Loop on the result set until we reach end of file

    while (xRs->next ())
    {
        OTypeInfo aInfo;
        aInfo.aTypeName         = xRow->getString   (1);
        aInfo.nType             = xRow->getShort    (2);
        aInfo.nPrecision        = xRow->getInt      (3);
        aInfo.aLiteralPrefix    = xRow->getString   (4);
        aInfo.aLiteralSuffix    = xRow->getString   (5);
        aInfo.aCreateParams     = xRow->getString   (6);
        aInfo.bNullable         = xRow->getBoolean  (7) == ColumnValue::NULLABLE;
        aInfo.bCaseSensitive    = xRow->getBoolean  (8);
        aInfo.nSearchType       = xRow->getShort    (9);
        aInfo.bUnsigned         = xRow->getBoolean  (10);
        aInfo.bCurrency         = xRow->getBoolean  (11);
        aInfo.bAutoIncrement    = xRow->getBoolean  (12);
        aInfo.aLocalTypeName    = xRow->getString   (13);
        aInfo.nMinimumScale     = xRow->getShort    (14);
        aInfo.nMaximumScale     = xRow->getShort    (15);
        aInfo.nNumPrecRadix     = (sal_Int16)xRow->getInt(18);



        // Now that we have the type info, save it
        // in the Hashtable if we don't already have an
        // entry for this SQL type.

        m_aTypeInfo.push_back(aInfo);
    }

    // Close the result set/statement.

    Reference< XCloseable> xClose(xRs,UNO_QUERY);
    xClose->close();
}
//------------------------------------------------------------------------------
void OConnection::disposing()
{
    // we noticed that we should be destroied in near future so we have to dispose our statements
    ::osl::MutexGuard aGuard(m_aMutex);

    for (OWeakRefArray::iterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aStatements.clear();

    m_bClosed   = sal_True;
    m_xMetaData = ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData>();

    dispose_ChildImpl();
    OConnection_BASE::disposing();
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
