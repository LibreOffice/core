/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cassert>

#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "cppuhelper/shlib.hxx"
#include "osl/file.hxx"
#include "rtl/ustring.hxx"

#include "paths.hxx"
#include "servicefactory_detail.hxx"
#include "typedescriptionprovider.hxx"

namespace {

css::uno::Reference< css::registry::XSimpleRegistry > readTypeRdbFile(
    rtl::OUString const & uri, bool optional,
    css::uno::Reference< css::registry::XSimpleRegistry > const & lastRegistry,
    css::uno::Reference< css::lang::XSingleServiceFactory > const &
        simpleRegistryFactory,
    css::uno::Reference< css::lang::XSingleServiceFactory > const &
        nestedRegistryFactory)
{
    assert(simpleRegistryFactory.is() && nestedRegistryFactory.is());
    try {
        css::uno::Reference< css::registry::XSimpleRegistry > simple(
            simpleRegistryFactory->createInstance(), css::uno::UNO_QUERY_THROW);
        simple->open(uri, true, false);
        if (lastRegistry.is()) {
            css::uno::Reference< css::registry::XSimpleRegistry > nested(
                nestedRegistryFactory->createInstance(),
                css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::uno::Any > args(2);
            args[0] <<= lastRegistry;
            args[1] <<= simple;
            css::uno::Reference< css::lang::XInitialization >(
                nested, css::uno::UNO_QUERY_THROW)->
                initialize(args);
            return nested;
        } else {
            return simple;
        }
    } catch (css::registry::InvalidRegistryException & e) {
        if (!optional) {
            throw css::uno::DeploymentException(
                "Invalid registry " + uri + ":" + e.Message,
                css::uno::Reference< css::uno::XInterface >());
        }
        SAL_INFO("cppuhelper", "Ignored optional " << uri);
        return lastRegistry;
    }
}

css::uno::Reference< css::registry::XSimpleRegistry > readTypeRdbDirectory(
    rtl::OUString const & uri, bool optional,
    css::uno::Reference< css::registry::XSimpleRegistry > const & lastRegistry,
    css::uno::Reference< css::lang::XSingleServiceFactory > const &
        simpleRegistryFactory,
    css::uno::Reference< css::lang::XSingleServiceFactory > const &
        nestedRegistryFactory)
{
    assert(simpleRegistryFactory.is() && nestedRegistryFactory.is());
    osl::Directory dir(uri);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << uri);
            return lastRegistry;
        }
        // fall through
    default:
        throw css::uno::DeploymentException(
            "Cannot open directory " + uri,
            css::uno::Reference< css::uno::XInterface >());
    }
    css::uno::Reference< css::registry::XSimpleRegistry > last(lastRegistry);
    for (;;) {
        rtl::OUString fileUri;
        if (!cppu::nextDirectoryItem(dir, &fileUri)) {
            break;
        }
        last = readTypeRdbFile(
            fileUri, optional, last, simpleRegistryFactory,
            nestedRegistryFactory);
    }
    return last;
}

}

css::uno::Reference< css::registry::XSimpleRegistry >
cppuhelper::createTypeRegistry(
    rtl::OUString const & uris, rtl::OUString const & libraryDirectoryUri)
{
    css::uno::Reference< css::lang::XMultiComponentFactory > factory(
        cppu::bootstrapInitialSF(libraryDirectoryUri));
    css::uno::Reference< css::lang::XSingleServiceFactory > simpleRegs(
        cppu::loadSharedLibComponentFactory(
            "bootstrap.uno" SAL_DLLEXTENSION, libraryDirectoryUri,
            "com.sun.star.comp.stoc.SimpleRegistry",
            css::uno::Reference< css::lang::XMultiServiceFactory >(
                factory, css::uno::UNO_QUERY_THROW),
            css::uno::Reference< css::registry::XRegistryKey >()),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::lang::XSingleServiceFactory > nestedRegs(
        cppu::loadSharedLibComponentFactory(
            "bootstrap.uno" SAL_DLLEXTENSION, libraryDirectoryUri,
            "com.sun.star.comp.stoc.NestedRegistry",
            css::uno::Reference< css::lang::XMultiServiceFactory >(
                factory, css::uno::UNO_QUERY_THROW),
            css::uno::Reference< css::registry::XRegistryKey >()),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::registry::XSimpleRegistry > reg;
    for (sal_Int32 i = 0; i != -1;) {
        rtl::OUString uri(uris.getToken(0, ' ', i));
        if (uri.isEmpty()) {
            continue;
        }
        bool optional;
        bool directory;
        cppu::decodeRdbUri(&uri, &optional, &directory);
        reg = directory
            ? readTypeRdbDirectory(uri, optional, reg, simpleRegs, nestedRegs)
            : readTypeRdbFile(uri, optional, reg, simpleRegs, nestedRegs);
    }
    return reg;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
