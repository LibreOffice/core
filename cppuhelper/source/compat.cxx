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

#include <cstdlib>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/module.h>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiComponentFactory;
        class XMultiServiceFactory;
        class XTypeProvider;
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

// Stubs for removed functionality, to be killed when we bump cppuhelper SONAME

namespace cppu {

SAL_DLLPUBLIC_EXPORT
css::uno::Reference< css::lang::XMultiComponentFactory > bootstrapInitialSF(
    OUString const &)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference< css::uno::XComponentContext > SAL_CALL
bootstrap_InitialComponentContext(
    css::uno::Reference< css::registry::XSimpleRegistry > const &,
    OUString const &)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference< css::registry::XSimpleRegistry >
SAL_CALL createNestedRegistry(OUString const &) {
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference< css::lang::XMultiServiceFactory >
SAL_CALL createRegistryServiceFactory(
    OUString const &, OUString const &, sal_Bool,
    OUString const &)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference< css::registry::XSimpleRegistry >
SAL_CALL createSimpleRegistry(OUString const &) {
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::reflection::XIdlClass * SAL_CALL
createStandardClassWithSequence(
    css::uno::Reference< css::lang::XMultiServiceFactory > const &,
    OUString const &,
    css::uno::Reference< css::reflection::XIdlClass > const &,
    css::uno::Sequence< OUString > const &)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference<css::uno::XInterface> SAL_CALL
invokeStaticComponentFactory(
    oslGenericFunction, OUString const &,
    css::uno::Reference<css::lang::XMultiServiceFactory> const &,
    css::uno::Reference<css::registry::XRegistryKey> const &,
    OUString const &)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT css::uno::Reference<css::uno::XInterface> SAL_CALL
loadSharedLibComponentFactory(
    OUString const &, OUString const &, OUString const &,
    css::uno::Reference<css::lang::XMultiServiceFactory> const &,
    css::uno::Reference<css::registry::XRegistryKey> const &,
    OUString const &)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

struct SAL_DLLPUBLIC_EXPORT ClassData {
    css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId();

    css::uno::Sequence<css::uno::Type> SAL_CALL getTypes();

    void SAL_CALL initTypeProvider();

    css::uno::Any SAL_CALL query(
        css::uno::Type const &, css::lang::XTypeProvider *);

    void SAL_CALL writeTypeOffset(css::uno::Type const &, sal_Int32);
};

css::uno::Sequence<sal_Int8> ClassData::getImplementationId() {
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

css::uno::Sequence<css::uno::Type> ClassData::getTypes() {
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

void ClassData::initTypeProvider() {
    std::abort();
}

css::uno::Any ClassData::query(
    css::uno::Type const &, css::lang::XTypeProvider *)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

void ClassData::writeTypeOffset(css::uno::Type const &, sal_Int32) {
    std::abort();
}

SAL_WNOUNREACHABLE_CODE_PUSH
struct SAL_DLLPUBLIC_EXPORT ClassDataBase {
    ClassDataBase();

    explicit ClassDataBase(sal_Int32);

    ~ClassDataBase();
};

ClassDataBase::ClassDataBase() {
    std::abort();
}

ClassDataBase::ClassDataBase(sal_Int32) {
    std::abort();
}

ClassDataBase::~ClassDataBase() {
    std::abort();
}
SAL_WNOUNREACHABLE_CODE_POP

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
