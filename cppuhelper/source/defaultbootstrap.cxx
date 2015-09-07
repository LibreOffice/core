/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <vector>

#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/component_context.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

using rtl::OUString;

#include "macro_expander.hxx"
#include "paths.hxx"
#include "servicemanager.hxx"
#include "typemanager.hxx"

namespace {

rtl::OUString getBootstrapVariable(
    rtl::Bootstrap const & bootstrap, rtl::OUString const & name)
{
    rtl::OUString v;
    if (!bootstrap.getFrom(name, v)) {
        throw css::uno::DeploymentException(
            "Cannot obtain " + name + " from uno ini");
    }
    return v;
}
}

css::uno::Reference< css::uno::XComponentContext >
cppu::defaultBootstrap_InitialComponentContext(rtl::OUString const & iniUri)
{
    rtl::Bootstrap bs(iniUri);
    if (bs.getHandle() == 0) {
        throw css::uno::DeploymentException(
            "Cannot open uno ini " + iniUri);
    }
    rtl::Reference< cppuhelper::ServiceManager > smgr(
        new cppuhelper::ServiceManager);
    smgr->init(getBootstrapVariable(bs, "UNO_SERVICES"));
    rtl::Reference< cppuhelper::TypeManager > tmgr(new cppuhelper::TypeManager);
    tmgr->init(getBootstrapVariable(bs, "UNO_TYPES"));
    cppu::ContextEntry_Init entry;
    std::vector< cppu::ContextEntry_Init > context_values;
    context_values.push_back(
        cppu::ContextEntry_Init(
            "/singletons/com.sun.star.lang.theServiceManager",
            css::uno::makeAny(
                css::uno::Reference< css::uno::XInterface >(
                    static_cast< cppu::OWeakObject * >(smgr.get()))),
            false));
    context_values.push_back(
        cppu::ContextEntry_Init(
            "/singletons/com.sun.star.reflection.theTypeDescriptionManager",
            css::uno::makeAny(
                css::uno::Reference< css::uno::XInterface >(
                    static_cast< cppu::OWeakObject * >(tmgr.get()))),
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
    cppu::installTypeDescriptionManager(tmgr.get());
    return context;
}

css::uno::Reference< css::uno::XComponentContext >
cppu::defaultBootstrap_InitialComponentContext()
{
    return defaultBootstrap_InitialComponentContext(getUnoIniUri());
}

void
cppu::preInitBootstrap(css::uno::Reference< css::uno::XComponentContext > const & xContext)
{
    if (!xContext.is())
        throw css::uno::DeploymentException("preInit: XComponentContext is not created");

    css::uno::Reference< css::uno::XInterface > xService;
    xContext->getValueByName("/singletons/com.sun.star.lang.theServiceManager") >>= xService;
    if (!xService.is())
        throw css::uno::DeploymentException("preInit: XMultiComponentFactory is not created");

    rtl::Reference<cppuhelper::ServiceManager> aService(reinterpret_cast<cppuhelper::ServiceManager*>(xService.get()));

    // pre-requisites:
    // In order to load implementations and invoke
    // component factory it is required:
    // 1) defaultBootstrap_InitialComponentContext()
    // 2) comphelper::setProcessServiceFactory(xSFactory);
    // 3) InitVCL()
    aService->loadImplementations();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
