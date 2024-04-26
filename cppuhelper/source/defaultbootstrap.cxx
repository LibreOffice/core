/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <vector>

#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include "macro_expander.hxx"
#include "paths.hxx"
#include "servicemanager.hxx"
#include "typemanager.hxx"

namespace com :: sun :: star :: uno { class XComponentContext; }

namespace {

OUString getBootstrapVariable(
    rtl::Bootstrap const & bootstrap, OUString const & name)
{
    OUString v;
    if (!bootstrap.getFrom(name, v)) {
        throw css::uno::DeploymentException(
            "Cannot obtain " + name + " from uno ini");
    }
    return v;
}

}

css::uno::Reference< css::uno::XComponentContext >
cppu::defaultBootstrap_InitialComponentContext(OUString const & iniUri)
{
    rtl::Bootstrap bs(iniUri);
    if (bs.getHandle() == nullptr) {
        throw css::uno::DeploymentException(
            "Cannot open uno ini " + iniUri);
    }
    rtl::Reference smgr(
        new cppuhelper::ServiceManager);
    smgr->init(getBootstrapVariable(bs, u"UNO_SERVICES"_ustr));
    rtl::Reference tmgr(new cppuhelper::TypeManager);
    tmgr->init(getBootstrapVariable(bs, u"UNO_TYPES"_ustr));
    std::vector< cppu::ContextEntry_Init > context_values
    {
        cppu::ContextEntry_Init(
            u"/singletons/com.sun.star.lang.theServiceManager"_ustr,
            css::uno::Any(
                css::uno::Reference< css::uno::XInterface >(
                    static_cast< cppu::OWeakObject * >(smgr.get()))),
            false),
        cppu::ContextEntry_Init(
            u"/singletons/com.sun.star.reflection.theTypeDescriptionManager"_ustr,
            css::uno::Any(
                css::uno::Reference< css::uno::XInterface >(
                    static_cast< cppu::OWeakObject * >(tmgr.get()))),
            false),
        cppu::ContextEntry_Init( //TODO: from services.rdb?
            u"/singletons/com.sun.star.util.theMacroExpander"_ustr,
            css::uno::Any(
                cppuhelper::detail::create_bootstrap_macro_expander_factory()),
            true)
    };
    smgr->addSingletonContextEntries(&context_values);
    context_values.push_back(
        cppu::ContextEntry_Init(
            u"/services/com.sun.star.security.AccessController/mode"_ustr,
            css::uno::Any(u"off"_ustr), false));
    context_values.push_back(
        cppu::ContextEntry_Init(
            u"/singletons/com.sun.star.security.theAccessController"_ustr,
            css::uno::Any(
                u"com.sun.star.security.AccessController"_ustr),
            true));
    css::uno::Reference< css::uno::XComponentContext > context(
        createComponentContext(context_values.data(), context_values.size()));
    smgr->setContext(context);
    cppu::installTypeDescriptionManager(tmgr);
    return context;
}

css::uno::Reference< css::uno::XComponentContext >
cppu::defaultBootstrap_InitialComponentContext()
{
    return defaultBootstrap_InitialComponentContext(getUnoIniUri());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
