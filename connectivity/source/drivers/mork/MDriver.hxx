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

#include <sal/config.h>

#include <cassert>

#include <boost/noncopyable.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/DriverPropertyInfo.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#define MORK_DRIVER_IMPL_NAME "com.sun.star.comp.sdbc.MorkDriver"

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
        class XInterface;
    }
} } }

namespace connectivity { namespace mork {
class ProfileAccess;

css::uno::Reference< css::uno::XInterface > SAL_CALL
create(css::uno::Reference< css::uno::XComponentContext > const &);

class MorkDriver:
    public cppu::WeakImplHelper< css::lang::XServiceInfo, css::sdbc::XDriver >,
    private boost::noncopyable
{
public:
    explicit MorkDriver(css::uno::Reference< css::uno::XComponentContext > const context);
    static OUString getImplementationName_Static()
        throw(css::uno::RuntimeException);
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static()
        throw (css::uno::RuntimeException);

    css::uno::Reference< com::sun::star::lang::XMultiServiceFactory > getFactory(){return m_xFactory;}
    OUString getProfilePath() {return m_sProfilePath;}
private:

    virtual ~MorkDriver() {}

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect(
        OUString const & url,
        css::uno::Sequence< css::beans::PropertyValue > const & info)
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL acceptsURL(
        OUString const & url)
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL
    getPropertyInfo(
        OUString const & url,
        css::uno::Sequence< css::beans::PropertyValue > const & info)
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getMajorVersion()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getMinorVersion()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Reference< css::uno::XComponentContext > context_;
    css::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    OUString m_sProfilePath;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
