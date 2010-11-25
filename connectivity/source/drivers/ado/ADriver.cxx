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
#include "ado/ADriver.hxx"
#include "ado/AConnection.hxx"
#include "ado/Awrapadox.hxx"
#include "ado/ACatalog.hxx"
#include "ado/Awrapado.hxx"
#include "ado/adoimp.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "connectivity/dbexception.hxx"
#include "resource/ado_res.hrc"
#include <Objbase.h>


#include "resource/sharedresources.hxx"

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::lang;

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
ODriver::ODriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB)
    : ODriver_BASE(m_aMutex)
    ,m_xORB(_xORB)
{
     if ( FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)) )
     {
         CoUninitialize();
         int h = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
         (void)h;
         ++h;
     }
}
// -------------------------------------------------------------------------
ODriver::~ODriver()
{
    CoUninitialize();
    CoInitialize(NULL);
}
//------------------------------------------------------------------------------
void ODriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);


    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    ODriver_BASE::disposing();
}
// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ODriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.sdbc.ado.ODriver");
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
    aSNS[1] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Driver");
    return aSNS;
}
//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL connectivity::ado::ODriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new ODriver(_rxFactory));
}

// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

// --------------------------------------------------------------------------------
sal_Bool SAL_CALL ODriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

// --------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
        return NULL;

    OConnection* pCon = new OConnection(this);
    pCon->construct(url,info);
    Reference< XConnection > xCon = pCon;
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL ODriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    return (!url.compareTo(::rtl::OUString::createFromAscii("sdbc:ado:"),9));
}
// -----------------------------------------------------------------------------
void ODriver::impl_checkURL_throw(const ::rtl::OUString& _sUrl)
{
    if ( !acceptsURL(_sUrl) )
    {
        SharedResources aResources;
        const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    } // if ( !acceptsURL(_sUrl) )
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL ODriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
    impl_checkURL_throw(url);
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< ::rtl::OUString > aBooleanValues(2);
        aBooleanValues[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) );
        aBooleanValues[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) );

        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IgnoreDriverPrivileges"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Ignore the privileges from the database driver."))
                ,sal_False
                ,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) )
                ,aBooleanValues)
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EscapeDateTime"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Escape date time format."))
                ,sal_False
                ,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) )
                ,aBooleanValues)
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TypeInfoSettings"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Defines how the type info of the database metadata should be manipulated."))
                ,sal_False
                ,::rtl::OUString( )
                ,Sequence< ::rtl::OUString > ())
        );
        return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
    }
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
// --------------------------------------------------------------------------------
// XDataDefinitionSupplier
Reference< XTablesSupplier > SAL_CALL ODriver::getDataDefinitionByConnection( const Reference< ::com::sun::star::sdbc::XConnection >& connection ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OConnection* pConnection = NULL;
    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(connection,UNO_QUERY);
    if(xTunnel.is())
    {
        OConnection* pSearchConnection = reinterpret_cast< OConnection* >( xTunnel->getSomething(OConnection::getUnoTunnelImplementationId()) );

        for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
        {
            if ((OConnection*) Reference< XConnection >::query(i->get().get()).get() == pSearchConnection)
            {
                pConnection = pSearchConnection;
                break;
            }
        }

    }

    Reference< XTablesSupplier > xTab = NULL;
    if(pConnection)
    {
        WpADOCatalog aCatalog;
        aCatalog.Create();
        if(aCatalog.IsValid())
        {
            aCatalog.putref_ActiveConnection(*pConnection->getConnection());
            OCatalog* pCatalog = new OCatalog(aCatalog,pConnection);
            xTab = pCatalog;
            pConnection->setCatalog(xTab);
            pConnection->setCatalog(pCatalog);
        }
    }
    return xTab;
}
// --------------------------------------------------------------------------------
Reference< XTablesSupplier > SAL_CALL ODriver::getDataDefinitionByURL( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    impl_checkURL_throw(url);
    return getDataDefinitionByConnection(connect(url,info));
}

// -----------------------------------------------------------------------------
void ADOS::ThrowException(ADOConnection* _pAdoCon,const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    ADOErrors *pErrors = NULL;
    _pAdoCon->get_Errors(&pErrors);
    if(!pErrors)
        return; // no error found

    pErrors->AddRef( );

    // alle aufgelaufenen Fehler auslesen und ausgeben
    sal_Int32 nLen;
    pErrors->get_Count(&nLen);
    if (nLen)
    {
        ::rtl::OUString sError;
        ::rtl::OUString aSQLState;
        SQLException aException;
        aException.ErrorCode = 1000;
        for (sal_Int32 i = nLen-1; i>=0; --i)
        {
            ADOError *pError = NULL;
            pErrors->get_Item(OLEVariant(i),&pError);
            WpADOError aErr(pError);
            OSL_ENSURE(pError,"No error in collection found! BAD!");
            if(pError)
            {
                if(i==nLen-1)
                    aException = SQLException(aErr.GetDescription(),_xInterface,aErr.GetSQLState(),aErr.GetNumber(),Any());
                else
                {
                    SQLException aTemp = SQLException(aErr.GetDescription(),
                        _xInterface,aErr.GetSQLState(),aErr.GetNumber(),makeAny(aException));
                    aTemp.NextException <<= aException;
                    aException = aTemp;
                }
            }
        }
        pErrors->Clear();
        pErrors->Release();
        throw aException;
    }
    pErrors->Release();
}


