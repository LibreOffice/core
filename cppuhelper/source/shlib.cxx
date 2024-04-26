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
#include <string_view>

#ifdef IOS
#include <premac.h>
#include <Foundation/Foundation.h>
#include <postmac.h>
#endif

#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/registry/CannotRegisterImplementationException.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/shlib.hxx>
#include <o3tl/string_view.hxx>
#include <osl/module.hxx>
#include <sal/log.hxx>
#include <uno/environment.hxx>
#include <uno/mapping.hxx>

#include "loadsharedlibcomponentfactory.hxx"

#if defined DISABLE_DYNLOADING
#include <osl/detail/component-mapping.h>
#endif

css::uno::Environment cppuhelper::detail::getEnvironment(
    OUString const & name, std::u16string_view implementation)
{
    OUString n(name);
    if (!implementation.empty()) {
        static char const * log = std::getenv("UNO_ENV_LOG");
        if (log != nullptr && *log != 0) {
            OString imps(log);
            for (sal_Int32 i = 0; i != -1;) {
                std::string_view imp(o3tl::getToken(imps, 0, ';', i));
                //TODO: this assumes UNO_ENV_LOG only contains ASCII characters:
                if (o3tl::equalsAscii(implementation, imp))
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
    std::u16string_view implementation, OUString const & prefix)
{
    char const * name = nullptr;
    css::uno::Environment env;
    OUString fullPrefix(prefix);
    if (!fullPrefix.isEmpty()) {
        fullPrefix += "_";
    }
    component_getImplementationEnvironmentExtFunc fp1
        = reinterpret_cast<component_getImplementationEnvironmentExtFunc>(
            module.getFunctionSymbol(fullPrefix + COMPONENT_GETENVEXT));
    if (fp1 != nullptr) {
        (*fp1)(
            &name, reinterpret_cast<uno_Environment **>(&env),
            (OUStringToOString(implementation, RTL_TEXTENCODING_ASCII_US)
             .getStr()),
            target.get());
    } else {
        component_getImplementationEnvironmentFunc fp2
            = reinterpret_cast<component_getImplementationEnvironmentFunc>(
                module.getFunctionSymbol(fullPrefix + COMPONENT_GETENV));
        if (fp2 != nullptr) {
            (*fp2)(&name, reinterpret_cast<uno_Environment **>(&env));
        } else {
            name = CPPU_CURRENT_LANGUAGE_BINDING_NAME; //TODO: fail
        }
    }
    if (!env.is() && name != nullptr) {
        env = cppuhelper::detail::getEnvironment(
            OUString::createFromAscii(name), implementation);
    }
    return env;
}

#endif

extern "C" void getFactory(va_list * args) {
    component_getFactoryFunc fn = va_arg(*args, component_getFactoryFunc);
    OString const * implementation = va_arg(*args, OString const *);
    void * smgr = va_arg(*args, void *);
    void ** factory = va_arg(*args, void **);
    *factory = (*fn)(implementation->getStr(), smgr, nullptr);
}

css::uno::Reference<css::uno::XInterface> invokeComponentFactory(
    css::uno::Environment const & source, css::uno::Environment const & target,
    component_getFactoryFunc function, std::u16string_view uri,
    std::u16string_view implementation,
    css::uno::Reference<css::lang::XMultiServiceFactory> const & serviceManager)
{
    if (!(source.is() && target.is())) {
        throw css::loader::CannotActivateFactoryException(
            u"cannot get environments"_ustr,
            css::uno::Reference<css::uno::XInterface>());
    }
    OString impl(
        OUStringToOString(implementation, RTL_TEXTENCODING_ASCII_US));
    if (source.get() == target.get()) {
        return css::uno::Reference<css::uno::XInterface>(
            static_cast<css::uno::XInterface *>(
                (*function)(impl.getStr(), serviceManager.get(), nullptr)),
            SAL_NO_ACQUIRE);
    }
    css::uno::Mapping mapTo(source, target);
    css::uno::Mapping mapFrom(target, source);
    if (!(mapTo.is() && mapFrom.is())) {
        throw css::loader::CannotActivateFactoryException(
            u"cannot get mappings"_ustr,
            css::uno::Reference<css::uno::XInterface>());
    }
    void * smgr = mapTo.mapInterface(
        serviceManager.get(),
        cppu::UnoType<css::lang::XMultiServiceFactory>::get());
    void * factory = nullptr;
    target.invoke(getFactory, function, &impl, smgr, &factory);
    if (smgr != nullptr) {
        (*target.get()->pExtEnv->releaseInterface)(
            target.get()->pExtEnv, smgr);
    }
    if (factory == nullptr) {
        throw css::loader::CannotActivateFactoryException(
            (OUString::Concat("calling factory function for \"") + implementation + "\" in <"
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

#if !defined DISABLE_DYNLOADING

extern "C" void getInstance(va_list * args) {
    cppuhelper::ImplementationConstructorFn * fn = va_arg(*args, cppuhelper::ImplementationConstructorFn *);
    void * ctxt = va_arg(*args, void *);
    assert(ctxt);
    void * argseq = va_arg(*args, void *);
    assert(argseq);
    void ** instance = va_arg(*args, void **);
    assert(instance);
    assert(*instance == nullptr);
    *instance = (*fn)(static_cast<css::uno::XComponentContext*>(ctxt),
            *static_cast<css::uno::Sequence<css::uno::Any> const*>(argseq));
}

cppuhelper::WrapperConstructorFn mapConstructorFn(
    css::uno::Environment const & source, css::uno::Environment const & target,
    cppuhelper::ImplementationConstructorFn *const constructorFunction)
{
    if (!(source.is() && target.is())) {
        throw css::loader::CannotActivateFactoryException(
            u"cannot get environments"_ustr,
            css::uno::Reference<css::uno::XInterface>());
    }
    if (source.get() == target.get()) {
        return cppuhelper::WrapperConstructorFn(constructorFunction);
    }
    // note: it should be valid to capture these mappings because they are
    // ref-counted, and the returned closure will always be invoked in the
    // "source" environment
    css::uno::Mapping mapTo(source, target);
    css::uno::Mapping mapFrom(target, source);
    if (!(mapTo.is() && mapFrom.is())) {
        throw css::loader::CannotActivateFactoryException(
            u"cannot get mappings"_ustr,
            css::uno::Reference<css::uno::XInterface>());
    }
    return [mapFrom, mapTo, target, constructorFunction]
        (css::uno::XComponentContext *const context, css::uno::Sequence<css::uno::Any> const& args)
        {
            void *const ctxt = mapTo.mapInterface(
                context,
                cppu::UnoType<css::uno::XComponentContext>::get());
            if (args.hasElements()) {
                std::abort(); // TODO map args
            }
            void * instance = nullptr;
            target.invoke(getInstance, constructorFunction, ctxt, &args, &instance);
            if (ctxt != nullptr) {
                (*target.get()->pExtEnv->releaseInterface)(
                    target.get()->pExtEnv, ctxt);
            }
            css::uno::XInterface * res = nullptr;
            if (instance == nullptr) {
                return res;
            }
            mapFrom.mapInterface(
                reinterpret_cast<void **>(&res), instance,
                cppu::UnoType<css::uno::XInterface>::get());
            (*target.get()->pExtEnv->releaseInterface)(
                target.get()->pExtEnv, instance);
            return res;
        };
}

#endif

}

void cppuhelper::detail::loadSharedLibComponentFactory(
    OUString const & uri, OUString const & environment,
    OUString const & prefix, OUString const & implementation,
    OUString const & constructor,
    css::uno::Reference<css::lang::XMultiServiceFactory> const & serviceManager,
    WrapperConstructorFn * constructorFunction,
    css::uno::Reference<css::uno::XInterface> * factory)
{
    assert(constructor.isEmpty() || !environment.isEmpty());
    assert(
        (constructorFunction == nullptr && constructor.isEmpty())
        || !*constructorFunction);
    assert(factory != nullptr && !factory->is());
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
        SAL_INFO("cppuhelper.shlib", "prefix=" << prefix << " implementation=" << implementation << " uri=" << uri);
        lib_to_factory_mapping const * map = lo_get_factory_map();
        component_getFactoryFunc fp = 0;
        for (int i = 0; map[i].name != 0; ++i) {
            if (uri.equalsAscii(map[i].name)) {
                fp = map[i].component_getFactory_function;
                break;
            }
        }
        if (fp == 0) {
            SAL_WARN("cppuhelper", "unknown factory name \"" << uri << "\"");
#ifdef IOS
            NSLog(@"Unknown factory %s", uri.toUtf8().getStr());
#endif
            throw css::loader::CannotActivateFactoryException(
                "unknown factory name \"" + uri + "\"",
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
#ifdef IOS
            NSLog(@"Unknown constructor %s", constructor.toUtf8().getStr());
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
        OUString sym;
        SAL_INFO("cppuhelper.shlib", "prefix=" << prefix << " implementation=" << implementation << " uri=" << uri);
        if (!prefix.isEmpty()) {
            sym = prefix + "_" COMPONENT_GETFACTORY;
        } else {
            sym = COMPONENT_GETFACTORY;
        }
        oslGenericFunction fp = mod.getFunctionSymbol(sym);
        if (fp == nullptr) {
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
        if (fp == nullptr) {
            throw css::loader::CannotActivateFactoryException(
                ("no constructor symbol \"" + constructor
                 + "\" in component library <" + uri + ">"),
                css::uno::Reference<css::uno::XInterface>());
        }
        css::uno::Environment curEnv(css::uno::Environment::getCurrent());
        *constructorFunction = mapConstructorFn(
            curEnv,
            (environment.isEmpty()
             ? getEnvironmentFromModule(mod, curEnv, implementation, prefix)
             : getEnvironment(environment, implementation)),
            reinterpret_cast<ImplementationConstructorFn *>(fp));
    }
    mod.release();
#endif
}

css::uno::Reference<css::uno::XInterface> cppu::loadSharedLibComponentFactory(
    OUString const & uri, OUString const & rPath,
    OUString const & rImplName,
    css::uno::Reference<css::lang::XMultiServiceFactory> const & xMgr,
    css::uno::Reference<css::registry::XRegistryKey> const & xKey)
{
    assert(rPath.isEmpty()); (void) rPath;
    assert(!xKey.is()); (void) xKey;
    css::uno::Reference<css::uno::XInterface> fac;
    cppuhelper::detail::loadSharedLibComponentFactory(
        uri, u""_ustr, u""_ustr, rImplName, u""_ustr, xMgr, nullptr, &fac);
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
    OUString const & uri, OUString const & rPath,
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
    if (fp == nullptr) {
        throw css::registry::CannotRegisterImplementationException(
            ("no symbol \"" COMPONENT_WRITEINFO "\" in component library <"
             + uri + ">"),
            css::uno::Reference<css::uno::XInterface>());
    }
    css::uno::Environment curEnv(css::uno::Environment::getCurrent());
    css::uno::Environment env(getEnvironmentFromModule(mod, curEnv, u"", u""_ustr));
    if (!(curEnv.is() && env.is())) {
        throw css::registry::CannotRegisterImplementationException(
            u"cannot get environments"_ustr,
            css::uno::Reference<css::uno::XInterface>());
    }
    css::uno::Mapping map(curEnv, env);
    if (!map.is()) {
        throw css::registry::CannotRegisterImplementationException(
            u"cannot get mapping"_ustr, css::uno::Reference<css::uno::XInterface>());
    }
    void * smgr = map.mapInterface(
        xMgr.get(), cppu::UnoType<css::lang::XMultiServiceFactory>::get());
    void * key = map.mapInterface(
        xKey.get(), cppu::UnoType<css::registry::XRegistryKey>::get());
    sal_Bool ok;
    env.invoke(writeInfo, fp, smgr, key, &ok);
    (*env.get()->pExtEnv->releaseInterface)(env.get()->pExtEnv, key);
    if (smgr != nullptr) {
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
