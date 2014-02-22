/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <cppuhelper/supportsservice.hxx>
#include "MDriver.hxx"
#include "MConnection.hxx"
#include "MNSProfileDiscover.hxx"

#include "resource/mork_res.hrc"
#include "resource/common_res.hrc"

using namespace connectivity::mork;

namespace connectivity
{
    namespace mork
    {
        css::uno::Reference< css::uno::XInterface > create(css::uno::Reference< css::uno::XComponentContext > const & context)
        {
            return static_cast< cppu::OWeakObject * >(new MorkDriver(context));
        }
    }
}

MorkDriver::MorkDriver(css::uno::Reference< css::uno::XComponentContext > const context):
    context_(context),
    m_xFactory(context_->getServiceManager(), css::uno::UNO_QUERY)
{
    SAL_INFO("connectivity.mork", "=> MorkDriver::MorkDriver()" );

    assert(context.is());
}



OUString MorkDriver::getImplementationName_Static(  ) throw(css::uno::RuntimeException)
{
    return OUString(MORK_DRIVER_IMPL_NAME);
}


css::uno::Sequence< OUString > MorkDriver::getSupportedServiceNames_Static(  ) throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > aSNS(1);
    aSNS[0] = "com.sun.star.sdbc.Driver";
    return aSNS;
}

OUString SAL_CALL MorkDriver::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL MorkDriver::supportsService(const OUString& serviceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > MorkDriver::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

css::uno::Reference< css::sdbc::XConnection > MorkDriver::connect(
    OUString const & url,
    css::uno::Sequence< css::beans::PropertyValue > const & info)
    throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> MorkDriver::connect()" );

    (void) url; (void) info; 
    css::uno::Reference< css::sdbc::XConnection > xCon;
    OConnection* pCon = new OConnection(this);
    xCon = pCon;    
    pCon->construct(url, info);
    return xCon;
}

sal_Bool MorkDriver::acceptsURL(OUString const & url)
    throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> MorkDriver::acceptsURL()" );
    
    //
    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    OUString aAddrbookURI(url.copy(nLen+1));
    
    nLen = aAddrbookURI.indexOf(':');
    OUString aAddrbookScheme;
    if ( nLen == -1 )
    {
        
        if ( !aAddrbookURI.isEmpty() )
        {
            aAddrbookScheme= aAddrbookURI;
        }
        else if( url == "sdbc:address:" )
        {
            return false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        aAddrbookScheme = aAddrbookURI.copy(0, nLen);
    }

    if (aAddrbookScheme.equalsAscii( "thunderbird" ) ||
        aAddrbookScheme.equalsAscii( "mozilla" ) )
    {
        return true;
    }

    return false;
}

css::uno::Sequence< css::sdbc::DriverPropertyInfo > MorkDriver::getPropertyInfo(
    OUString const & url,
    css::uno::Sequence< css::beans::PropertyValue > const & info)
    throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    
    (void) url; (void) info; 
    return css::uno::Sequence< css::sdbc::DriverPropertyInfo >();
}

sal_Int32 MorkDriver::getMajorVersion() throw (css::uno::RuntimeException) {
    
    return 0;
}

sal_Int32 MorkDriver::getMinorVersion() throw (css::uno::RuntimeException) {
    
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
