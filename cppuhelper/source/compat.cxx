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

#include "sal/config.h"

#include <cstdlib>

#include "com/sun/star/loader/CannotActivateFactoryException.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "osl/module.h"
#include "sal/types.h"

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiComponentFactory;
        class XMultiServiceFactory;
    }
    namespace reflection { class XIdlClass; }
    namespace registry {
        class XRegistryKey;
        class XSimpleRegistry;
    }
    namespace uno {
        class XComponentContext;
        class XInterface;
    }
} } }
namespace rtl { class OUString; }

// Stubs for removed functionality, to be killed when we bump cppuhelper SONAME

namespace cppu {

SAL_DLLPUBLIC_EXPORT
css::uno::Reference< css::lang::XMultiComponentFactory > bootstrapInitialSF(
    rtl::OUString const &) SAL_THROW((com::sun::star::uno::Exception))
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference< css::uno::XComponentContext > SAL_CALL
bootstrap_InitialComponentContext(
    css::uno::Reference< css::registry::XSimpleRegistry > const &,
    rtl::OUString const &) SAL_THROW((css::uno::Exception))
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference< css::registry::XSimpleRegistry >
SAL_CALL createNestedRegistry(rtl::OUString const &) SAL_THROW(()) {
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference< css::lang::XMultiServiceFactory >
SAL_CALL createRegistryServiceFactory(
    rtl::OUString const &, rtl::OUString const &, sal_Bool,
    rtl::OUString const &) SAL_THROW((css::uno::Exception))
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference< css::registry::XSimpleRegistry >
SAL_CALL createSimpleRegistry(rtl::OUString const &) SAL_THROW(()) {
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::reflection::XIdlClass * SAL_CALL
createStandardClassWithSequence(
    css::uno::Reference< css::lang::XMultiServiceFactory > const &,
    rtl::OUString const &,
    css::uno::Reference< css::reflection::XIdlClass > const &,
    css::uno::Sequence< rtl::OUString > const &) SAL_THROW(())
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference<css::uno::XInterface> SAL_CALL
invokeStaticComponentFactory(
    oslGenericFunction, rtl::OUString const &,
    css::uno::Reference<css::lang::XMultiServiceFactory> const &,
    css::uno::Reference<css::registry::XRegistryKey> const &,
    rtl::OUString const &)
    SAL_THROW((css::loader::CannotActivateFactoryException))
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference<css::uno::XInterface> SAL_CALL
loadSharedLibComponentFactory(
    rtl::OUString const &, rtl::OUString const &, rtl::OUString const &,
    css::uno::Reference<css::lang::XMultiServiceFactory> const &,
    css::uno::Reference<css::registry::XRegistryKey> const &,
    rtl::OUString const &)
    SAL_THROW((css::loader::CannotActivateFactoryException))
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
