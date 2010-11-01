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

#include <set>

#include "boost/noncopyable.hpp"
#include "com/sun/star/configuration/XUpdate.hpp"
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
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/unload.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "broadcaster.hxx"
#include "components.hxx"
#include "lock.hxx"
#include "modifications.hxx"
#include "rootaccess.hxx"
#include "update.hxx"

namespace configmgr { namespace update {

namespace {

namespace css = com::sun::star;

std::set< rtl::OUString > seqToSet(
    css::uno::Sequence< rtl::OUString > const & sequence)
{
    return std::set< rtl::OUString >(
        sequence.getConstArray(),
        sequence.getConstArray() + sequence.getLength());
}

class Service:
    public cppu::WeakImplHelper1< css::configuration::XUpdate >,
    private boost::noncopyable
{
public:
    Service() {}

private:
    virtual ~Service() {}

    virtual void SAL_CALL insertExtensionXcsFile(
        sal_Bool shared, rtl::OUString const & fileUri)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL insertExtensionXcuFile(
        sal_Bool shared, rtl::OUString const & fileUri)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeExtensionXcuFile(rtl::OUString const & fileUri)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL insertModificationXcuFile(
        rtl::OUString const & fileUri,
        css::uno::Sequence< rtl::OUString > const & includedPaths,
        css::uno::Sequence< rtl::OUString > const & excludedPaths)
        throw (css::uno::RuntimeException);
};

void Service::insertExtensionXcsFile(
    sal_Bool shared, rtl::OUString const & fileUri)
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard g(lock);
    Components::getSingleton().insertExtensionXcsFile(shared, fileUri);
}

void Service::insertExtensionXcuFile(
    sal_Bool shared, rtl::OUString const & fileUri)
    throw (css::uno::RuntimeException)
{
    Broadcaster bc;
    {
        osl::MutexGuard g(lock);
        Modifications mods;
        Components::getSingleton().insertExtensionXcuFile(
            shared, fileUri, &mods);
        Components::getSingleton().initGlobalBroadcaster(
            mods, rtl::Reference< RootAccess >(), &bc);
    }
    bc.send();
}

void Service::removeExtensionXcuFile(rtl::OUString const & fileUri)
    throw (css::uno::RuntimeException)
{
    Broadcaster bc;
    {
        osl::MutexGuard g(lock);
        Modifications mods;
        Components::getSingleton().removeExtensionXcuFile(fileUri, &mods);
        Components::getSingleton().initGlobalBroadcaster(
            mods, rtl::Reference< RootAccess >(), &bc);
    }
    bc.send();
}

void Service::insertModificationXcuFile(
    rtl::OUString const & fileUri,
    css::uno::Sequence< rtl::OUString > const & includedPaths,
    css::uno::Sequence< rtl::OUString > const & excludedPaths)
    throw (css::uno::RuntimeException)
{
    Broadcaster bc;
    {
        osl::MutexGuard g(lock);
        Modifications mods;
        Components::getSingleton().insertModificationXcuFile(
            fileUri, seqToSet(includedPaths), seqToSet(excludedPaths), &mods);
        Components::getSingleton().initGlobalBroadcaster(
            mods, rtl::Reference< RootAccess >(), &bc);
    }
    bc.send();
}

class Factory:
    public cppu::WeakImplHelper1< css::lang::XSingleComponentFactory >,
    private boost::noncopyable
{
public:
    Factory() {}

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
    css::uno::Reference< css::uno::XComponentContext > const &)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    if (Arguments.getLength() != 0) {
        throw css::uno::Exception(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.comp.configuration.Update must be"
                    " instantiated without arguments")),
            static_cast< cppu::OWeakObject * >(this));
    }
    return static_cast< cppu::OWeakObject * >(new Service);
}

}

rtl::OUString getImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.configuration.Update"));
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    rtl::OUString name(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.Update_Service"));
    return css::uno::Sequence< rtl::OUString >(&name, 1);
}

css::uno::Reference< css::lang::XSingleComponentFactory > createFactory(
    cppu::ComponentFactoryFunc, rtl::OUString const &,
    css::uno::Sequence< rtl::OUString > const &, rtl_ModuleCount *)
    SAL_THROW(())
{
    return new Factory;
}

} }
