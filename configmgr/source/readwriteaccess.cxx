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
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "components.hxx"
#include "lock.hxx"
#include "rootaccess.hxx"

namespace configmgr::read_write_access {

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
    { return "com.sun.star.comp.configuration.ReadWriteAccess"; }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override
    { return { "com.sun.star.configuration.ReadWriteAccess" }; }

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
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_configuration_ReadWriteAccess_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new configmgr::read_write_access::Service(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
