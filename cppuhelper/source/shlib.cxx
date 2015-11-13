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
#include <cstdlib>

#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/registry/CannotRegisterImplementationException.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/shlib.hxx>
#include <osl/module.hxx>
#include <sal/log.hxx>
#include <uno/environment.hxx>
#include <uno/mapping.hxx>

#include "loadsharedlibcomponentfactory.hxx"

#if defined DISABLE_DYNLOADING
#include <osl/detail/component-mapping.h>
#endif

css::uno::Environment cppuhelper::detail::getEnvironment(
    rtl::OUString const & name, rtl::OUString const & implementation)
{
    rtl::OUString n(name);
    if (!implementation.isEmpty()) {
        static char const * log = std::getenv("UNO_ENV_LOG");
        if (log != 0 && *log != 0) {
            rtl::OString imps(log);
            for (sal_Int32 i = 0; i != -1;) {
                rtl::OString imp(imps.getToken(0, ';', i));
                //TODO: this assumes UNO_ENV_LOG only contains ASCII characters:
                if (implementation.equalsAsciiL(imp.getStr(), imp.getLength()))
                {
                    n += ":log";
                    break;
                }
            }
        }
    }
    return css::uno::Environment(n);
}

namespace {

#if !defined DISABLE_DYNLOADING

css::uno::Environment getEnvironmentFromModule(
    osl::Module const & module, css::uno::Environment const & target,
    rtl::OUString const & implementation, rtl::OUString const & prefix)
{
    char const * name = 0;
    css::uno::Environment env;
    rtl::OUString fullPrefix(prefix);
    if (!fullPrefix.isEmpty()) {
        fullPrefix += "_";
    }
    component_getImplementationEnvironmentExtFunc fp1
        = reinterpret_cast<component_getImplementationEnvironmentExtFunc>(
            module.getFunctionSymbol(fullPrefix + COMPONENT_GETENVEXT));
    if (fp1 != 0) {
        (*fp1)(
            &name, reinterpret_cast<uno_Environment **>(&env),
            (rtl::OUStringToOString(implementation, RTL_TEXTENCODING_ASCII_US)
             .getStr()),
            target.get());
    } else {
        component_getImplementationEnvironmentFunc fp2
            = reinterpret_cast<component_getImplementationEnvironmentFunc>(
                module.getFunctionSymbol(fullPrefix + COMPONENT_GETENV));
        if (fp2 != 0) {
            (*fp2)(&name, reinterpret_cast<uno_Environment **>(&env));
        } else {
            name = CPPU_CURRENT_LANGUAGE_BINDING_NAME; //TODO: fail
        }
    }
    if (!env.is() && name != 0) {
        env = cppuhelper::detail::getEnvironment(
            rtl::OUString::createFromAscii(name), implementation);
    }
    return env;
}

#endif

extern "C" void getFactory(va_list * args) {
    component_getFactoryFunc fn = va_arg(*args, component_getFactoryFunc);
    rtl::OString const * implementation = va_arg(*args, rtl::OString const *);
    void * smgr = va_arg(*args, void *);
    void * key = va_arg(*args, void *);
    void ** factory = va_arg(*args, void **);
    *factory = (*fn)(implementation->getStr(), smgr, key);
}

css::uno::Reference<css::uno::XInterface> invokeComponentFactory(
    css::uno::Environment const & source, css::uno::Environment const & target,
    component_getFactoryFunc function, rtl::OUString const & uri,
    rtl::OUString const & implementation,
    css::uno::Reference<css::lang::XMultiServiceFactory> const & serviceManager)
{
    if (!(source.is() && target.is())) {
        throw css::loader::CannotActivateFactoryException(
            "cannot get environments",
            css::uno::Reference<css::uno::XInterface>());
    }
    rtl::OString impl(
        rtl::OUStringToOString(implementation, RTL_TEXTENCODING_ASCII_US));
    if (source.get() == target.get()) {
        return css::uno::Reference<css::uno::XInterface>(
            static_cast<css::uno::XInterface *>(
                (*function)(impl.getStr(), serviceManager.get(), 0)),
            SAL_NO_ACQUIRE);
    } else {
        css::uno::Mapping mapTo(source, target);
        css::uno::Mapping mapFrom(target, source);
        if (!(mapTo.is() && mapFrom.is())) {
            throw css::loader::CannotActivateFactoryException(
                "cannot get mappings",
                css::uno::Reference<css::uno::XInterface>());
        }
        void * smgr = mapTo.mapInterface(
            serviceManager.get(),
            cppu::UnoType<css::lang::XMultiServiceFactory>::get());
        void * factory = 0;
        target.invoke(getFactory, function, &impl, smgr, 0, &factory);
        if (smgr != 0) {
            (*target.get()->pExtEnv->releaseInterface)(
                target.get()->pExtEnv, smgr);
        }
        if (factory == 0) {
            throw css::loader::CannotActivateFactoryException(
                ("calling factory function for \"" + implementation + "\" in <"
                 + uri + "> returned null"),
                css::uno::Reference<css::uno::XInterface>());
        }
        css::uno::Reference<css::uno::XInterface> res;
        mapFrom.mapInterface(
            reinterpret_cast<void **>(&res), factory,
            cppu::UnoType<css::uno::XInterface>::get());
        (*target.get()->pExtEnv->releaseInterface)(
            target.get()->pExtEnv, factory);
        return res;
    }
}

}

void cppuhelper::detail::loadSharedLibComponentFactory(
    rtl::OUString const & uri, rtl::OUString const & environment,
    rtl::OUString const & prefix, rtl::OUString const & implementation,
    rtl::OUString const & constructor,
    css::uno::Reference<css::lang::XMultiServiceFactory> const & serviceManager,
    ImplementationConstructorFn ** constructorFunction,
    css::uno::Reference<css::uno::XInterface> * factory)
{
    assert(constructor.isEmpty() || !environment.isEmpty());
    assert(
        (constructorFunction == 0 && constructor.isEmpty())
        || *constructorFunction == 0);
    assert(factory != 0 && !factory->is());
#if defined DISABLE_DYNLOADING
    assert(!environment.isEmpty());
    if (constructor.isEmpty()) {
        css::uno::Environment curEnv(css::uno::Environment::getCurrent());
        css::uno::Environment env(getEnvironment(environment, implementation));
        if (!(curEnv.is() && env.is())) {
            throw css::loader::CannotActivateFactoryException(
                "cannot get environments",
                css::uno::Reference<css::uno::XInterface>());
        }
        if (curEnv.get() != env.get()) {
            std::abort();//TODO
        }
        rtl::OUString name(prefix == "direct" ? implementation : uri);
        SAL_INFO("cppuhelper.shlib", "prefix=" << prefix << " implementation=" << implementation << " uri=" << uri);
        lib_to_factory_mapping const * map = lo_get_factory_map();
        component_getFactoryFunc fp = 0;
        for (int i = 0; map[i].name != 0; ++i) {
            if (name.equalsAscii(map[i].name)) {
                fp = map[i].component_getFactory_function;
                break;
            }
        }
        if (fp == 0) {
            SAL_WARN("cppuhelper", "unknown factory name \"" << name << "\"");
#if defined IOS && !defined SAL_LOG_WARN
            // If the above SAL_WARN expanded to nothing, print to stderr...
            fprintf(stderr, "Unknown factory name %s\n", OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr());
#endif
            throw css::loader::CannotActivateFactoryException(
                "unknown factory name \"" + name + "\"",
                css::uno::Reference<css::uno::XInterface>());
        }
        *factory = invokeComponentFactory(
            css::uno::Environment::getCurrent(),
            getEnvironment(environment, implementation), fp, uri,
            implementation, serviceManager);
    } else {
        SAL_INFO("cppuhelper.shlib", "constructor=" << constructor);
        lib_to_constructor_mapping const * map = lo_get_constructor_map();
        for (int i = 0; map[i].name != 0; ++i) {
            if (constructor.equalsAscii(map[i].name)) {
                *constructorFunction
                    = reinterpret_cast<ImplementationConstructorFn *>(
                        map[i].constructor_function);
                return;
            }
        }
        SAL_WARN("cppuhelper", "unknown constructor name \"" << constructor << "\"");
#if defined IOS && !defined SAL_LOG_WARN
            // If the above SAL_WARN expanded to nothing, print to stderr...
            fprintf(stderr, "Unknown constructor name %s\n", OUStringToOString(constructor, RTL_TEXTENCODING_UTF8).getStr());
#endif
        throw css::loader::CannotActivateFactoryException(
            "unknown constructor name \"" + constructor + "\"",
            css::uno::Reference<css::uno::XInterface>());
    }
#else
    osl::Module mod(uri, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL);
    if (!mod.is()) {
        throw css::loader::CannotActivateFactoryException(
            "loading component library <" + uri + "> failed",
            css::uno::Reference<css::uno::XInterface>());
    }
    if (constructor.isEmpty()) {
        rtl::OUString sym;
        SAL_INFO("cppuhelper.shlib", "prefix=" << prefix << " implementation=" << implementation << " uri=" << uri);
        if (prefix == "direct") {
            sym = implementation.replace('.', '_') + "_" COMPONENT_GETFACTORY;
        } else if (!prefix.isEmpty()) {
            sym = prefix + "_" COMPONENT_GETFACTORY;
        } else {
            sym = COMPONENT_GETFACTORY;
        }
        oslGenericFunction fp = mod.getFunctionSymbol(sym);
        if (fp == 0) {
            throw css::loader::CannotActivateFactoryException(
                ("no factory symbol \"" + sym + "\" in component library <"
                 + uri + ">"),
                css::uno::Reference<css::uno::XInterface>());
        }
        css::uno::Environment curEnv(css::uno::Environment::getCurrent());
        *factory = invokeComponentFactory(
            curEnv,
            (environment.isEmpty()
             ? getEnvironmentFromModule(mod, curEnv, implementation, prefix)
             : getEnvironment(environment, implementation)),
            reinterpret_cast<component_getFactoryFunc>(fp), uri, implementation,
            serviceManager);
    } else {
        SAL_INFO("cppuhelper.shlib", "constructor=" << constructor);
        oslGenericFunction fp = mod.getFunctionSymbol(constructor);
        if (fp == 0) {
            throw css::loader::CannotActivateFactoryException(
                ("no constructor symbol \"" + constructor
                 + "\" in component library <" + uri + ">"),
                css::uno::Reference<css::uno::XInterface>());
        }
        *constructorFunction = reinterpret_cast<ImplementationConstructorFn *>(
            fp);
    }
    mod.release();
#endif
}

css::uno::Reference<css::uno::XInterface> cppu::loadSharedLibComponentFactory(
    rtl::OUString const & uri, rtl::OUString const & rPath,
    rtl::OUString const & rImplName,
    css::uno::Reference<css::lang::XMultiServiceFactory> const & xMgr,
    css::uno::Reference<css::registry::XRegistryKey> const & xKey)
{
    assert(rPath.isEmpty()); (void) rPath;
    assert(!xKey.is()); (void) xKey;
    css::uno::Reference<css::uno::XInterface> fac;
    cppuhelper::detail::loadSharedLibComponentFactory(
        uri, "", "", rImplName, "", xMgr, 0, &fac);
    return fac;
}

#if !defined DISABLE_DYNLOADING

namespace {

extern "C" void writeInfo(va_list * args) {
    component_writeInfoFunc fn = va_arg(*args, component_writeInfoFunc);
    void * smgr = va_arg(*args, void *);
    void * key = va_arg(*args, void *);
    sal_Bool * ok = va_arg(*args, sal_Bool *);
    *ok = (*fn)(smgr, key);
}

}

void cppu::writeSharedLibComponentInfo(
    rtl::OUString const & uri, rtl::OUString const & rPath,
    css::uno::Reference<css::lang::XMultiServiceFactory> const & xMgr,
    css::uno::Reference<css::registry::XRegistryKey> const & xKey)
{
    assert(rPath.isEmpty()); (void) rPath;
    osl::Module mod(uri, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL);
    if (!mod.is()) {
        throw css::registry::CannotRegisterImplementationException(
            "loading component library <" + uri + "> failed",
            css::uno::Reference<css::uno::XInterface>());
    }
    oslGenericFunction fp = mod.getFunctionSymbol(COMPONENT_WRITEINFO);
    if (fp == 0) {
        throw css::registry::CannotRegisterImplementationException(
            ("no symbol \"" COMPONENT_WRITEINFO "\" in component library <"
             + uri + ">"),
            css::uno::Reference<css::uno::XInterface>());
    }
    css::uno::Environment curEnv(css::uno::Environment::getCurrent());
    css::uno::Environment env(getEnvironmentFromModule(mod, curEnv, "", ""));
    if (!(curEnv.is() && env.is())) {
        throw css::registry::CannotRegisterImplementationException(
            "cannot get environments",
            css::uno::Reference<css::uno::XInterface>());
    }
    css::uno::Mapping map(curEnv, env);
    if (!map.is()) {
        throw css::registry::CannotRegisterImplementationException(
            "cannot get mapping", css::uno::Reference<css::uno::XInterface>());
    }
    void * smgr = map.mapInterface(
        xMgr.get(), cppu::UnoType<css::lang::XMultiServiceFactory>::get());
    void * key = map.mapInterface(
        xKey.get(), cppu::UnoType<css::registry::XRegistryKey>::get());
    sal_Bool ok;
    env.invoke(writeInfo, fp, smgr, key, &ok);
    (*env.get()->pExtEnv->releaseInterface)(env.get()->pExtEnv, key);
    if (smgr != 0) {
        (*env.get()->pExtEnv->releaseInterface)(env.get()->pExtEnv, smgr);
    }
    if (!ok) {
        throw css::registry::CannotRegisterImplementationException(
            ("calling \"" COMPONENT_WRITEINFO "\" in component library <" + uri
             + "> returned false"),
            css::uno::Reference<css::uno::XInterface>());
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
