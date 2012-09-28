/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
//    css::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xServiceFactory(;
    m_ProfileAccess = new ProfileAccess();
    assert(context.is());
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString MorkDriver::getImplementationName_Static(  ) throw(css::uno::RuntimeException)
{
    return rtl::OUString(MORK_DRIVER_IMPL_NAME);
}

//------------------------------------------------------------------------------
css::uno::Sequence< ::rtl::OUString > MorkDriver::getSupportedServiceNames_Static(  ) throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > aSNS(1);
    aSNS[0] = ::rtl::OUString( "com.sun.star.sdbc.Driver");
    return aSNS;
}

rtl::OUString SAL_CALL MorkDriver::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL MorkDriver::supportsService(const rtl::OUString& serviceName)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(serviceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

css::uno::Sequence< rtl::OUString > MorkDriver::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

css::uno::Reference< css::sdbc::XConnection > MorkDriver::connect(
    rtl::OUString const & url,
    css::uno::Sequence< css::beans::PropertyValue > const & info)
    throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> MorkDriver::connect()" );

    (void) url; (void) info; // avoid warnings
    css::uno::Reference< css::sdbc::XConnection > xCon;
    OConnection* pCon = new OConnection(this);
    xCon = pCon;    // important here because otherwise the connection could be deleted inside (refcount goes -> 0)
    pCon->construct(url, info);
    return xCon;
}

sal_Bool MorkDriver::acceptsURL(rtl::OUString const & url)
    throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> MorkDriver::acceptsURL()" );

    //... TODO
    (void) url; // avoid warnings
    return true;
}

css::uno::Sequence< css::sdbc::DriverPropertyInfo > MorkDriver::getPropertyInfo(
    rtl::OUString const & url,
    css::uno::Sequence< css::beans::PropertyValue > const & info)
    throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    //... TODO
    (void) url; (void) info; // avoid warnings
    return css::uno::Sequence< css::sdbc::DriverPropertyInfo >();
}

sal_Int32 MorkDriver::getMajorVersion() throw (css::uno::RuntimeException) {
    //... TODO
    return 0;
}

sal_Int32 MorkDriver::getMinorVersion() throw (css::uno::RuntimeException) {
    //... TODO
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
