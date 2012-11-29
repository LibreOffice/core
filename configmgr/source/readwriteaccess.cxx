/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 * (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "com/sun/star/configuration/XReadWriteAccess.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/NotInitializedException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/ChangesSet.hpp"
#include "cppuhelper/implbase3.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "components.hxx"
#include "lock.hxx"
#include "readwriteaccess.hxx"
#include "rootaccess.hxx"

namespace configmgr { namespace read_write_access {

namespace {

class Service:
    public cppu::WeakImplHelper3<
        css::lang::XServiceInfo, css::lang::XInitialization,
        css::configuration::XReadWriteAccess >,
    private boost::noncopyable
{
public:
    explicit Service(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        context_(context) {}

private:
    virtual ~Service() {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    { return read_write_access::getImplementationName(); }

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const &)
        throw (css::uno::RuntimeException)
    { return false; }

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException)
    { return read_write_access::getSupportedServiceNames(); }

    virtual void SAL_CALL initialize(
        css::uno::Sequence< css::uno::Any > const & aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL getByHierarchicalName(
        rtl::OUString const & aName)
        throw (
            css::container::NoSuchElementException, css::uno::RuntimeException)
    { return getRoot()->getByHierarchicalName(aName); }

    virtual sal_Bool SAL_CALL hasByHierarchicalName(rtl::OUString const & aName)
        throw (css::uno::RuntimeException)
    { return getRoot()->hasByHierarchicalName(aName); }

    virtual void SAL_CALL replaceByHierarchicalName(
        rtl::OUString const & aName, css::uno::Any const & aElement)
        throw (
            css::lang::IllegalArgumentException,
            css::container::NoSuchElementException,
            css::lang::WrappedTargetException, css::uno::RuntimeException)
    { getRoot()->replaceByHierarchicalName(aName, aElement); }

    virtual void SAL_CALL commitChanges()
        throw (css::lang::WrappedTargetException, css::uno::RuntimeException)
    { getRoot()->commitChanges(); }

    virtual sal_Bool SAL_CALL hasPendingChanges()
        throw (css::uno::RuntimeException)
    { return getRoot()->hasPendingChanges(); }

    virtual css::util::ChangesSet SAL_CALL getPendingChanges()
        throw (css::uno::RuntimeException)
    { return getRoot()->getPendingChanges(); }

    rtl::Reference< RootAccess > getRoot();

    css::uno::Reference< css::uno::XComponentContext > context_;

    osl::Mutex mutex_;
    rtl::Reference< RootAccess > root_;
};

void Service::initialize(css::uno::Sequence< css::uno::Any > const & aArguments)
    throw (css::uno::Exception, css::uno::RuntimeException)
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

rtl::OUString getImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.comp.configuration.ReadWriteAccess"));
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    rtl::OUString name(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.ReadWriteAccess"));
    return css::uno::Sequence< rtl::OUString >(&name, 1);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
