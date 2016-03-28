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

#include "ado/ADriver.hxx"
#include "ado/AConnection.hxx"
#include "ado/Awrapadox.hxx"
#include "ado/ACatalog.hxx"
#include "ado/Awrapado.hxx"
#include "ado/adoimp.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "resource/ado_res.hrc"
#include <objbase.h>

#include "resource/sharedresources.hxx"

#include <memory>

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;


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

ODriver::~ODriver()
{
    CoUninitialize();
    CoInitialize(NULL);
}

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

OUString ODriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.sdbc.ado.ODriver");
}

Sequence< OUString > ODriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< OUString > aSNS( 2 );
    aSNS[0] = "com.sun.star.sdbc.Driver";
    aSNS[1] = "com.sun.star.sdbcx.Driver";
    return aSNS;
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL connectivity::ado::ODriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new ODriver(_rxFactory));
}


OUString SAL_CALL ODriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL ODriver::supportsService( const OUString& _rServiceName ) throw(RuntimeException)
{
    return cppu::supportsService(this, _rServiceName);
}


Sequence< OUString > SAL_CALL ODriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}


Reference< XConnection > SAL_CALL ODriver::connect( const OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
        return NULL;

    // we need to wrap the connection as the construct call might throw
    std::unique_ptr<OConnection> pCon(new OConnection(this));
    pCon->construct(url,info);
    OConnection* pPtr = pCon.get();
    Reference< XConnection > xCon = pCon.release();
    m_xConnections.push_back(WeakReferenceHelper(*pPtr));

    return xCon;
}

sal_Bool SAL_CALL ODriver::acceptsURL( const OUString& url )
        throw(SQLException, RuntimeException)
{
    return url.startsWith("sdbc:ado:");
}

void ODriver::impl_checkURL_throw(const OUString& _sUrl)
{
    if ( !acceptsURL(_sUrl) )
    {
        SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    } // if ( !acceptsURL(_sUrl) )
}

Sequence< DriverPropertyInfo > SAL_CALL ODriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
    impl_checkURL_throw(url);
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< OUString > aBooleanValues(2);
        aBooleanValues[0] = "false";
        aBooleanValues[1] = "true";

        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("IgnoreDriverPrivileges")
                ,OUString("Ignore the privileges from the database driver.")
                ,sal_False
                ,OUString( "false" )
                ,aBooleanValues)
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("EscapeDateTime")
                ,OUString("Escape date time format.")
                ,sal_False
                ,OUString( "true" )
                ,aBooleanValues)
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("TypeInfoSettings")
                ,OUString("Defines how the type info of the database metadata should be manipulated.")
                ,sal_False
                ,OUString( )
                ,Sequence< OUString > ())
        );
        return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
    }
    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL ODriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}

sal_Int32 SAL_CALL ODriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}

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

        for (OWeakRefArray::const_iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
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

Reference< XTablesSupplier > SAL_CALL ODriver::getDataDefinitionByURL( const OUString& url, const Sequence< PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    impl_checkURL_throw(url);
    return getDataDefinitionByConnection(connect(url,info));
}


void ADOS::ThrowException(ADOConnection* _pAdoCon,const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    ADOErrors *pErrors = NULL;
    _pAdoCon->get_Errors(&pErrors);
    if(!pErrors)
        return; // no error found

    pErrors->AddRef( );

    // read all noted errors and issue them
    sal_Int32 nLen;
    pErrors->get_Count(&nLen);
    if (nLen)
    {
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
