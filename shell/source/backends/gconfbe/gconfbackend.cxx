/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#include "precompiled_shell.hxx"
#include "sal/config.h"

#include <cstddef>

#include "boost/noncopyable.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XCurrentContext.hpp"
#include "cppu/unotype.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/current_context.hxx"
#include "uno/lbnames.h"

#include "gconfaccess.hxx"
#include "orbit.h"

namespace {

namespace css = com::sun::star;

rtl::OUString SAL_CALL getServiceImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.comp.configuration.backend.GconfBackend"));
}

css::uno::Sequence< rtl::OUString > SAL_CALL getServiceSupportedServiceNames() {
    rtl::OUString name(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.backend.GconfBackend"));
    return css::uno::Sequence< rtl::OUString >(&name, 1);
}

class Service:
    public cppu::WeakImplHelper2<
        css::lang::XServiceInfo, css::container::XNameAccess >,
    private boost::noncopyable
{
public:
    Service();

private:
    virtual ~Service() {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    { return getServiceImplementationName(); }

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    { return ServiceName == getSupportedServiceNames()[0]; }

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException)
    { return getServiceSupportedServiceNames(); }

    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException)
    { return cppu::UnoType< cppu::UnoVoidType >::get(); }

    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException)
    { return true; }

    virtual css::uno::Any SAL_CALL getByName(rtl::OUString const & aName)
        throw (
            css::container::NoSuchElementException,
            css::lang::WrappedTargetException, css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getElementNames()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasByName(rtl::OUString const & aName)
        throw (css::uno::RuntimeException);

    bool enabled_;
};

Service::Service(): enabled_(false) {
    css::uno::Reference< css::uno::XCurrentContext > context(
        css::uno::getCurrentContext());
    if (context.is()) {
        rtl::OUString desktop;
        context->getValueByName(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("system.desktop-environment"))) >>=
            desktop;
        enabled_ = desktop.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("GNOME")) &&
            ((orbit_major_version == 2 && orbit_minor_version >= 8) ||
             orbit_major_version > 2);
            // ORBit-2 versions < 2.8 cause a deadlock with the gtk+ VCL plugin
    }
}

css::uno::Any Service::getByName(rtl::OUString const & aName)
    throw (
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    for (std::size_t i = 0; i < gconfaccess::nConfigurationValues; ++i) {
        if (aName.equalsAscii(
                gconfaccess::ConfigurationValues[i].OOoConfItem))
        {
            return enabled_
                ? gconfaccess::getValue(gconfaccess::ConfigurationValues[i])
                : css::uno::makeAny(cppu::UnoType< cppu::UnoVoidType >::get());
        }
    }
    throw css::container::NoSuchElementException(
        aName, static_cast< cppu::OWeakObject * >(this));
}

css::uno::Sequence< rtl::OUString > Service::getElementNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > names(
        gconfaccess::nConfigurationValues);
    for (std::size_t i = 0; i < gconfaccess::nConfigurationValues; ++i) {
        names[i] = rtl::OUString::createFromAscii(
            gconfaccess::ConfigurationValues[i].OOoConfItem);
    }
    return names;
}

sal_Bool Service::hasByName(rtl::OUString const & aName)
    throw (css::uno::RuntimeException)
{
    for (std::size_t i = 0; i < gconfaccess::nConfigurationValues; ++i) {
        if (aName.equalsAscii(
                gconfaccess::ConfigurationValues[i].OOoConfItem))
        {
            return true;
        }
    }
    return false;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
    css::uno::Reference< css::uno::XComponentContext > const &)
{
    return static_cast< cppu::OWeakObject * >(new Service);
}

static cppu::ImplementationEntry const services[] = {
    { &createInstance, &getServiceImplementationName,
      &getServiceSupportedServiceNames, &cppu::createSingleComponentFactory, 0,
      0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(
    char const ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey)
{
    return component_writeInfoHelper(pServiceManager, pRegistryKey, services);
}
