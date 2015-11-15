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

#include "NDriver.hxx"
#include "NConnection.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbexception.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <ucbhelper/content.hxx>
#include <signal.h>
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"

using namespace osl;
using namespace connectivity::evoab;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;


OEvoabDriver::OEvoabDriver(const Reference< XMultiServiceFactory >& _rxFactory) :
        ODriver_BASE( m_aMutex ), m_xFactory( _rxFactory )
{
}

OEvoabDriver::~OEvoabDriver()
{
}

void OEvoabDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // when driver will be destroied so all our connections have to be destroied as well
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
        {
            try
            {
                xComp->dispose();
            }
            catch (const com::sun::star::lang::DisposedException&)
            {
                xComp.clear();
            }
        }
    }
    m_xConnections.clear();
    connectivity::OWeakRefArray().swap(m_xConnections); // this really clears

    ODriver_BASE::disposing();
}

// static ServiceInfo

OUString OEvoabDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return OUString(EVOAB_DRIVER_IMPL_NAME);
    // this name is referenced in the configuration and in the evoab.xml
    // Please take care when changing it.
}


Sequence< OUString > OEvoabDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence<OUString> aSNS { "com.sun.star.sdbc.Driver" };
    return aSNS;
}

OUString SAL_CALL OEvoabDriver::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL OEvoabDriver::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL OEvoabDriver::getSupportedServiceNames(  ) throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}


::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL connectivity::evoab::OEvoabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new OEvoabDriver(_rxFactory));
}

Reference< XConnection > SAL_CALL OEvoabDriver::connect( const OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if ( ! acceptsURL(url) )
        return nullptr;

    OEvoabConnection* pCon = new OEvoabConnection( *this );
    pCon->construct(url,info);
        Reference< XConnection > xCon = pCon;
        m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}

sal_Bool SAL_CALL OEvoabDriver::acceptsURL( const OUString& url )
    throw(SQLException, RuntimeException, std::exception)
{
    return acceptsURL_Stat(url);
}


Sequence< DriverPropertyInfo > SAL_CALL OEvoabDriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException, std::exception)
{
    if ( ! acceptsURL(url) )
    {
        ::connectivity::SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    } // if ( ! acceptsURL(url) )

    // if you have something special to say return it here :-)
    return Sequence< DriverPropertyInfo >();
}


sal_Int32 SAL_CALL OEvoabDriver::getMajorVersion(  ) throw(RuntimeException, std::exception)
{
    return 1;
}

sal_Int32 SAL_CALL OEvoabDriver::getMinorVersion(  ) throw(RuntimeException, std::exception)
{
    return 0;
}

bool OEvoabDriver::acceptsURL_Stat( const OUString& url )
{
    return ( url == "sdbc:address:evolution:local" || url == "sdbc:address:evolution:groupwise" || url == "sdbc:address:evolution:ldap" ) && EApiInit();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
