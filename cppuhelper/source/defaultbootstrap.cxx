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
 * [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *   (initial developer) ]
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

#include <cassert>
#include <vector>

#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/component_context.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

using rtl::OUString;

#include "macro_expander.hxx"
#include "paths.hxx"
#include "servicemanager.hxx"
#include "typedescriptionprovider.hxx"

namespace {

rtl::OUString getBootstrapVariable(
    rtl::Bootstrap const & bootstrap, rtl::OUString const & name)
{
    rtl::OUString v;
    if (!bootstrap.getFrom(name, v)) {
        throw css::uno::DeploymentException(
            "Cannot obtain " + name + " from uno ini",
            css::uno::Reference< css::uno::XInterface >());
    }
    return v;
}

}

css::uno::Reference< css::uno::XComponentContext >
cppu::defaultBootstrap_InitialComponentContext(rtl::OUString const & iniUri)
    SAL_THROW((css::uno::Exception))
{
    rtl::Bootstrap bs(iniUri);
    if (bs.getHandle() == 0) {
        throw css::uno::DeploymentException(
            "Cannot open uno ini " + iniUri,
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::Reference< cppuhelper::ServiceManager > smgr(
        new cppuhelper::ServiceManager(
            getBootstrapVariable(bs, "UNO_SERVICES")));
    cppu::ContextEntry_Init entry;
    std::vector< cppu::ContextEntry_Init > context_values;
    context_values.push_back(
        cppu::ContextEntry_Init(
            "/singletons/com.sun.star.lang.theServiceManager",
            css::uno::makeAny(
                css::uno::Reference< css::uno::XInterface >(
                    static_cast< cppu::OWeakObject * >(smgr.get()))),
            false));
    context_values.push_back( //TODO: from services.rdb?
        cppu::ContextEntry_Init(
            "/singletons/com.sun.star.util.theMacroExpander",
            css::uno::makeAny(
                cppuhelper::detail::create_bootstrap_macro_expander_factory()),
            true));
    smgr->addSingletonContextEntries(&context_values);
    context_values.push_back(
        cppu::ContextEntry_Init(
            "/services/com.sun.star.security.AccessController/mode",
            css::uno::makeAny(rtl::OUString("off")), false));
    context_values.push_back(
        cppu::ContextEntry_Init(
            "/singletons/com.sun.star.security.theAccessController",
            css::uno::makeAny(
                rtl::OUString("com.sun.star.security.AccessController")),
            true));
    assert(!context_values.empty());
    css::uno::Reference< css::uno::XComponentContext > context(
        createComponentContext(
            &context_values[0], context_values.size(),
            css::uno::Reference< css::uno::XComponentContext >()));
    smgr->setContext(context);
    css::uno::Reference< css::container::XHierarchicalNameAccess > tdmgr(
        context->getValueByName(
            "/singletons/com.sun.star.reflection.theTypeDescriptionManager"),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XSet >(
        tdmgr, css::uno::UNO_QUERY_THROW)->
        insert(
            css::uno::makeAny(
                cppuhelper::createTypeDescriptionProviders(
                    getBootstrapVariable(bs, "UNO_TYPES"), smgr.get(),
                    context)));
    cppu::installTypeDescriptionManager(tdmgr);
    return context;
}

css::uno::Reference< css::uno::XComponentContext >
cppu::defaultBootstrap_InitialComponentContext()
    SAL_THROW((css::uno::Exception))
{
    return defaultBootstrap_InitialComponentContext(getUnoIniUri());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
