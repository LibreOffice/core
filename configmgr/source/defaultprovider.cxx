/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
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
*
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/weak.hxx"
#include "sal/types.h"
#include "rtl/unload.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "configurationprovider.hxx"
#include "lock.hxx"

namespace configmgr { namespace default_provider {

namespace {

namespace css = com::sun::star;

class Factory:
    public cppu::WeakImplHelper1< css::lang::XSingleComponentFactory >,
    private boost::noncopyable
{
public:
    Factory()
    {
        lock_ = lock();
    }

private:
    virtual ~Factory() {}

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithContext(
        css::uno::Reference< css::uno::XComponentContext > const & Context)
        throw (css::uno::Exception, css::uno::RuntimeException);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArgumentsAndContext(
        css::uno::Sequence< css::uno::Any > const & Arguments,
        css::uno::Reference< css::uno::XComponentContext > const & Context)
        throw (css::uno::Exception, css::uno::RuntimeException);

    boost::shared_ptr<osl::Mutex> lock_;
};

css::uno::Reference< css::uno::XInterface > Factory::createInstanceWithContext(
    css::uno::Reference< css::uno::XComponentContext > const & Context)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    return createInstanceWithArgumentsAndContext(
        css::uno::Sequence< css::uno::Any >(), Context);
}

css::uno::Reference< css::uno::XInterface >
Factory::createInstanceWithArgumentsAndContext(
    css::uno::Sequence< css::uno::Any > const & Arguments,
    css::uno::Reference< css::uno::XComponentContext > const & Context)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    if (Arguments.getLength() != 0) {
        throw css::uno::Exception(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.DefaultProvider must be"
                    " instantiated without arguments")),
            static_cast< cppu::OWeakObject * >(this));
    }
    osl::MutexGuard guard(*lock_);
    static css::uno::Reference< css::uno::XInterface > singleton(
        configuration_provider::createDefault(Context));
    return singleton;
}

}

rtl::OUString getImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.comp.configuration.DefaultProvider"));
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    rtl::OUString name(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.DefaultProvider"));
    return css::uno::Sequence< rtl::OUString >(&name, 1);
}

css::uno::Reference< css::lang::XSingleComponentFactory >
SAL_CALL createFactory(
    cppu::ComponentFactoryFunc, rtl::OUString const &,
    css::uno::Sequence< rtl::OUString > const &, rtl_ModuleCount *)
    SAL_THROW(())
{
    return new Factory;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
