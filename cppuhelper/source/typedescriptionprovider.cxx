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
#include <vector>

#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/file.hxx"
#include "rtl/ustring.hxx"

#include "paths.hxx"
#include "typedescriptionprovider.hxx"

namespace {

void readTypeRdbFile(
    rtl::OUString const & uri, bool optional,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context,
    std::vector<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > *
            providers)
{
    assert(serviceManager.is());
    assert(providers != 0);
    css::uno::Reference< css::registry::XSimpleRegistry > reg(
        serviceManager->createInstanceWithContext(
            "com.sun.star.comp.stoc.SimpleRegistry", context),
        css::uno::UNO_QUERY_THROW);
    try {
        reg->open(uri, true, false);
    } catch (css::registry::InvalidRegistryException & e) {
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << uri);
            return;
        }
        throw css::uno::DeploymentException(
            "Invalid registry " + uri + ":" + e.Message,
            css::uno::Reference< css::uno::XInterface >());
    }
    css::uno::Sequence< css::uno::Any > arg(1);
    arg[0] <<= reg;
    providers->push_back(
        css::uno::Reference< css::container::XHierarchicalNameAccess >(
            serviceManager->createInstanceWithArgumentsAndContext(
                "com.sun.star.comp.stoc.RegistryTypeDescriptionProvider", arg,
                context),
            css::uno::UNO_QUERY_THROW));
}

void readTypeRdbDirectory(
    rtl::OUString const & uri, bool optional,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context,
    std::vector<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > *
            providers)
{
    osl::Directory dir(uri);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << uri);
            return;
        }
        // fall through
    default:
        throw css::uno::DeploymentException(
            "Cannot open directory " + uri,
            css::uno::Reference< css::uno::XInterface >());
    }
    for (;;) {
        rtl::OUString fileUri;
        if (!cppu::nextDirectoryItem(dir, &fileUri)) {
            break;
        }
        readTypeRdbFile(fileUri, optional, serviceManager, context, providers);
    }
}

}

css::uno::Sequence<
    css::uno::Reference< css::container::XHierarchicalNameAccess > >
cppuhelper::createTypeDescriptionProviders(
    rtl::OUString const & uris,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    std::vector<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > provs;
    for (sal_Int32 i = 0; i != -1;) {
        rtl::OUString uri(uris.getToken(0, ' ', i));
        if (uri.isEmpty()) {
            continue;
        }
        bool optional;
        bool directory;
        cppu::decodeRdbUri(&uri, &optional, &directory);
        if (directory) {
            readTypeRdbDirectory(
                uri, optional, serviceManager, context, &provs);
        } else {
            readTypeRdbFile(uri, optional, serviceManager, context, &provs);
        }
    }
    css::uno::Sequence<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > provs2(
            static_cast< sal_Int32 >(provs.size())); //TODO: check overflow
    std::vector<
        css::uno::Reference<
            css::container::XHierarchicalNameAccess > >::iterator i(
                provs.begin());
    for (sal_Int32 j = 0; j != provs2.getLength(); ++j) {
        provs2[j] = *i++;
    }
    return provs2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
