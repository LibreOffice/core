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
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "driver.hxx"

namespace css = com::sun::star;

namespace connectivity
{
namespace mork
{

class Service:
    public cppu::WeakImplHelper2< css::lang::XServiceInfo, css::sdbc::XDriver >,
    private boost::noncopyable
{
public:
    Service(css::uno::Reference< css::uno::XComponentContext > const context):
        context_(context)
    {
        assert(context.is());
    }

private:
    virtual ~Service() {}

    rtl::OUString SAL_CALL getImplementationName()
    throw (css::uno::RuntimeException)
    {
        return connectivity::mork::getImplementationName();
    }

    sal_Bool SAL_CALL supportsService(rtl::OUString const &ServiceName)
    throw (css::uno::RuntimeException)
    {
        return ServiceName == getSupportedServiceNames()[0];    //TODO
    }

    css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException)
    {
        return connectivity::mork::getSupportedServiceNames();
    }

    css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect(
        rtl::OUString const &url,
        css::uno::Sequence< css::beans::PropertyValue > const &info)
    throw (css::sdbc::SQLException, css::uno::RuntimeException);

    sal_Bool SAL_CALL acceptsURL(
        rtl::OUString const &url)
    throw (css::sdbc::SQLException, css::uno::RuntimeException);

    css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL
    getPropertyInfo(
        rtl::OUString const &url,
        css::uno::Sequence< css::beans::PropertyValue > const &info)
    throw (css::sdbc::SQLException, css::uno::RuntimeException);

    sal_Int32 SAL_CALL getMajorVersion()
    throw (css::uno::RuntimeException);

    sal_Int32 SAL_CALL getMinorVersion()
    throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
};

class Connection:
    public cppu::WeakImplHelper1< css::sdbc::XConnection >,
    private boost::noncopyable
{
public:
    Connection(
        css::uno::Reference< css::uno::XComponentContext > const context);

private:
    ~Connection();

    // XConnection
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const ::rtl::OUString &sql )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const ::rtl::OUString &sql )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    ::rtl::OUString SAL_CALL nativeSQL( const ::rtl::OUString &sql )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    void SAL_CALL setAutoCommit( sal_Bool autoCommit )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    sal_Bool SAL_CALL getAutoCommit(  )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    void SAL_CALL commit(  )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    void SAL_CALL rollback(  )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    sal_Bool SAL_CALL isClosed(  )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    void SAL_CALL setReadOnly( sal_Bool readOnly )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    sal_Bool SAL_CALL isReadOnly(  )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    void SAL_CALL setCatalog( const ::rtl::OUString &catalog )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    ::rtl::OUString SAL_CALL getCatalog(  )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    void SAL_CALL setTransactionIsolation( sal_Int32 level )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    sal_Int32 SAL_CALL getTransactionIsolation(  )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    void SAL_CALL setTypeMap( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap )
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // XCloseable
    void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);


    css::uno::Reference< css::uno::XComponentContext > context_;
};

}
}

connectivity::mork::Connection::Connection(
    css::uno::Reference< css::uno::XComponentContext > const context):
    context_(context)
{
    assert(context.is());
}

connectivity::mork::Connection::~Connection()
{
}

css::uno::Reference< css::sdbc::XConnection > connectivity::mork::Service::connect(
    rtl::OUString const &url,
    css::uno::Sequence< css::beans::PropertyValue > const &info)
throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    //... TODO
    (void) url;
    (void) info; // avoid warnings

    return new connectivity::mork::Connection(context_);
}

sal_Bool connectivity::mork::Service::acceptsURL(rtl::OUString const &url)
throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    //... TODO
    (void) url; // avoid warnings
    return false;
}

css::uno::Sequence< css::sdbc::DriverPropertyInfo > connectivity::mork::Service::getPropertyInfo(
    rtl::OUString const &url,
    css::uno::Sequence< css::beans::PropertyValue > const &info)
throw (css::sdbc::SQLException, css::uno::RuntimeException)
{
    //... TODO
    (void) url;
    (void) info; // avoid warnings
    return css::uno::Sequence< css::sdbc::DriverPropertyInfo >();
}

sal_Int32 connectivity::mork::Service::getMajorVersion() throw (css::uno::RuntimeException)
{
    //... TODO
    return 0;
}

sal_Int32 connectivity::mork::Service::getMinorVersion() throw (css::uno::RuntimeException)
{
    //... TODO
    return 0;
}

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const &context)
{
    return static_cast< cppu::OWeakObject * >(new connectivity::mork::Service(context));
}

rtl::OUString connectivity::mork::getImplementationName()
{
    return rtl::OUString("com.sun.star.comp.sdbc.MorkDriver");
}

css::uno::Sequence< rtl::OUString > connectivity::mork::getSupportedServiceNames()
{
    rtl::OUString name("com.sun.star.sdbc.Driver");
    return css::uno::Sequence< rtl::OUString >(&name, 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
