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


#include <rtl/bootstrap.hxx>

#include <uno/lbnames.h>
#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <compbase2.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

#include "macro_expander.hxx"
#include "paths.hxx"

constexpr OUString SERVICE_NAME_A = u"com.sun.star.lang.MacroExpander"_ustr;
constexpr OUString SERVICE_NAME_B = u"com.sun.star.lang.BootstrapMacroExpander"_ustr;
constexpr OUStringLiteral IMPL_NAME = u"com.sun.star.lang.comp.cppuhelper.BootstrapMacroExpander";

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using rtl::Bootstrap;

namespace cppu
{

static Bootstrap const & get_unorc()
{
    static rtlBootstrapHandle s_bstrap = rtl_bootstrap_args_open(getUnoIniUri().pData);
    return *reinterpret_cast<Bootstrap const *>(&s_bstrap);
}

}

namespace cppuhelper::detail {

OUString expandMacros(OUString const & text) {
    OUString t(text);
    rtl_bootstrap_expandMacros_from_handle(
        cppu::get_unorc().getHandle(), &t.pData);
    return t;
}

}

namespace
{

OUString s_impl_name()
{
    return IMPL_NAME;
}

Sequence< OUString > const & s_get_service_names()
{
    static const Sequence< OUString > IMPL_NAMES {
        SERVICE_NAME_A,
        SERVICE_NAME_B
    };
    return IMPL_NAMES;
}

typedef cppuhelper::WeakComponentImplHelper2<
    util::XMacroExpander, lang::XServiceInfo > t_uno_impl;

class Bootstrap_MacroExpander : public t_uno_impl
{
public:
    Bootstrap_MacroExpander()
        {}

    // XMacroExpander impl
    virtual OUString SAL_CALL expandMacros( OUString const & exp ) override;
    // XServiceInfo impl
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};


// XServiceInfo impl

OUString Bootstrap_MacroExpander::getImplementationName()
{
    return s_impl_name();
}

sal_Bool Bootstrap_MacroExpander::supportsService( OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

Sequence< OUString > Bootstrap_MacroExpander::getSupportedServiceNames()
{
    return s_get_service_names();
}

// XMacroExpander impl

OUString Bootstrap_MacroExpander::expandMacros( OUString const & exp )
{
    return cppuhelper::detail::expandMacros( exp );
}


Reference< XInterface > service_create(
    SAL_UNUSED_PARAMETER Reference< XComponentContext > const & )
{
    return static_cast< ::cppu::OWeakObject * >( new Bootstrap_MacroExpander );
}

}

namespace cppuhelper::detail {

Reference< lang::XSingleComponentFactory > create_bootstrap_macro_expander_factory()
{
    Reference< lang::XSingleComponentFactory > free(::cppu::createSingleComponentFactory(
                                                        service_create,
                                                        s_impl_name(),
                                                        s_get_service_names() ));

    uno::Environment curr_env(Environment::getCurrent());
    uno::Environment target_env(CPPU_CURRENT_LANGUAGE_BINDING_NAME);

    uno::Mapping target2curr(target_env, curr_env);

    return Reference<lang::XSingleComponentFactory>(
        static_cast<lang::XSingleComponentFactory *>(
            target2curr.mapInterface(free.get(), cppu::UnoType<decltype(free)>::get())),
        SAL_NO_ACQUIRE);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
