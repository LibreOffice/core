/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/configuration/XReadWriteAccess.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/ChangesSet.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "components.hxx"
#include "lock.hxx"
#include "readwriteaccess.hxx"
#include "rootaccess.hxx"

namespace configmgr { namespace read_write_access {

namespace {

class Service:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::lang::XInitialization,
        css::configuration::XReadWriteAccess >
{
public:
    explicit Service(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        context_(context) {}

private:
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;

    virtual ~Service() override {}

    virtual OUString SAL_CALL getImplementationName() override
    { return read_write_access::getImplementationName(); }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override
    { return read_write_access::getSupportedServiceNames(); }

    virtual void SAL_CALL initialize(
        css::uno::Sequence< css::uno::Any > const & aArguments) override;

    virtual css::uno::Any SAL_CALL getByHierarchicalName(
        OUString const & aName) override
    { return getRoot()->getByHierarchicalName(aName); }

    virtual sal_Bool SAL_CALL hasByHierarchicalName(OUString const & aName) override
    { return getRoot()->hasByHierarchicalName(aName); }

    virtual void SAL_CALL replaceByHierarchicalName(
        OUString const & aName, css::uno::Any const & aElement) override
    { getRoot()->replaceByHierarchicalName(aName, aElement); }

    virtual void SAL_CALL commitChanges() override
    { getRoot()->commitChanges(); }

    virtual sal_Bool SAL_CALL hasPendingChanges() override
    { return getRoot()->hasPendingChanges(); }

    virtual css::uno::Sequence< ::css::util::ElementChange > SAL_CALL getPendingChanges() override
    { return getRoot()->getPendingChanges(); }

    css::beans::Property SAL_CALL getPropertyByHierarchicalName(
        OUString const & aHierarchicalName)
        override
    { return getRoot()->getPropertyByHierarchicalName(aHierarchicalName); }

    sal_Bool SAL_CALL hasPropertyByHierarchicalName(
        OUString const & aHierarchicalName) override
    { return getRoot()->hasPropertyByHierarchicalName(aHierarchicalName); }

    rtl::Reference< RootAccess > getRoot();

    css::uno::Reference< css::uno::XComponentContext > context_;

    osl::Mutex mutex_;
    rtl::Reference< RootAccess > root_;
};

void Service::initialize(css::uno::Sequence< css::uno::Any > const & aArguments)
{
    OUString locale;
    if (aArguments.getLength() != 1 || !(aArguments[0] >>= locale)) {
        throw css::lang::IllegalArgumentException(
            "not exactly one string argument",
            static_cast< cppu::OWeakObject * >(this), -1);
    }
    osl::MutexGuard g1(mutex_);
    if (root_.is()) {
        throw css::uno::RuntimeException(
            "already initialized", static_cast< cppu::OWeakObject * >(this));
    }
    osl::MutexGuard g2(*lock());
    Components & components = Components::getSingleton(context_);
    root_ = new RootAccess(components, "/", locale, true);
    components.addRootAccess(root_);
}

rtl::Reference< RootAccess > Service::getRoot() {
    osl::MutexGuard g(mutex_);
    if (!root_.is()) {
        throw css::lang::NotInitializedException(
            "not initialized", static_cast< cppu::OWeakObject * >(this));
    }
    return root_;
}

}

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return static_cast< cppu::OWeakObject * >(new Service(context));
}

OUString getImplementationName() {
    return OUString("com.sun.star.comp.configuration.ReadWriteAccess");
}

css::uno::Sequence< OUString > getSupportedServiceNames() {
    return css::uno::Sequence< OUString > { "com.sun.star.configuration.ReadWriteAccess" };
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
