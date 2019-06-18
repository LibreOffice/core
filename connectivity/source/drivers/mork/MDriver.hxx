/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MDRIVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MDRIVER_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/DriverPropertyInfo.hpp>

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>

#define MORK_DRIVER_IMPL_NAME "com.sun.star.comp.sdbc.MorkDriver"

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
        class XInterface;
    }
} } }

namespace connectivity { namespace mork {
class ProfileAccess;

class MorkDriver:
    public cppu::WeakImplHelper< css::lang::XServiceInfo, css::sdbc::XDriver >
{
public:
    explicit MorkDriver(const css::uno::Reference< css::uno::XComponentContext >& context);

    const css::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& getFactory() const {return m_xFactory;}
    const OUString& getProfilePath() const {return m_sProfilePath;}
private:

    MorkDriver(const MorkDriver&) = delete;
    MorkDriver& operator=(const MorkDriver&) = delete;

    virtual ~MorkDriver() override {}

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect(
        OUString const & url,
        css::uno::Sequence< css::beans::PropertyValue > const & info) override;

    virtual sal_Bool SAL_CALL acceptsURL(
        OUString const & url) override;

    virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL
    getPropertyInfo(
        OUString const & url,
        css::uno::Sequence< css::beans::PropertyValue > const & info) override;

    virtual sal_Int32 SAL_CALL getMajorVersion() override;

    virtual sal_Int32 SAL_CALL getMinorVersion() override;

    css::uno::Reference< css::uno::XComponentContext > context_;
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
    OUString m_sProfilePath;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
