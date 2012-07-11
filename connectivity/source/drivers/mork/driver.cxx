/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cassert>

#include "boost/noncopyable.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/sdbc/DriverPropertyInfo.hpp"
#include "com/sun/star/sdbc/SQLException.hpp"
#include "com/sun/star/sdbc/XConnection.hpp"
#include "com/sun/star/sdbc/XDriver.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "driver.hxx"

namespace connectivity { namespace mork {

namespace {

namespace css = com::sun::star;

class Service:
    public cppu::WeakImplHelper2< css::lang::XServiceInfo, css::sdbc::XDriver >,
    private boost::noncopyable
{
public:
    Service(css::uno::Reference< css::uno::XComponentContext > const context):
        context_(context)
    { assert(context.is()); }

private:
    virtual ~Service() {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    { return connectivity::mork::getImplementationName(); }

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    { return ServiceName == getSupportedServiceNames()[0]; } //TODO

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException)
    { return connectivity::mork::getSupportedServiceNames(); }

    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect(
        rtl::OUString const & url,
        css::uno::Sequence< css::beans::PropertyValue > const & info)
        throw (css::sdbc::SQLException, css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL acceptsURL(
        rtl::OUString const & url)
        throw (css::sdbc::SQLException, css::uno::RuntimeException);

    virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL
    getPropertyInfo(
        rtl::OUString const & url,
        css::uno::Sequence< css::beans::PropertyValue > const & info)
        throw (css::sdbc::SQLException, css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getMajorVersion()
        throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getMinorVersion()
        throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
};

class Connection:
    public cppu::WeakImplHelper1< css::sdbc::XConnection >,
    private boost::noncopyable
{
public:
    Connection(
        css::uno::Reference< css::uno::XComponentContext > const context):
        context_(context)
    { assert(context.is()); }

private:
    virtual ~Connection() {}

    //... TODO

    css::uno::Reference< css::uno::XComponentContext > context_;
};

css::uno::Reference< css::sdbc::XConnection > Service::connect(
    rtl::OUString const & url,
    css::uno::Sequence< css::beans::PropertyValue > const & info)
    throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    //... TODO
    (void) url; (void) info; // avoid warnings
    return static_cast< cppu::OWeakObject * >(new Connection(context_));
}

sal_Bool Service::acceptsURL(rtl::OUString const & url)
    throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    //... TODO
    (void) url; // avoid warnings
    return false;
}

css::uno::Sequence< css::sdbc::DriverPropertyInfo > Service::getPropertyInfo(
    rtl::OUString const & url,
    css::uno::Sequence< css::beans::PropertyValue > const & info)
    throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    //... TODO
    (void) url; (void) info; // avoid warnings
    return css::uno::Sequence< css::sdbc::DriverPropertyInfo >();
}

sal_Int32 Service::getMajorVersion() throw (css::uno::RuntimeException) {
    //... TODO
    return 0;
}

sal_Int32 Service::getMinorVersion() throw (css::uno::RuntimeException) {
    //... TODO
    return 0;
}

}

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return static_cast< cppu::OWeakObject * >(new Service(context));
}

rtl::OUString getImplementationName() {
    return rtl::OUString("com.sun.star.comp.sdbc.MorkDriver");
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    rtl::OUString name("com.sun.star.sdbc.Driver");
    return css::uno::Sequence< rtl::OUString >(&name, 1);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
