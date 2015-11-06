/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <cassert>
#include <memory>
#include <set>

#include <com/sun/star/configuration/XUpdate.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "broadcaster.hxx"
#include "components.hxx"
#include "lock.hxx"
#include "modifications.hxx"
#include "rootaccess.hxx"
#include "update.hxx"

namespace configmgr { namespace update {

namespace {

std::set< OUString > seqToSet(
    css::uno::Sequence< OUString > const & sequence)
{
    return std::set< OUString >(
        sequence.getConstArray(),
        sequence.getConstArray() + sequence.getLength());
}

class Service:
    public cppu::WeakImplHelper< css::configuration::XUpdate >
{
public:
    explicit Service(css::uno::Reference< css::uno::XComponentContext > const context):
        context_(context)
    {
        assert(context.is());
        lock_ = lock();
    }

private:
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;

    virtual ~Service() {}

    virtual void SAL_CALL insertExtensionXcsFile(
        sal_Bool shared, OUString const & fileUri)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL insertExtensionXcuFile(
        sal_Bool shared, OUString const & fileUri)
        throw (css::uno::RuntimeException,
               std::exception) override;

    virtual void SAL_CALL removeExtensionXcuFile(OUString const & fileUri)
        throw (css::uno::RuntimeException,
               std::exception) override;

    virtual void SAL_CALL insertModificationXcuFile(
        OUString const & fileUri,
        css::uno::Sequence< OUString > const & includedPaths,
        css::uno::Sequence< OUString > const & excludedPaths)
        throw (css::uno::RuntimeException, std::exception) override;

    std::shared_ptr<osl::Mutex> lock_;
    css::uno::Reference< css::uno::XComponentContext > context_;
};

void Service::insertExtensionXcsFile(
    sal_Bool shared, OUString const & fileUri)
    throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(*lock_);
    Components::getSingleton(context_).insertExtensionXcsFile(shared, fileUri);
}

void Service::insertExtensionXcuFile(
    sal_Bool shared, OUString const & fileUri)
    throw (css::uno::RuntimeException,
           std::exception)
{
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        Components & components = Components::getSingleton(context_);
        Modifications mods;
        components.insertExtensionXcuFile(shared, fileUri, &mods);
        components.initGlobalBroadcaster(
            mods, rtl::Reference< RootAccess >(), &bc);
    }
    bc.send();
}

void Service::removeExtensionXcuFile(OUString const & fileUri)
    throw (css::uno::RuntimeException, std::exception)
{
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        Components & components = Components::getSingleton(context_);
        Modifications mods;
        components.removeExtensionXcuFile(fileUri, &mods);
        components.initGlobalBroadcaster(
            mods, rtl::Reference< RootAccess >(), &bc);
    }
    bc.send();
}

void Service::insertModificationXcuFile(
    OUString const & fileUri,
    css::uno::Sequence< OUString > const & includedPaths,
    css::uno::Sequence< OUString > const & excludedPaths)
    throw (css::uno::RuntimeException, std::exception)
{
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        Components & components = Components::getSingleton(context_);
        Modifications mods;
        components.insertModificationXcuFile(
            fileUri, seqToSet(includedPaths), seqToSet(excludedPaths), &mods);
        components.initGlobalBroadcaster(
            mods, rtl::Reference< RootAccess >(), &bc);
    }
    bc.send();
}

}

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return static_cast< cppu::OWeakObject * >(new Service(context));
}

OUString getImplementationName() {
    return OUString("com.sun.star.comp.configuration.Update");
}

css::uno::Sequence< OUString > getSupportedServiceNames() {
    return css::uno::Sequence< OUString > { "com.sun.star.configuration.Update_Service" };
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
