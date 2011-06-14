/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#ifdef SYSTEM_ODBC_HEADERS
#include <sqltypes.h>
#else
#include <odbc/sqltypes.h>
#endif

#include <unotools/tempfile.hxx>
#include <sal/macros.h>
#include "adabas/BDriver.hxx"
#include "adabas/BConnection.hxx"
#include "odbc/OFunctions.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "odbc/OTools.hxx"
#include "connectivity/dbexception.hxx"
#include "TConnection.hxx"
#include "diagnose_ex.h"

#include <osl/process.h>
#include <unotools/ucbhelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include "resource/adabas_res.hrc"
#include "resource/sharedresources.hxx"


#include <memory>
#include <sys/stat.h>

#if defined(UNX)
const char sNewLine = '\012';
#else
const char sNewLine[] = "\015\012"; // \015\012 and not \n
#endif
#define ADABAS_DB_11            "11.02.00"
#define ADABAS_KERNEL_11        "11.02"
#define ADABAS_DB_12            "12.01.00"
#define ADABAS_KERNEL_12        "12.01"
#define CURRENT_DB_VERSION      "13.01.00"
#define CURRENT_KERNEL_VERSION  "13.01"

#define OPROCESS_ADABAS     (osl_Process_HIDDEN | osl_Process_WAIT | osl_Process_SEARCHPATH)
#define OPROCESS_ADABAS_DBG (osl_Process_WAIT | osl_Process_SEARCHPATH)


using namespace connectivity;
namespace connectivity
{
    namespace adabas
    {
        // extern declaration of the function pointer
    extern T3SQLAllocHandle pODBC3SQLAllocHandle;
    extern T3SQLConnect pODBC3SQLConnect;
    extern T3SQLDriverConnect pODBC3SQLDriverConnect;
    extern T3SQLBrowseConnect pODBC3SQLBrowseConnect;
    extern T3SQLDataSources pODBC3SQLDataSources;
    extern T3SQLDrivers pODBC3SQLDrivers;
    extern T3SQLGetInfo pODBC3SQLGetInfo;
    extern T3SQLGetFunctions pODBC3SQLGetFunctions;
    extern T3SQLGetTypeInfo pODBC3SQLGetTypeInfo;
    extern T3SQLSetConnectAttr pODBC3SQLSetConnectAttr;
    extern T3SQLGetConnectAttr pODBC3SQLGetConnectAttr;
    extern T3SQLSetEnvAttr pODBC3SQLSetEnvAttr;
    extern T3SQLGetEnvAttr pODBC3SQLGetEnvAttr;
    extern T3SQLSetStmtAttr pODBC3SQLSetStmtAttr;
    extern T3SQLGetStmtAttr pODBC3SQLGetStmtAttr;
    //extern T3SQLSetDescField pODBC3SQLSetDescField;
    //extern T3SQLGetDescField pODBC3SQLGetDescField;
    //extern T3SQLGetDescRec pODBC3SQLGetDescRec;
    //extern T3SQLSetDescRec pODBC3SQLSetDescRec;
    extern T3SQLPrepare pODBC3SQLPrepare;
    extern T3SQLBindParameter pODBC3SQLBindParameter;
    //extern T3SQLGetCursorName pODBC3SQLGetCursorName;
    extern T3SQLSetCursorName pODBC3SQLSetCursorName;
    extern T3SQLExecute pODBC3SQLExecute;
    extern T3SQLExecDirect pODBC3SQLExecDirect;
    //extern T3SQLNativeSql pODBC3SQLNativeSql;
    extern T3SQLDescribeParam pODBC3SQLDescribeParam;
    extern T3SQLNumParams pODBC3SQLNumParams;
    extern T3SQLParamData pODBC3SQLParamData;
    extern T3SQLPutData pODBC3SQLPutData;
    extern T3SQLRowCount pODBC3SQLRowCount;
    extern T3SQLNumResultCols pODBC3SQLNumResultCols;
    extern T3SQLDescribeCol pODBC3SQLDescribeCol;
    extern T3SQLColAttribute pODBC3SQLColAttribute;
    extern T3SQLBindCol pODBC3SQLBindCol;
    extern T3SQLFetch pODBC3SQLFetch;
    extern T3SQLFetchScroll pODBC3SQLFetchScroll;
    extern T3SQLGetData pODBC3SQLGetData;
    extern T3SQLSetPos pODBC3SQLSetPos;
    extern T3SQLBulkOperations pODBC3SQLBulkOperations;
    extern T3SQLMoreResults pODBC3SQLMoreResults;
    //extern T3SQLGetDiagField pODBC3SQLGetDiagField;
    extern T3SQLGetDiagRec pODBC3SQLGetDiagRec;
    extern T3SQLColumnPrivileges pODBC3SQLColumnPrivileges;
    extern T3SQLColumns pODBC3SQLColumns;
    extern T3SQLForeignKeys pODBC3SQLForeignKeys;
    extern T3SQLPrimaryKeys pODBC3SQLPrimaryKeys;
    extern T3SQLProcedureColumns pODBC3SQLProcedureColumns;
    extern T3SQLProcedures pODBC3SQLProcedures;
    extern T3SQLSpecialColumns pODBC3SQLSpecialColumns;
    extern T3SQLStatistics pODBC3SQLStatistics;
    extern T3SQLTablePrivileges pODBC3SQLTablePrivileges;
    extern T3SQLTables pODBC3SQLTables;
    extern T3SQLFreeStmt pODBC3SQLFreeStmt;
    extern T3SQLCloseCursor pODBC3SQLCloseCursor;
    extern T3SQLCancel pODBC3SQLCancel;
    extern T3SQLEndTran pODBC3SQLEndTran;
    extern T3SQLDisconnect pODBC3SQLDisconnect;
    extern T3SQLFreeHandle pODBC3SQLFreeHandle;
    extern T3SQLGetCursorName pODBC3SQLGetCursorName;
    extern T3SQLNativeSql pODBC3SQLNativeSql;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace utl;
using namespace osl;
using namespace ::dbtools;

    sal_Bool LoadFunctions(oslModule pODBCso);
    sal_Bool LoadLibrary_ADABAS(::rtl::OUString &_rPath);
    // --------------------------------------------------------------------------------
void ODriver::fillInfo(const Sequence< PropertyValue >& info, TDatabaseStruct& _rDBInfo)
{
    const PropertyValue* pIter = info.getConstArray();
    const PropertyValue* pEnd = pIter + info.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DatabaseName"))))
        {
            pIter->Value >>= _rDBInfo.sDBName;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlUser"))))
        {
            pIter->Value >>= _rDBInfo.sControlUser;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlPassword"))))
        {
            pIter->Value >>= _rDBInfo.sControlPassword;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataCacheSizeIncrement"))))
            pIter->Value >>= _rDBInfo.nDataIncrement;
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShutdownDatabase"))))
            pIter->Value >>= _rDBInfo.bShutDown;
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("User"))))
        {
            pIter->Value >>= _rDBInfo.sSysUser;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Password"))))
        {
            pIter->Value >>= _rDBInfo.sSysPassword;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DomainPassword"))))
        {
            pIter->Value >>= _rDBInfo.sDomainPassword;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CacheSize"))))
        {
            pIter->Value >>= _rDBInfo.sCacheSize;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RestoreDatabase"))))
        {
            pIter->Value >>= _rDBInfo.bRestoreDatabase;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Backup"))))
        {
            pIter->Value >>= _rDBInfo.sBackupFile;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataDevSpace"))))
        {
            pIter->Value >>= _rDBInfo.sDataDevName;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYSDEVSPACE"))))
        {
            pIter->Value >>= _rDBInfo.sSysDevSpace;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TRANSACTION_LOG"))))
        {
            pIter->Value >>= _rDBInfo.sTransLogName;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataDevSize"))))
        {
            pIter->Value >>= _rDBInfo.nDataSize;
        }
        else if(pIter->Name.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LogDevSize"))))
        {
            pIter->Value >>= _rDBInfo.nLogSize;
        }
    }
}


// --------------------------------------------------------------------------------
ODriver::ODriver(const Reference< XMultiServiceFactory >& _rxFactory) : ODBCDriver(_rxFactory)
{
    osl_incrementInterlockedCount(&m_refCount);
    fillEnvironmentVariables();
    Reference< XComponent >  xComponent(m_xORB, UNO_QUERY);
    if (xComponent.is())
    {
        Reference< ::com::sun::star::lang::XEventListener> xEvtL((::cppu::OWeakObject*)this,UNO_QUERY);
        xComponent->addEventListener(xEvtL);
    }
    osl_decrementInterlockedCount(&m_refCount);
}
//------------------------------------------------------------------------------
void ODriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ODriver_BASE::disposing();
    Reference< XComponent >  xComponent(m_xORB, UNO_QUERY);
    if (xComponent.is())
    {
        Reference< XEventListener> xEvtL((::cppu::OWeakObject*)this,UNO_QUERY);
        xComponent->removeEventListener(xEvtL);
    }
    m_xORB.clear();
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ODriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.sdbcx.adabas.ODriver"));
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.Driver"));
    aSNS[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.Driver"));
    return aSNS;
}
//------------------------------------------------------------------
::rtl::OUString SAL_CALL ODriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
sal_Bool SAL_CALL ODriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    const Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------
Any SAL_CALL ODriver::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType, static_cast<XDataDefinitionSupplier*>(this));
    if ( !aRet.hasValue() )
        aRet = ODriver_BASE::queryInterface(rType);
    return aRet.hasValue() ? aRet : ODriver_BASE2::queryInterface(rType);
}
//------------------------------------------------------------------
Reference< XInterface >  SAL_CALL ODriver_CreateInstance(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFac) throw( Exception )
{
    return *(new ODriver(_rxFac));
}
// -----------------------------------------------------------------------------
void SAL_CALL ODriver::disposing( const EventObject& Source ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if(m_xORB.is() && Reference< XMultiServiceFactory >(Source.Source,UNO_QUERY) == m_xORB)
    {
        TDatabaseMap::iterator aIter = m_aDatabaseMap.begin();
        for(;aIter != m_aDatabaseMap.end();++aIter)
        {
            if(aIter->second.bShutDown)
            {
                ::rtl::OUString sName;
                if(getDBName(aIter->first,sName))
                {
                    XUTIL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SHUTDOWN QUICK")),sName,aIter->second.sControlUser,aIter->second.sControlPassword);
                    X_STOP(sName);
                }
            }
        }
    m_xORB.clear();
    }
}
// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
        return NULL;

    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
        throw DisposedException();

    TDatabaseStruct aDBInfo;
    aDBInfo.bShutDown = sal_False;
    fillInfo(info,aDBInfo);
    aDBInfo.sControlUser = aDBInfo.sControlUser.toAsciiUpperCase();
    aDBInfo.sControlPassword = aDBInfo.sControlPassword.toAsciiUpperCase();
    aDBInfo.sSysUser = aDBInfo.sSysUser.toAsciiUpperCase();
    aDBInfo.sSysPassword = aDBInfo.sSysPassword.toAsciiUpperCase();


    TDatabaseMap::iterator aFind = m_aDatabaseMap.find(url);
    if(aFind == m_aDatabaseMap.end()) // only when not found yet
        m_aDatabaseMap[url] = aDBInfo;
    else
    {
        if(aFind->second.bShutDown != aDBInfo.bShutDown)
            aFind->second.bShutDown &= aDBInfo.bShutDown;
    }

    ::rtl::OUString sName;
    if(aDBInfo.sControlPassword.getLength() && aDBInfo.sControlUser.getLength() && getDBName(url,sName))
    {
        // check if we have to add a new data dev space
        checkAndInsertNewDevSpace(sName,aDBInfo);

        convertOldVersion(sName,aDBInfo);
        // check if we must restart the database
        checkAndRestart(sName,aDBInfo);
    }


    if(!m_pDriverHandle)
    {
        ::rtl::OUString aPath;
        if(!EnvironmentHandle(aPath))
            throw SQLException(aPath,*this,::rtl::OUString(),1000,Any());
    }
    OAdabasConnection* pCon = new OAdabasConnection(m_pDriverHandle,this);
    Reference< XConnection > xCon = pCon;
    SQLRETURN nSQLRETURN = pCon->Construct(url,info);

    if (nSQLRETURN == SQL_ERROR || nSQLRETURN == SQL_NO_DATA)
    {
        odbc::OTools::ThrowException(pCon,nSQLRETURN,pCon->getConnection(),SQL_HANDLE_DBC,*this);
    }

    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// -----------------------------------------------------------------------------
sal_Bool ODriver::getDBName(const ::rtl::OUString& _rName,::rtl::OUString& sDBName) const
{
    sDBName = ::rtl::OUString();
    ::rtl::OUString sName = _rName.copy(12);
    sal_Int32 nPos = sName.indexOf(':');
    if(nPos != -1 && nPos < 1)
        sDBName = sName.copy(1);
    return (nPos != -1 && nPos < 1);
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL ODriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    return (!url.compareTo(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:adabas:")),12));
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL ODriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& /*info*/) throw(SQLException, RuntimeException)
{
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShutdownDatabase"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Shut down service when closing."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlUser"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Control user name."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"))
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlPassword"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Control password."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataCacheSizeIncrement"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Data increment (MB)."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet of the database."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
    }

    SharedResources aResources;
    const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
    return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// -----------------------------------------------------------------------------
// XCreateCatalog
void SAL_CALL ODriver::createCatalog( const Sequence< PropertyValue >& info ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
        throw DisposedException();

    try
    {
        TDatabaseStruct aDBInfo;
        fillInfo(info,aDBInfo);

        ::rtl::OUString envVar(RTL_CONSTASCII_USTRINGPARAM("DBSERVICE"));
        ::rtl::OUString envData(RTL_CONSTASCII_USTRINGPARAM("0"));
        osl_setEnvironment(envVar.pData, envData.pData);

        m_sDbRunDir = m_sDbWorkURL + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/wrk/")) + aDBInfo.sDBName;
        String sTemp;
        LocalFileHelper::ConvertURLToPhysicalName(m_sDbRunDir,sTemp);
        m_sDbRunDir = sTemp;

        createNeededDirs(aDBInfo.sDBName);
        if(CreateFiles(aDBInfo))
        {
            ::connectivity::SharedResources aResources;
            const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(
                    STR_NO_DISK_SPACE,
                    "$filename$",aDBInfo.sDBName
                 ) );
            ::dbtools::throwGenericSQLException(sError,*this);
        } // if(CreateFiles(aDBInfo))

        createDb(aDBInfo);
    }
    catch( SQLException&)
    {
        throw;
    }
    catch(Exception&)
    {
        throw SQLException();
    }

}
// -----------------------------------------------------------------------------
// XDropCatalog
void SAL_CALL ODriver::dropCatalog( const ::rtl::OUString& /*catalogName*/, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ::dbtools::throwFeatureNotImplementedException( "!XDropCatalog::dropCatalog", *this );
}
//-----------------------------------------------------------------------------
// ODBC Environment (common for all Connections):
SQLHANDLE ODriver::EnvironmentHandle(::rtl::OUString &_rPath)
{
    // Has an Environment already been created (for this Instance)?
    if (!m_pDriverHandle)
    {
        SQLHANDLE h = SQL_NULL_HANDLE;
        // allocate Environment

        // load ODBC-DLL now:
        if (! LoadLibrary_ADABAS(_rPath))
            return SQL_NULL_HANDLE;

        if (N3SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&h) != SQL_SUCCESS)
            return SQL_NULL_HANDLE;

        // Save in global Structure...
        m_pDriverHandle = h;
        SQLRETURN nError = N3SQLSetEnvAttr(h, SQL_ATTR_ODBC_VERSION,(SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER);
        OSL_UNUSED( nError );
        //N3SQLSetEnvAttr(h, SQL_ATTR_CONNECTION_POOLING,(SQLPOINTER) SQL_CP_ONE_PER_HENV, SQL_IS_INTEGER);
    }

    return m_pDriverHandle;
}
// --------------------------------------------------------------------------------
// XDataDefinitionSupplier
Reference< XTablesSupplier > SAL_CALL ODriver::getDataDefinitionByConnection( const Reference< ::com::sun::star::sdbc::XConnection >& connection ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XTablesSupplier > xTab;
    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(connection,UNO_QUERY);
    if ( xTunnel.is() )
    {

        OAdabasConnection* pConnection = NULL;
        OAdabasConnection* pSearchConnection = reinterpret_cast< OAdabasConnection* >( xTunnel->getSomething(OAdabasConnection::getUnoTunnelImplementationId()) );
        for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
        {
            if ( (OAdabasConnection*) Reference< XConnection >::query(i->get().get()).get() == pSearchConnection )
            {
                pConnection = pSearchConnection;
                break;
            }
        }


        if ( pConnection )
            xTab = pConnection->createCatalog();
    }
    return xTab;
}

// --------------------------------------------------------------------------------
Reference< XTablesSupplier > SAL_CALL ODriver::getDataDefinitionByURL( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
    {
        SharedResources aResources;
        const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    } // if ( ! acceptsURL(url) )

    return getDataDefinitionByConnection(connect(url,info));
}
// -----------------------------------------------------------------------------
void SAL_CALL ODriver::acquire() throw()
{
    ODriver_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODriver::release() throw()
{
    ODriver_BASE::release();
}
ODriver::~ODriver()
{
}
// -----------------------------------------------------------------------------
oslGenericFunction ODriver::getOdbcFunction(sal_Int32 _nIndex) const
{
    oslGenericFunction pFunction = NULL;
    switch(_nIndex)
    {
        case ODBC3SQLAllocHandle:
            pFunction = (oslGenericFunction)pODBC3SQLAllocHandle;;
            break;
        case ODBC3SQLConnect:
            pFunction = (oslGenericFunction)pODBC3SQLConnect;
            break;
        case ODBC3SQLDriverConnect:
            pFunction = (oslGenericFunction)pODBC3SQLDriverConnect;
            break;
        case ODBC3SQLBrowseConnect:
            pFunction = (oslGenericFunction)pODBC3SQLBrowseConnect;
            break;
        case ODBC3SQLDataSources:
            pFunction = (oslGenericFunction)pODBC3SQLDataSources;
            break;
        case ODBC3SQLDrivers:
            pFunction = (oslGenericFunction)pODBC3SQLDrivers;
            break;
        case ODBC3SQLGetInfo:

            pFunction = (oslGenericFunction)pODBC3SQLGetInfo;
            break;
        case ODBC3SQLGetFunctions:

            pFunction = (oslGenericFunction)pODBC3SQLGetFunctions;
            break;
        case ODBC3SQLGetTypeInfo:

            pFunction = (oslGenericFunction)pODBC3SQLGetTypeInfo;
            break;
        case ODBC3SQLSetConnectAttr:

            pFunction = (oslGenericFunction)pODBC3SQLSetConnectAttr;
            break;
        case ODBC3SQLGetConnectAttr:

            pFunction = (oslGenericFunction)pODBC3SQLGetConnectAttr;
            break;
        case ODBC3SQLSetEnvAttr:

            pFunction = (oslGenericFunction)pODBC3SQLSetEnvAttr;
            break;
        case ODBC3SQLGetEnvAttr:

            pFunction = (oslGenericFunction)pODBC3SQLGetEnvAttr;
            break;
        case ODBC3SQLSetStmtAttr:

            pFunction = (oslGenericFunction)pODBC3SQLSetStmtAttr;
            break;
        case ODBC3SQLGetStmtAttr:

            pFunction = (oslGenericFunction)pODBC3SQLGetStmtAttr;
            break;
        case ODBC3SQLPrepare:

            pFunction = (oslGenericFunction)pODBC3SQLPrepare;
            break;
        case ODBC3SQLBindParameter:

            pFunction = (oslGenericFunction)pODBC3SQLBindParameter;
            break;
        case ODBC3SQLSetCursorName:

            pFunction = (oslGenericFunction)pODBC3SQLSetCursorName;
            break;
        case ODBC3SQLExecute:

            pFunction = (oslGenericFunction)pODBC3SQLExecute;
            break;
        case ODBC3SQLExecDirect:

            pFunction = (oslGenericFunction)pODBC3SQLExecDirect;
            break;
        case ODBC3SQLDescribeParam:

            pFunction = (oslGenericFunction)pODBC3SQLDescribeParam;
            break;
        case ODBC3SQLNumParams:

            pFunction = (oslGenericFunction)pODBC3SQLNumParams;
            break;
        case ODBC3SQLParamData:

            pFunction = (oslGenericFunction)pODBC3SQLParamData;
            break;
        case ODBC3SQLPutData:

            pFunction = (oslGenericFunction)pODBC3SQLPutData;
            break;
        case ODBC3SQLRowCount:

            pFunction = (oslGenericFunction)pODBC3SQLRowCount;
            break;
        case ODBC3SQLNumResultCols:

            pFunction = (oslGenericFunction)pODBC3SQLNumResultCols;
            break;
        case ODBC3SQLDescribeCol:

            pFunction = (oslGenericFunction)pODBC3SQLDescribeCol;
            break;
        case ODBC3SQLColAttribute:

            pFunction = (oslGenericFunction)pODBC3SQLColAttribute;
            break;
        case ODBC3SQLBindCol:

            pFunction = (oslGenericFunction)pODBC3SQLBindCol;
            break;
        case ODBC3SQLFetch:

            pFunction = (oslGenericFunction)pODBC3SQLFetch;
            break;
        case ODBC3SQLFetchScroll:

            pFunction = (oslGenericFunction)pODBC3SQLFetchScroll;
            break;
        case ODBC3SQLGetData:

            pFunction = (oslGenericFunction)pODBC3SQLGetData;
            break;
        case ODBC3SQLSetPos:

            pFunction = (oslGenericFunction)pODBC3SQLSetPos;
            break;
        case ODBC3SQLBulkOperations:

            pFunction = (oslGenericFunction)pODBC3SQLBulkOperations;
            break;
        case ODBC3SQLMoreResults:

            pFunction = (oslGenericFunction)pODBC3SQLMoreResults;
            break;
        case ODBC3SQLGetDiagRec:

            pFunction = (oslGenericFunction)pODBC3SQLGetDiagRec;
            break;
        case ODBC3SQLColumnPrivileges:

            pFunction = (oslGenericFunction)pODBC3SQLColumnPrivileges;
            break;
        case ODBC3SQLColumns:

            pFunction = (oslGenericFunction)pODBC3SQLColumns;
            break;
        case ODBC3SQLForeignKeys:

            pFunction = (oslGenericFunction)pODBC3SQLForeignKeys;
            break;
        case ODBC3SQLPrimaryKeys:

            pFunction = (oslGenericFunction)pODBC3SQLPrimaryKeys;
            break;
        case ODBC3SQLProcedureColumns:

            pFunction = (oslGenericFunction)pODBC3SQLProcedureColumns;
            break;
        case ODBC3SQLProcedures:

            pFunction = (oslGenericFunction)pODBC3SQLProcedures;
            break;
        case ODBC3SQLSpecialColumns:

            pFunction = (oslGenericFunction)pODBC3SQLSpecialColumns;
            break;
        case ODBC3SQLStatistics:

            pFunction = (oslGenericFunction)pODBC3SQLStatistics;
            break;
        case ODBC3SQLTablePrivileges:

            pFunction = (oslGenericFunction)pODBC3SQLTablePrivileges;
            break;
        case ODBC3SQLTables:

            pFunction = (oslGenericFunction)pODBC3SQLTables;
            break;
        case ODBC3SQLFreeStmt:

            pFunction = (oslGenericFunction)pODBC3SQLFreeStmt;
            break;
        case ODBC3SQLCloseCursor:

            pFunction = (oslGenericFunction)pODBC3SQLCloseCursor;
            break;
        case ODBC3SQLCancel:

            pFunction = (oslGenericFunction)pODBC3SQLCancel;
            break;
        case ODBC3SQLEndTran:

            pFunction = (oslGenericFunction)pODBC3SQLEndTran;
            break;
        case ODBC3SQLDisconnect:

            pFunction = (oslGenericFunction)pODBC3SQLDisconnect;
            break;
        case ODBC3SQLFreeHandle:

            pFunction = (oslGenericFunction)pODBC3SQLFreeHandle;
            break;
        case ODBC3SQLGetCursorName:

            pFunction = (oslGenericFunction)pODBC3SQLGetCursorName;
            break;
        case ODBC3SQLNativeSql:

            pFunction = (oslGenericFunction)pODBC3SQLNativeSql;
            break;
        default:
            OSL_FAIL("Function unknown!");
    }
    return pFunction;
}
// -----------------------------------------------------------------------------
void ODriver::createNeededDirs(const ::rtl::OUString& sDBName)
{
    ::rtl::OUString sDbWork,sDBConfig,sTemp;

    if(m_sDbWork.getLength())
    {
        sDbWork = m_sDbWorkURL;
        if(!UCBContentHelper::IsFolder(m_sDbWorkURL))
            UCBContentHelper::MakeFolder(m_sDbWorkURL);

        sDbWork += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
        sDbWork += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("wrk"));
        if(!UCBContentHelper::IsFolder(sDbWork))
            UCBContentHelper::MakeFolder(sDbWork);

        sDbWork += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));

        sTemp = sDbWork;
        sTemp += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("config"));
        if(!UCBContentHelper::IsFolder(sTemp))
            UCBContentHelper::MakeFolder(sTemp);

        sTemp = sDbWork;
        sTemp += sDBName;
        if(!UCBContentHelper::IsFolder(sTemp))
            UCBContentHelper::MakeFolder(sTemp);
    }

    if(m_sDbConfig.getLength())
    {
        sDBConfig = m_sDbConfigURL;
        if(!UCBContentHelper::IsFolder(sDBConfig))
            UCBContentHelper::MakeFolder(sDBConfig);

        sDBConfig += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
        sTemp = sDBConfig;
        sTemp += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("config"));
        if(!UCBContentHelper::IsFolder(sTemp))
            UCBContentHelper::MakeFolder(sTemp);

        sTemp += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
        sTemp += sDBName;
        if(UCBContentHelper::Exists(sTemp))
            UCBContentHelper::Kill(sTemp);

#if !(defined(WNT))
        sTemp = sDBConfig;
        sTemp += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("diag"));
        if(!UCBContentHelper::IsFolder(sTemp))
            UCBContentHelper::MakeFolder(sTemp);

        sTemp = sDBConfig;
        sTemp += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ipc"));
        if(!UCBContentHelper::IsFolder(sTemp))
            UCBContentHelper::MakeFolder(sTemp);

        sTemp = sDBConfig;
        sTemp += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("spool"));
        if(!UCBContentHelper::IsFolder(sTemp))
            UCBContentHelper::MakeFolder(sTemp);
#endif
    }
}
// -----------------------------------------------------------------------------
void ODriver::clearDatabase(const ::rtl::OUString& sDBName)
{ // stop the database
    ::rtl::OUString sCommand;
#if defined(WNT)
    ::rtl::OUString sStop = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("stop"));
    const sal_uInt32 nArgsCount = 2;
    rtl_uString *pArgs[nArgsCount] = { sDBName.pData, sStop.pData };
#else
    sCommand =  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("x_clear"));
    const sal_uInt32 nArgsCount = 1;
    rtl_uString* pArgs[nArgsCount] = { sDBName.pData };
#endif

    oslProcess aApp;

#if OSL_DEBUG_LEVEL > 0
    oslProcessError eError =
#endif
        osl_executeProcess(sCommand.pData, pArgs, nArgsCount,
                           OPROCESS_ADABAS, NULL, m_sDbWorkURL.pData,
                           NULL, 0, &aApp);
    OSL_ENSURE( eError == osl_Process_E_None, "ODriver::clearDatabase: calling the executable failed!" );
}
// -----------------------------------------------------------------------------
void ODriver::createDb( const TDatabaseStruct& _aInfo)
{

    clearDatabase(_aInfo.sDBName);

    X_PARAM(_aInfo.sDBName,_aInfo.sControlUser,_aInfo.sControlPassword,String::CreateFromAscii("BINIT"));

    String sTemp;
    LocalFileHelper::ConvertURLToPhysicalName(_aInfo.sSysDevSpace,sTemp);

    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYSDEVSPACE")),sTemp);

    sTemp.Erase();
    LocalFileHelper::ConvertURLToPhysicalName(_aInfo.sTransLogName,sTemp);
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TRANSACTION_LOG")),sTemp);
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MAXUSERTASKS")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("3")));
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MAXDEVSPACES")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("7")));
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MAXDATADEVSPACES")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("5")));
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MAXDATAPAGES")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("25599")));
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MAXBACKUPDEVS")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("1")));
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MAXSERVERDB")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("1")));
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATA_CACHE_PAGES")),_aInfo.sCacheSize);
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CONV_CACHE_PAGES")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("23")));
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PROC_DATA_PAGES")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("40")));
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RUNDIRECTORY")),m_sDbRunDir);
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KERNELTRACESIZE")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("100")));
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LOG_QUEUE_PAGES")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("10")));

#if !defined(WNT)
    PutParam(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OPMSG1")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/dev/null")));
#endif

    X_PARAM(_aInfo.sDBName,_aInfo.sControlUser,_aInfo.sControlPassword,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BCHECK")));

    X_START(_aInfo.sDBName);

    //  SHOW_STATE()
    //  %m_sDbRoot%\bin\xutil -d %_aInfo.sDBName% -u %CONUSR%,%CONPWD% -b %INITCMD%
    ::rtl::OUString aBatch2 =  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-b "));

//  if(!bBsp && INITCMD.Len() >= 40)
//  {
//      DirEntry aTmp(INITCMD);
//      aTmp.CopyTo(aInitFile, FSYS_ACTION_COPYFILE);
//      INITCMD = aInitFile.GetFull();
//  }
    // generate the init file for the database
    String sInitFile = getDatabaseInitFile(_aInfo);

    LocalFileHelper::ConvertURLToPhysicalName(sInitFile,sTemp);
    aBatch2 += sTemp;
    XUTIL(aBatch2,_aInfo.sDBName,_aInfo.sControlUser,_aInfo.sControlPassword);
#if OSL_DEBUG_LEVEL < 2
    if(UCBContentHelper::Exists(sInitFile))
        UCBContentHelper::Kill(sInitFile);
#endif

    // install system tables
    installSystemTables(_aInfo);
    // now we have to make our SYSDBA user "NOT EXCLUSIVE"
    {
        String sExt;
        sExt.AssignAscii(".sql");

        String sWorkUrl(m_sDbWorkURL);
        ::utl::TempFile aInitFile(String::CreateFromAscii("Init"),&sExt,&sWorkUrl);
        aInitFile.EnableKillingFile();
        {
            ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(aInitFile.GetURL(),STREAM_WRITE) );
            (*pFileStream)  << "ALTER USER \""
                            << ::rtl::OString(_aInfo.sSysUser,_aInfo.sSysUser.getLength(),gsl_getSystemTextEncoding())
                            << "\" NOT EXCLUSIVE "
                            << sNewLine;
            pFileStream->Flush();
        }
        { // just to get sure that the tempfile still lives
            sTemp.Erase();
            LocalFileHelper::ConvertURLToPhysicalName(aInitFile.GetURL(),sTemp);
            LoadBatch(_aInfo.sDBName,_aInfo.sSysUser,_aInfo.sSysPassword,sTemp);
        }
    }
}


//-------------------------------------------------------------------------------------------------
int ODriver::X_PARAM(const ::rtl::OUString& _DBNAME,
            const ::rtl::OUString& _USR,
            const ::rtl::OUString& _PWD,
            const ::rtl::OUString& _CMD)
{
    //  %XPARAM% -u %CONUSR%,%CONPWD% BINIT
    ::rtl::OUString sCommandFile = generateInitFile();
    {
        ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(sCommandFile,STREAM_STD_READWRITE));
        pFileStream->Seek(STREAM_SEEK_TO_END);
        (*pFileStream)  << "x_param"
#if defined(WNT)
                        << ".exe"
#endif
                        << " -d "
                        << ::rtl::OString(_DBNAME,_DBNAME.getLength(),gsl_getSystemTextEncoding())
                        << " -u "
                        << ::rtl::OString(_USR,_USR.getLength(),gsl_getSystemTextEncoding())
                        << ","
                        << ::rtl::OString(_PWD,_PWD.getLength(),gsl_getSystemTextEncoding())
                        << " "
                        << ::rtl::OString(_CMD,_CMD.getLength(),gsl_getSystemTextEncoding())
#if defined(WNT)
#if (OSL_DEBUG_LEVEL > 1) || defined(DBG_UTIL)
                        << " >> %DBWORK%\\create.log 2>&1"
#endif
#else
#if (OSL_DEBUG_LEVEL > 1) || defined(DBG_UTIL)
                        << " >> /tmp/kstart.log"
#else
                        << " > /dev/null"
#endif
#endif
                        << " "
                        << sNewLine
                        << sNewLine;

        pFileStream->Flush();
    }

    oslProcess aApp;

#if OSL_DEBUG_LEVEL > 0
    oslProcessError eError =
#endif
        osl_executeProcess(sCommandFile.pData, NULL, 0,
                           osl_Process_HIDDEN | osl_Process_WAIT,
                           NULL, m_sDbWorkURL.pData, NULL, 0, &aApp);
    OSL_ENSURE( eError == osl_Process_E_None, "ODriver::X_PARAM: calling the executable failed!" );

#if OSL_DEBUG_LEVEL < 2
    if(UCBContentHelper::Exists(sCommandFile))
        UCBContentHelper::Kill(sCommandFile);
#endif

    return 0;
}
// -----------------------------------------------------------------------------
sal_Int32 ODriver::CreateFiles(const TDatabaseStruct& _aInfo)
{
    int nRet = CreateFile(_aInfo.sSysDevSpace,_aInfo.nDataSize/50) ? 0 : -9;
    if(!nRet)
        nRet = CreateFile(_aInfo.sTransLogName,_aInfo.nLogSize) ? 0 : -10;
    if(!nRet)
        nRet = CreateFile(_aInfo.sDataDevName,_aInfo.nDataSize) ? 0 : -11;

    return nRet;

}
// -----------------------------------------------------------------------------
void ODriver::PutParam(const ::rtl::OUString& sDBName,
                      const ::rtl::OUString& rWhat,
                      const ::rtl::OUString& rHow)
{
    const sal_uInt32 nArgsCount = 3;
    rtl_uString* pArgs[nArgsCount] = { sDBName.pData, rWhat.pData, rHow.pData  };

    ::rtl::OUString sCommand = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("putparam"));
#if defined(WNT)
    sCommand += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".exe"));
#endif

    oslProcess aApp;


#if OSL_DEBUG_LEVEL > 0
    oslProcessError eError =
#endif
        osl_executeProcess(sCommand.pData, pArgs, nArgsCount,
                           OPROCESS_ADABAS,
                           NULL, m_sDbWorkURL.pData, NULL, 0, &aApp);

    OSL_ENSURE( eError == osl_Process_E_None, "ODriver::PutParam: calling the executable failed!" );
}
// -----------------------------------------------------------------------------
sal_Bool ODriver::CreateFile(const ::rtl::OUString &_FileName,
                sal_Int32 _nSize)
{
OSL_TRACE("CreateFile %d",_nSize);
    sal_Bool bOK = sal_True;
    try
    {
        ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(_FileName,STREAM_WRITE));
        if( !pFileStream.get())
        {
            ::connectivity::SharedResources aResources;
            const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(
                    STR_NO_DISK_SPACE,
                    "$filename$",_FileName
                 ) );
            ::dbtools::throwGenericSQLException(sError,*this);
        }
        (*pFileStream).SetFiller('\0');
        sal_Int32 nNewSize = 0;
        sal_Int32 nCount = _nSize /2;
        for(sal_Int32 i=0; bOK && i < nCount; ++i)
        {
            nNewSize += 8192;//4096;
            bOK = (*pFileStream).SetStreamSize(nNewSize);
            pFileStream->Flush();
        }

        bOK = bOK && static_cast<sal_Int32>(pFileStream->Seek(STREAM_SEEK_TO_END)) == nNewSize;
    }
    catch(Exception&)
    {
    OSL_TRACE("Exception");
    }
    if(!bOK)
    {
        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(
                STR_NO_DISK_SPACE,
                "$filename$",_FileName
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }

    return bOK;
    // dd if=/dev/zero bs=4k of=$DEV_NAME count=$2
}
// -----------------------------------------------------------------------------
int ODriver::X_START(const ::rtl::OUString& sDBName)
{
    ::rtl::OUString sCommand;
#if defined(WNT)

    ::rtl::OUString sArg1 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-d"));
    ::rtl::OUString sArg3 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-NoDBService"));
    ::rtl::OUString sArg4 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-NoDBWindow"));

    const sal_uInt32 nArgsCount = 4;
    rtl_uString* pArgs[nArgsCount] = { sArg1.pData, sDBName.pData, sArg3.pData, sArg4.pData };
    sCommand =  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("strt.exe"));
#else
    const sal_uInt32 nArgsCount = 1;
    rtl_uString* pArgs[nArgsCount] = { sDBName.pData };
    sCommand =  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("x_start"));
#endif

    oslProcess aApp;

    oslProcessError eError = osl_executeProcess(sCommand.pData, pArgs, nArgsCount,
                                                OPROCESS_ADABAS, NULL, m_sDbWorkURL.pData,
                                                NULL, 0, &aApp);

    if(eError == osl_Process_E_NotFound)
    {
        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(
                STR_COMMAND_NOT_FOUND,
                "$databasename$",sDBName,
                "$progname$",sCommand
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
    OSL_ASSERT(eError == osl_Process_E_None);

    oslProcessInfo aInfo;
    aInfo.Size = sizeof(oslProcessInfo);

    if(osl_getProcessInfo(aApp, osl_Process_EXITCODE, &aInfo) == osl_Process_E_None && aInfo.Code)
        return aInfo.Code;

    return 0;
}
// -----------------------------------------------------------------------------
int ODriver::X_STOP(const ::rtl::OUString& sDBName)
{
    ::rtl::OUString sCommand;
#if defined(WNT)

    ::rtl::OUString sArg1 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-d"));
    ::rtl::OUString sArg2 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-NoDBService"));

    const sal_uInt32 nArgsCount = 3;
    rtl_uString* pArgs[nArgsCount] = { sArg1.pData, sDBName.pData, sArg2.pData };
    sCommand =  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("stp.exe"));
#else
    const sal_uInt32 nArgsCount = 1;
    rtl_uString* pArgs[nArgsCount] = { sDBName.pData };
    sCommand = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("x_stop"));
#endif

    oslProcess aApp;

    oslProcessError eError = osl_executeProcess(sCommand.pData, pArgs, nArgsCount,
                                                OPROCESS_ADABAS, NULL, m_sDbWorkURL.pData,
                                                NULL, 0, &aApp);

    OSL_ASSERT(eError == osl_Process_E_None);
    if(eError != osl_Process_E_None)
        return 1;

    oslProcessInfo aInfo;
    aInfo.Size = sizeof(oslProcessInfo);

    if(osl_getProcessInfo(aApp, osl_Process_EXITCODE, &aInfo) == osl_Process_E_None && aInfo.Code)
        return aInfo.Code;

    return 0;
}
// -----------------------------------------------------------------------------
void ODriver::XUTIL(const ::rtl::OUString& _rParam,
           const ::rtl::OUString& _DBNAME,
           const ::rtl::OUString& _USRNAME,
           const ::rtl::OUString& _USRPWD)
{
    String sWorkUrl(m_sDbWorkURL);
    String sExt = String::CreateFromAscii(".log");
    ::utl::TempFile aCmdFile(String::CreateFromAscii("xutil"),&sExt,&sWorkUrl);
    aCmdFile.EnableKillingFile();

    String sPhysicalPath;
    LocalFileHelper::ConvertURLToPhysicalName(aCmdFile.GetURL(),sPhysicalPath);

    ::rtl::OUString sCommandFile = generateInitFile();

    {
        ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(sCommandFile,STREAM_STD_READWRITE));
        pFileStream->Seek(STREAM_SEEK_TO_END);
        (*pFileStream)  <<
#if defined(WNT)
                            "xutil.exe"
#else
                            "utility"
#endif
                        << " -u "
                        << ::rtl::OString(_USRNAME,_USRNAME.getLength(),gsl_getSystemTextEncoding())
                        << ","
                        << ::rtl::OString(_USRPWD,_USRPWD.getLength(),gsl_getSystemTextEncoding())
                        << " -d "
                        << ::rtl::OString(_DBNAME,_DBNAME.getLength(),gsl_getSystemTextEncoding())
                        << " "
                        << ::rtl::OString(_rParam,_rParam.getLength(),gsl_getSystemTextEncoding())
                        << " > "
                        << ::rtl::OString(sPhysicalPath.GetBuffer(),sPhysicalPath.Len(),gsl_getSystemTextEncoding())
                        << " 2>&1"
                        << sNewLine;
        pFileStream->Flush();
    }

    oslProcess aApp;


#if OSL_DEBUG_LEVEL > 0
    oslProcessError eError =
#endif
        osl_executeProcess(sCommandFile.pData, NULL, 0,
                           osl_Process_HIDDEN | osl_Process_WAIT,
                           NULL, m_sDbWorkURL.pData, NULL, 0, &aApp);
    OSL_ENSURE( eError == osl_Process_E_None, "ODriver::XUTIL: calling the executable failed!" );

#if OSL_DEBUG_LEVEL < 2
    if(UCBContentHelper::Exists(sCommandFile))
        UCBContentHelper::Kill(sCommandFile);
#endif
}
// -----------------------------------------------------------------------------
void ODriver::LoadBatch(const ::rtl::OUString& sDBName,
               const ::rtl::OUString& _rUSR,
               const ::rtl::OUString& _rPWD,
               const ::rtl::OUString& _rBatch)
{
    OSL_ENSURE(_rBatch.getLength(),"No batch file given!");
    String sWorkUrl(m_sDbWorkURL);
    String sExt = String::CreateFromAscii(".log");
    ::utl::TempFile aCmdFile(String::CreateFromAscii("LoadBatch"),&sExt,&sWorkUrl);
#if OSL_DEBUG_LEVEL < 2
    aCmdFile.EnableKillingFile();
#endif

    String sPhysicalPath;
    LocalFileHelper::ConvertURLToPhysicalName(aCmdFile.GetURL(),sPhysicalPath);

    ::rtl::OUString sCommandFile = generateInitFile();
    {
        ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(sCommandFile,STREAM_STD_READWRITE));
        pFileStream->Seek(STREAM_SEEK_TO_END);
        (*pFileStream)  << "xload"
#if defined(WNT)
                        << ".exe"
#endif
                        << " -d "
                        << ::rtl::OString(sDBName,sDBName.getLength(),gsl_getSystemTextEncoding())
                        << " -u "
                        << ::rtl::OString(_rUSR,_rUSR.getLength(),gsl_getSystemTextEncoding())
                        << ","
                        << ::rtl::OString(_rPWD,_rPWD.getLength(),gsl_getSystemTextEncoding());

        if ( !isKernelVersion(CURRENT_DB_VERSION) )
            (*pFileStream) << " -S adabas -b ";
        else
            (*pFileStream) << " -S NATIVE -b ";

        (*pFileStream)  << ::rtl::OString(_rBatch,_rBatch.getLength(),gsl_getSystemTextEncoding())
                        << " > "
                        << ::rtl::OString(sPhysicalPath.GetBuffer(),sPhysicalPath.Len(),gsl_getSystemTextEncoding())
                        << " 2>&1"
                        << sNewLine;

        pFileStream->Flush();
    }

    oslProcess aApp;


#if OSL_DEBUG_LEVEL > 0
    oslProcessError eError =
#endif
        osl_executeProcess(sCommandFile.pData, NULL, 0,
                           osl_Process_HIDDEN | osl_Process_WAIT,
                           NULL, m_sDbWorkURL.pData, NULL, 0, &aApp);
    OSL_ENSURE( eError == osl_Process_E_None, "ODriver::LoadBatch: calling the executable failed!" );

#if OSL_DEBUG_LEVEL < 2
    if(UCBContentHelper::Exists(sCommandFile))
        UCBContentHelper::Kill(sCommandFile);
#endif
}
// -----------------------------------------------------------------------------
void ODriver::fillEnvironmentVariables()
{
    // read the environment vars
    struct env_data
    {
        const sal_Char*     pAsciiEnvName;
        ::rtl::OUString*    pValue;
        ::rtl::OUString*    pValueURL;
    } EnvData[] = {
        { "DBWORK",     &m_sDbWork,     &m_sDbWorkURL },
        { "DBCONFIG",   &m_sDbConfig,   &m_sDbConfigURL },
        { "DBROOT",     &m_sDbRoot,     &m_sDbRootURL }
    };

    for ( size_t i = 0; i < SAL_N_ELEMENTS( EnvData ); ++i )
    {
        ::rtl::OUString sVarName = ::rtl::OUString::createFromAscii( EnvData[i].pAsciiEnvName );
        ::rtl::OUString sEnvValue;
        if(osl_getEnvironment( sVarName.pData, &sEnvValue.pData ) == osl_Process_E_None )
        {
            *EnvData[i].pValue = sEnvValue;
            String sURL;
            LocalFileHelper::ConvertPhysicalNameToURL( *EnvData[i].pValue, sURL );
            *EnvData[i].pValueURL = sURL;
        }
    }

    m_sDelimit =  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
}
// -----------------------------------------------------------------------------
::rtl::OUString ODriver::generateInitFile() const
{
    String sExt;
#if !defined(WNT)
    sExt = String::CreateFromAscii(".sh");
#else
    sExt = String::CreateFromAscii(".bat");
#endif

    String sWorkUrl(m_sDbWorkURL);
    ::utl::TempFile aCmdFile(String::CreateFromAscii("Init"),&sExt,&sWorkUrl);
#if !defined(WNT)
    String sPhysicalPath;
    LocalFileHelper::ConvertURLToPhysicalName(aCmdFile.GetURL(),sPhysicalPath);
    chmod(ByteString(sPhysicalPath,gsl_getSystemTextEncoding()).GetBuffer(),S_IRUSR|S_IWUSR|S_IXUSR);
#endif

#if !defined(WNT)
    SvStream* pFileStream = aCmdFile.GetStream(STREAM_WRITE);
    (*pFileStream)  << "#!/bin/sh"
                    << sNewLine
                    << "cd \"$DBWORK\""
                    << sNewLine
                    << sNewLine;
    pFileStream->Flush();
#endif

    return aCmdFile.GetURL();
}
// -----------------------------------------------------------------------------
::rtl::OUString ODriver::getDatabaseInitFile(  const TDatabaseStruct& _aDBInfo)
{
    String sExt;
    sExt.AssignAscii(".ins");


    String sWorkUrl(m_sDbWorkURL);
    ::utl::TempFile aInitFile(String::CreateFromAscii("Init"),&sExt,&sWorkUrl);
    {
        SvStream* pFileStream = aInitFile.GetStream(STREAM_WRITE);
        (*pFileStream) << "* @(#)init.cmd  6.1.1   1994-11-10\n";
        (*pFileStream) << "init config\n";
        (*pFileStream) << "* default code:\n";
        (*pFileStream) << "ascii\n";
        (*pFileStream) << "* date time format\n";
        (*pFileStream) << "internal\n";
        (*pFileStream) << "* command timeout:\n";
        (*pFileStream) << "900\n";
        (*pFileStream) << "* lock timeout:\n";
        (*pFileStream) << "360\n";
        (*pFileStream) << "* request timeout:\n";
        (*pFileStream) << "180\n";
        (*pFileStream) << "* log mode:\n";
        (*pFileStream) << "demo\n";
        (*pFileStream) << "* log segment size:\n";
        (*pFileStream) << "0\n";
        (*pFileStream) << "* no of archive logs:\n";
        (*pFileStream) << "0\n";
        (*pFileStream) << "* no of data devspaces:\n";
        (*pFileStream) << "1\n";
        (*pFileStream) << "* mirror devspaces:\n";
        (*pFileStream) << "n\n";
        (*pFileStream) << "if $rc <> 0 then stop\n";
        (*pFileStream) << "*---  device description ---\n";
        (*pFileStream) << "* sys devspace name:\n";
        {
            String sTemp;
            LocalFileHelper::ConvertURLToPhysicalName(_aDBInfo.sSysDevSpace,sTemp);
            (*pFileStream) << ::rtl::OString(sTemp.GetBuffer(),sTemp.Len(),gsl_getSystemTextEncoding());
        }
        (*pFileStream) << "\n* log devspace size:\n";
        (*pFileStream) << ::rtl::OString::valueOf(_aDBInfo.nLogSize);
        (*pFileStream) << "\n* log devspace name:\n";
        {
            String sTemp;
            LocalFileHelper::ConvertURLToPhysicalName(_aDBInfo.sTransLogName,sTemp);
            (*pFileStream) << ::rtl::OString(sTemp.GetBuffer(),sTemp.Len(),gsl_getSystemTextEncoding());
        }
        (*pFileStream) << "\n* data devspace size:\n";
        (*pFileStream) << ::rtl::OString::valueOf(_aDBInfo.nDataSize);
        (*pFileStream) << "\n* data devspace name:\n";
        {
            String sTemp;
            LocalFileHelper::ConvertURLToPhysicalName(_aDBInfo.sDataDevName,sTemp);
            (*pFileStream) << ::rtl::OString(sTemp.GetBuffer(),sTemp.Len(),gsl_getSystemTextEncoding());
        }

        (*pFileStream) << "\n* END INIT CONFIG\n";
        (*pFileStream) << "if $rc <> 0 then stop\n";
        if(_aDBInfo.bRestoreDatabase)
        {
            (*pFileStream) << "RESTORE DATA QUICK FROM '";
            {
                String sTemp;
                LocalFileHelper::ConvertURLToPhysicalName(_aDBInfo.sBackupFile,sTemp);
                (*pFileStream) << ::rtl::OString(sTemp.GetBuffer(),sTemp.Len(),gsl_getSystemTextEncoding());
            }
            (*pFileStream) << "' BLOCKSIZE 8\n";
            (*pFileStream) << "if $rc <> 0 then stop\n";
            (*pFileStream) << "RESTART\n";

        }
        else
        {
            (*pFileStream) << "ACTIVATE SERVERDB SYSDBA \"";
            (*pFileStream) << ::rtl::OString(_aDBInfo.sSysUser,_aDBInfo.sSysUser.getLength(),gsl_getSystemTextEncoding());
            (*pFileStream) << "\" PASSWORD \"";
            (*pFileStream) << ::rtl::OString(_aDBInfo.sSysPassword,_aDBInfo.sSysPassword.getLength(),gsl_getSystemTextEncoding());
            (*pFileStream) << "\"\n";
        }
        (*pFileStream) << "if $rc <> 0 then stop\n";
        (*pFileStream) << "exit\n";
    }
    return aInitFile.GetURL();
}
// -----------------------------------------------------------------------------
void ODriver::X_CONS(const ::rtl::OUString& sDBName,const ::rtl::OString& _ACTION,const ::rtl::OUString& _FILENAME)
{
    String sPhysicalPath;
    LocalFileHelper::ConvertURLToPhysicalName(_FILENAME,sPhysicalPath);

    ::rtl::OUString sCommandFile = generateInitFile();
    {
        ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(sCommandFile,STREAM_STD_READWRITE));
        pFileStream->Seek(STREAM_SEEK_TO_END);

        (*pFileStream)  << "x_cons"
#if defined(WNT)
                        << ".exe"
#endif
                        << " "
                        << ::rtl::OString(sDBName,sDBName.getLength(),gsl_getSystemTextEncoding())
                        << " SHOW "
                        << _ACTION
                        << " > "
                        << ::rtl::OString(sPhysicalPath.GetBuffer(),sPhysicalPath.Len(),gsl_getSystemTextEncoding())
                        << sNewLine;
        pFileStream->Flush();
    }

    oslProcess aApp;

    osl_executeProcess(sCommandFile.pData, NULL, 0,
                       osl_Process_HIDDEN | osl_Process_WAIT,
                       NULL, m_sDbWorkURL.pData, NULL, 0, &aApp);

#if OSL_DEBUG_LEVEL < 2
    if(UCBContentHelper::Exists(sCommandFile))
        UCBContentHelper::Kill(sCommandFile);
#endif
}
// -----------------------------------------------------------------------------
void ODriver::checkAndRestart(const ::rtl::OUString& sDBName,const TDatabaseStruct& _rDbInfo)
{
    String sWorkUrl(m_sDbWorkURL);
    String sExt = String::CreateFromAscii(".st");
    ::utl::TempFile aCmdFile(String::CreateFromAscii("State"),&sExt,&sWorkUrl);
    aCmdFile.EnableKillingFile();

    X_CONS(sDBName,"STATE",aCmdFile.GetURL());
    SvStream* pFileStream = aCmdFile.GetStream(STREAM_SHARE_DENYALL);
    if ( pFileStream )
    {
        ByteString sStateLine;
        sal_Bool bRead = sal_True;
        sal_Int32 nStart = 2;
        while(bRead && !pFileStream->IsEof())
        {
            String aLine;
            bRead = pFileStream->ReadLine(sStateLine);
            if(bRead)
            {
                if(sStateLine.Search("WARM") != STRING_NOTFOUND)
                {   // nothing to do
                    nStart = 0;
                    break;
                }
                else if(sStateLine.Search("COLD") != STRING_NOTFOUND)
                {
                    nStart = 1;
                    break;
                }
            }
        }
        switch(nStart)
        {
            case 2:
                clearDatabase(sDBName);
                X_START(sDBName);
                // don't break here
            case 1:
                XUTIL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RESTART")),sDBName,_rDbInfo.sControlUser,_rDbInfo.sControlPassword);
            case 0:
                break;
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool ODriver::isVersion(const ::rtl::OUString& sDBName, const char* _pVersion)
{
    String sWorkUrl(m_sDbWorkURL);
    String sExt = String::CreateFromAscii(".st");
    ::utl::TempFile aCmdFile(String::CreateFromAscii("DevSpaces"),&sExt,&sWorkUrl);
    aCmdFile.EnableKillingFile();

    String sPhysicalPath;
    LocalFileHelper::ConvertURLToPhysicalName(aCmdFile.GetURL(),sPhysicalPath);

    ::rtl::OUString sCommandFile = generateInitFile();
    {
        ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(sCommandFile,STREAM_STD_READWRITE));
        pFileStream->Seek(STREAM_SEEK_TO_END);

        (*pFileStream)  << "getparam"
#if defined(WNT)
                        << ".exe"
#endif
                        << " "
                        << ::rtl::OString(sDBName,sDBName.getLength(),gsl_getSystemTextEncoding())
                        << " KERNELVERSION > "
                        << ::rtl::OString(sPhysicalPath.GetBuffer(),sPhysicalPath.Len(),gsl_getSystemTextEncoding())
                        << sNewLine;
    }

    oslProcess aApp;

    osl_executeProcess(sCommandFile.pData, NULL, 0,
                       OPROCESS_ADABAS,
                       NULL, m_sDbWorkURL.pData, NULL, 0, &aApp);

#if OSL_DEBUG_LEVEL < 2
    if(UCBContentHelper::Exists(sCommandFile))
        UCBContentHelper::Kill(sCommandFile);
#endif
    SvStream* pFileStream = aCmdFile.GetStream(STREAM_STD_READWRITE);
    ByteString sStateLine;
    sal_Bool bRead = sal_True;
    sal_Bool bIsVersion = sal_False;
    while ( pFileStream && bRead && !pFileStream->IsEof() )
    {
        bRead = pFileStream->ReadLine(sStateLine);
        if ( bRead )
        {
            bIsVersion = sStateLine.GetToken(1,' ').Equals(_pVersion) != 0;
            break;
        }
    }
    return bIsVersion;
}
// -----------------------------------------------------------------------------
void ODriver::checkAndInsertNewDevSpace(const ::rtl::OUString& sDBName,
                                        const TDatabaseStruct& _rDBInfo)
{
    //  %DBROOT%\pgm\getparam %2 DATA_CACHE_PAGES > %3
    String sWorkUrl(m_sDbWorkURL);
    String sExt = String::CreateFromAscii(".st");
    ::utl::TempFile aCmdFile(String::CreateFromAscii("DevSpaces"),&sExt,&sWorkUrl);
    aCmdFile.EnableKillingFile();

    String sPhysicalPath;
    LocalFileHelper::ConvertURLToPhysicalName(aCmdFile.GetURL(),sPhysicalPath);

    ::rtl::OUString sCommandFile = generateInitFile();
    {
        ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(sCommandFile,STREAM_STD_READWRITE));
        pFileStream->Seek(STREAM_SEEK_TO_END);

        (*pFileStream)  << "getparam"
#if defined(WNT)
                        << ".exe"
#endif
                        << " "
                        << ::rtl::OString(sDBName,sDBName.getLength(),gsl_getSystemTextEncoding())
                        << " DATA_CACHE_PAGES > "
                        << ::rtl::OString(sPhysicalPath.GetBuffer(),sPhysicalPath.Len(),gsl_getSystemTextEncoding())
                        << sNewLine;
    }

    oslProcess aApp;
    osl_executeProcess(sCommandFile.pData, NULL, 0,
                       OPROCESS_ADABAS,
                       NULL, m_sDbWorkURL.pData, NULL, 0, &aApp);

#if OSL_DEBUG_LEVEL < 2
    if(UCBContentHelper::Exists(sCommandFile))
        UCBContentHelper::Kill(sCommandFile);
#endif
    SvStream* pFileStream = aCmdFile.GetStream(STREAM_STD_READWRITE);
    ByteString sStateLine;
    sal_Bool bRead = sal_True;
    sal_Int32 nDataPages = 0;
    while(pFileStream && bRead && !pFileStream->IsEof())
    {
        bRead = pFileStream->ReadLine(sStateLine);
        if(bRead)
        {
            nDataPages = sStateLine.ToInt32();
            if(nDataPages && nDataPages < 100)
            {
                // the space isn't big enough anymore so we increment it
                PutParam(sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATA_CACHE_PAGES")),::rtl::OUString::valueOf(nDataPages));
                X_PARAM(sDBName,_rDBInfo.sControlUser,_rDBInfo.sControlPassword,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BCHECK")));
            }
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool ODriver::isKernelVersion(const char* _pVersion)
{
    ::utl::TempFile aCmdFile(String::CreateFromAscii("KernelVersion"));
    aCmdFile.EnableKillingFile();

    String sPhysicalPath;
    LocalFileHelper::ConvertURLToPhysicalName(aCmdFile.GetURL(),sPhysicalPath);

    ::rtl::OUString sCommandFile = generateInitFile();
    {
        ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(sCommandFile,STREAM_STD_READWRITE));
        pFileStream->Seek(STREAM_SEEK_TO_END);

        (*pFileStream)  << "dbversion"
                        << " > "
                        << ::rtl::OString(sPhysicalPath.GetBuffer(),sPhysicalPath.Len(),gsl_getSystemTextEncoding())
                        << sNewLine;
    }

    oslProcess aApp;
    osl_executeProcess(sCommandFile.pData, NULL, 0,
                       OPROCESS_ADABAS,
                       NULL, m_sDbWorkURL.pData, NULL, 0, &aApp);

#if OSL_DEBUG_LEVEL < 2
    if(UCBContentHelper::Exists(sCommandFile))
        UCBContentHelper::Kill(sCommandFile);
#endif
    SvStream* pFileStream = aCmdFile.GetStream(STREAM_STD_READWRITE);
    ByteString sStateLine;
    sal_Bool bRead = sal_True;
    sal_Bool bIsVersion = sal_True;
    while ( pFileStream && bRead && !pFileStream->IsEof() )
    {
        bRead = pFileStream->ReadLine(sStateLine);
        if ( bRead )
        {
            // convert a 11.02.00 to a 12.01.30 version
            bIsVersion = sStateLine.GetToken(0).Equals(_pVersion) != 0;
            break;
        }
    }
    return bIsVersion;
}
// -----------------------------------------------------------------------------
void ODriver::installSystemTables(  const TDatabaseStruct& _aInfo)
{
#if defined(WNT)
    //  xutil -d %_DBNAME% -u %_CONTROL_USER%,%_CONTROL_PWD% -b %m_sDbRoot%\env\TERMCHAR.ind
    ::rtl::OUString aBatch =  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-b "));
    ::rtl::OUString sTemp2 = m_sDbRootURL   + m_sDelimit
                                            + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("env"))
                                            + m_sDelimit
                                            + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TERMCHAR.ind"));
    String sTemp;
    sal_Bool bOk = LocalFileHelper::ConvertURLToPhysicalName(sTemp2,sTemp);
    aBatch += sTemp;

    XUTIL(aBatch,_aInfo.sDBName,_aInfo.sControlUser,_aInfo.sControlPassword);

    //  xutil -d %_DBNAME% -u %_CONTROL_USER%,%_CONTROL_PWD% DIAGNOSE TRIGGER OFF
    XUTIL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DIAGNOSE TRIGGER OFF")),_aInfo.sDBName,_aInfo.sControlUser,_aInfo.sControlPassword);
    //  xload -d %_DBNAME% -u %_SYSDBA_USER%,%_SYSDBA_PWD% -S NATIVE -b %m_sDbRoot%\env\DBS.ins %_DOMAINPWD%
    {
        sTemp2 = m_sDbRootURL
                                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("env"))
                                + m_sDelimit
                                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DBS.ins"));
        sTemp.Erase();
        bOk = LocalFileHelper::ConvertURLToPhysicalName(sTemp2,sTemp);
        OSL_ENSURE(bOk,"File could be converted into file system path!");
        sTemp.AppendAscii(" ");
        sTemp += String(_aInfo.sDomainPassword);

        LoadBatch(_aInfo.sDBName,_aInfo.sSysUser,_aInfo.sSysPassword,sTemp);
    }
    //  xload -d %_DBNAME% -u DOMAIN,%_DOMAINPWD% -S NATIVE -b %m_sDbRoot%\env\XDD.ins
    {
        sTemp2 = m_sDbRootURL
                                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("env"))
                                + m_sDelimit
                                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("XDD.ins"));
        sTemp.Erase();
        bOk = LocalFileHelper::ConvertURLToPhysicalName(sTemp2,sTemp);
        OSL_ENSURE(bOk,"File could be converted into file system path!");

        LoadBatch(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DOMAIN")),_aInfo.sDomainPassword,sTemp);
    }
    //  xload -d %_DBNAME% -u %_SYSDBA_USER%,%_SYSDBA_PWD% -S NATIVE -b %m_sDbRoot%\env\QP.ins
    {
        sTemp2 = m_sDbRootURL
                                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("env"))
                                + m_sDelimit
                                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("QP.ins"));
        sTemp.Erase();
        bOk = LocalFileHelper::ConvertURLToPhysicalName(sTemp2,sTemp);
        OSL_ENSURE(bOk,"File could be converted into file system path!");
        LoadBatch(_aInfo.sDBName,_aInfo.sSysUser,_aInfo.sSysPassword,sTemp);
    }
    //  xload  -d %_DBNAME% -u DOMAIN,%_DOMAINPWD% -S NATIVE -b %m_sDbRoot%\env\SPROC.ins
    {
        sTemp2 = m_sDbRootURL
                                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("env"))
                                + m_sDelimit
                                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SPROC.ins"));
        sTemp.Erase();
        bOk = LocalFileHelper::ConvertURLToPhysicalName(sTemp2,sTemp);
        OSL_ENSURE(bOk,"File could be converted into file system path!");

        LoadBatch(_aInfo.sDBName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DOMAIN")),_aInfo.sDomainPassword,sTemp);
    }

    //  xutil -d %_DBNAME% -u %_CONTROL_USER%,%_CONTROL_PWD%  DIAGNOSE TRIGGER ON
    XUTIL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DIAGNOSE TRIGGER ON")),_aInfo.sDBName,_aInfo.sControlUser,_aInfo.sControlPassword);
    //  xutil -d %_DBNAME% -u %_CONTROL_USER%,%_CONTROL_PWD%  SET NOLOG OFF
    XUTIL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SET NOLOG OFF")),_aInfo.sDBName,_aInfo.sControlUser,_aInfo.sControlPassword);
    //  xutil -d %_DBNAME% -u %_CONTROL_USER%,%_CONTROL_PWD%  SHUTDOWN QUICK
    XUTIL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SHUTDOWN QUICK")),_aInfo.sDBName,_aInfo.sControlUser,_aInfo.sControlPassword);
    //  xutil -d %_DBNAME% -u %_CONTROL_USER%,%_CONTROL_PWD%  RESTART
    XUTIL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RESTART")),_aInfo.sDBName,_aInfo.sControlUser,_aInfo.sControlPassword);

#else // UNX
    ::rtl::OUString sCommandFile = generateInitFile();
    {
        ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(sCommandFile,STREAM_STD_READWRITE));
        pFileStream->Seek(STREAM_SEEK_TO_END);
        (*pFileStream)  << "x_dbinst"
                        << " -d "
                        << ::rtl::OString(_aInfo.sDBName,_aInfo.sDBName.getLength(),gsl_getSystemTextEncoding())
                        << " -u "
                        << ::rtl::OString(_aInfo.sSysUser,_aInfo.sSysUser.getLength(),gsl_getSystemTextEncoding())
                        << ","
                        << ::rtl::OString(_aInfo.sSysPassword,_aInfo.sSysPassword.getLength(),gsl_getSystemTextEncoding())
                        << " -w "
                        << ::rtl::OString(_aInfo.sDomainPassword,_aInfo.sDomainPassword.getLength(),gsl_getSystemTextEncoding())
                        << " -b ";

        if ( isKernelVersion(ADABAS_KERNEL_11) )
            (*pFileStream) << "-i all";
        (*pFileStream)
#if (OSL_DEBUG_LEVEL > 1) || defined(DBG_UTIL)
                    << " >> /tmp/kstart.log"
#else
                    << " > /dev/null"
#endif
                        << sNewLine
                        << sNewLine;
        pFileStream->Flush();
    }
    // now execute the command

    oslProcess aApp;
    osl_executeProcess(sCommandFile.pData, NULL, 0,
                       osl_Process_WAIT | osl_Process_HIDDEN,
                       NULL, m_sDbWorkURL.pData, NULL, 0, &aApp);
#if OSL_DEBUG_LEVEL < 2
    if(UCBContentHelper::Exists(sCommandFile))
        UCBContentHelper::Kill(sCommandFile);
#endif

#endif //WNT,UNX
}
// -----------------------------------------------------------------------------
void ODriver::convertOldVersion(const ::rtl::OUString& sDBName,const TDatabaseStruct& _rDbInfo)
{
    // first we have to check if this databse is a old version and we have to update the system tables
    if ( !isVersion(sDBName,CURRENT_DB_VERSION) && isKernelVersion(CURRENT_DB_VERSION) )
    {
        if (    !_rDbInfo.sControlUser.getLength()
            ||  !_rDbInfo.sControlPassword.getLength())
        {
            ::connectivity::SharedResources aResources;
            const ::rtl::OUString sError( aResources.getResourceString(STR_DATABASE_NEEDS_CONVERTING) );
            ::dbtools::throwGenericSQLException(sError,*this);
        }
        String sCommandFile = m_sDbWorkURL;
        sCommandFile += String::CreateFromAscii("/xparam.prt");
        if ( UCBContentHelper::Exists(sCommandFile) )
            UCBContentHelper::Kill(sCommandFile);
        X_PARAM(sDBName,_rDbInfo.sControlUser,_rDbInfo.sControlPassword,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BCHECK")));

        if ( UCBContentHelper::Exists(sCommandFile) )
        {
            {
                ::std::auto_ptr<SvStream> pFileStream( UcbStreamHelper::CreateStream(sCommandFile,STREAM_STD_READ) );
                ByteString sStateLine;
                sal_Bool bRead = sal_True;
                static ByteString s_ErrorId("-21100");
                while ( pFileStream.get() && bRead && !pFileStream->IsEof() )
                {
                    bRead = pFileStream->ReadLine(sStateLine);
                    if ( bRead && s_ErrorId == sStateLine.GetToken(0,' ') )
                    {
                        UCBContentHelper::Kill(sCommandFile);
                        ::rtl::OUString sError(::rtl::OUString::createFromAscii(sStateLine.GetBuffer()));
                        throw SQLException(sError,*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")),1000,Any());
                    }
                }
            }

            UCBContentHelper::Kill(sCommandFile);
        }
    }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace adabas
}// namespace connectivity
// -----------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
