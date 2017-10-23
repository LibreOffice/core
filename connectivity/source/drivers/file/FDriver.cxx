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

#include <file/FDriver.hxx>
#include <file/FConnection.hxx>
#include <file/fcode.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>
#include <strings.hrc>
#include <resource/sharedresources.hxx>


using namespace connectivity::file;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;

OFileDriver::OFileDriver(const css::uno::Reference< css::uno::XComponentContext >& _rxContext)
    : ODriver_BASE(m_aMutex)
    ,m_xContext(_rxContext)
{
}

void OFileDriver::disposing()
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

OUString OFileDriver::getImplementationName_Static(  )
{
    return OUString("com.sun.star.sdbc.driver.file.Driver");
}

Sequence< OUString > OFileDriver::getSupportedServiceNames_Static(  )
{
    Sequence< OUString > aSNS( 2 );
    aSNS[0] = "com.sun.star.sdbc.Driver";
    aSNS[1] = "com.sun.star.sdbcx.Driver";
    return aSNS;
}


OUString SAL_CALL OFileDriver::getImplementationName(  )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL OFileDriver::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}


Sequence< OUString > SAL_CALL OFileDriver::getSupportedServiceNames(  )
{
    return getSupportedServiceNames_Static();
}


Reference< XConnection > SAL_CALL OFileDriver::connect( const OUString& url, const Sequence< PropertyValue >& info )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODriver_BASE::rBHelper.bDisposed);

    OConnection* pCon = new OConnection(this);
    Reference< XConnection > xCon = pCon;
    pCon->construct(url,info);
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}

sal_Bool SAL_CALL OFileDriver::acceptsURL( const OUString& url )
{
    return url.startsWith("sdbc:file:");
}

Sequence< DriverPropertyInfo > SAL_CALL OFileDriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& /*info*/ )
{
    if ( acceptsURL(url) )
    {
        std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< OUString > aBoolean(2);
        aBoolean[0] = "0";
        aBoolean[1] = "1";

        aDriverInfo.push_back(DriverPropertyInfo(
                "CharSet"
                ,"CharSet of the database."
                ,false
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "Extension"
                ,"Extension of the file format."
                ,false
                ,".*"
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "ShowDeleted"
                ,"Display inactive records."
                ,false
                ,"0"
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "EnableSQL92Check"
                ,"Use SQL92 naming constraints."
                ,false
                ,"0"
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "UseRelativePath"
                ,"Handle the connection url as relative path."
                ,false
                ,"0"
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "URL"
                ,"The URL of the database document which is used to create an absolute path."
                ,false
                ,OUString()
                ,Sequence< OUString >())
                );
        return Sequence< DriverPropertyInfo >(&(aDriverInfo[0]),aDriverInfo.size());
    } // if ( acceptsURL(url) )
    {
        ::connectivity::SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    } // if ( ! acceptsURL(url) )
    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL OFileDriver::getMajorVersion(  )
{
    return 1;
}

sal_Int32 SAL_CALL OFileDriver::getMinorVersion(  )
{
    return 0;
}


// XDataDefinitionSupplier
Reference< XTablesSupplier > SAL_CALL OFileDriver::getDataDefinitionByConnection( const Reference< css::sdbc::XConnection >& connection )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODriver_BASE::rBHelper.bDisposed);

    Reference< XTablesSupplier > xTab = nullptr;
    Reference< css::lang::XUnoTunnel> xTunnel(connection,UNO_QUERY);
    if(xTunnel.is())
    {
        OConnection* pSearchConnection = reinterpret_cast< OConnection* >( xTunnel->getSomething(OConnection::getUnoTunnelImplementationId()) );
        OConnection* pConnection = nullptr;
        for (OWeakRefArray::const_iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
        {
            if (static_cast<OConnection*>( Reference< XConnection >::query(i->get().get()).get() ) == pSearchConnection)
            {
                pConnection = pSearchConnection;
                break;
            }
        }

        if(pConnection)
            xTab = pConnection->createCatalog();
    }
    return xTab;
}


Reference< XTablesSupplier > SAL_CALL OFileDriver::getDataDefinitionByURL( const OUString& url, const Sequence< PropertyValue >& info )
{
    if ( ! acceptsURL(url) )
    {
        ::connectivity::SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    }
    return getDataDefinitionByConnection(connect(url,info));
}


OOperandAttr::OOperandAttr(sal_uInt16 _nPos,const Reference< XPropertySet>& _xColumn)
    : OOperandRow(_nPos,::comphelper::getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))))
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
