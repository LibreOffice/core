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
#include "cppuhelper/implbase2.hxx"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/configuration/XReadWriteAccess.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/ChangesSet.hpp"
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

namespace css = com::sun::star;

class Service:
    public cppu::WeakImplHelper2<
        css::lang::XServiceInfo, css::configuration::XReadWriteAccess >,
    private boost::noncopyable
{
public:
    Service(css::uno::Reference< css::uno::XComponentContext > const & context);

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

    virtual css::uno::Any SAL_CALL getByHierarchicalName(
        rtl::OUString const & aName)
        throw (
            css::container::NoSuchElementException, css::uno::RuntimeException)
    { return root_->getByHierarchicalName(aName); }

    virtual sal_Bool SAL_CALL hasByHierarchicalName(rtl::OUString const & aName)
        throw (css::uno::RuntimeException)
    { return root_->hasByHierarchicalName(aName); }

    virtual void SAL_CALL replaceByHierarchicalName(
        rtl::OUString const & aName, css::uno::Any const & aElement)
        throw (
            css::lang::IllegalArgumentException,
            css::container::NoSuchElementException,
            css::lang::WrappedTargetException, css::uno::RuntimeException)
    { root_->replaceByHierarchicalName(aName, aElement); }

    virtual void SAL_CALL commitChanges()
        throw (css::lang::WrappedTargetException, css::uno::RuntimeException)
    { root_->commitChanges(); }

    virtual sal_Bool SAL_CALL hasPendingChanges()
        throw (css::uno::RuntimeException)
    { return root_->hasPendingChanges(); }

    virtual css::util::ChangesSet SAL_CALL getPendingChanges()
        throw (css::uno::RuntimeException)
    { return root_->getPendingChanges(); }

    rtl::Reference< RootAccess > root_;
};

Service::Service(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    osl::MutexGuard guard(*lock());
    Components & components = Components::getSingleton(context);
    root_ = new RootAccess(
        components, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*")), true);
    components.addRootAccess(root_);
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
