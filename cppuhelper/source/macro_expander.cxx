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
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include "macro_expander.hxx"
#include "paths.hxx"

#define SERVICE_NAME_A "com.sun.star.lang.MacroExpander"
#define SERVICE_NAME_B "com.sun.star.lang.BootstrapMacroExpander"
#define IMPL_NAME "com.sun.star.lang.comp.cppuhelper.BootstrapMacroExpander"

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using rtl::Bootstrap;
using rtl::OUString;

namespace cppu
{

Bootstrap const & get_unorc()
{
    static rtlBootstrapHandle s_bstrap = 0;
    if (! s_bstrap)
    {
        OUString iniName(getUnoIniUri());
        rtlBootstrapHandle bstrap = rtl_bootstrap_args_open( iniName.pData );

        ClearableMutexGuard guard( Mutex::getGlobalMutex() );
        if (s_bstrap)
        {
            guard.clear();
            rtl_bootstrap_args_close( bstrap );
        }
        else
        {
            s_bstrap = bstrap;
        }
    }
    return *reinterpret_cast<Bootstrap const *>(&s_bstrap);
}

}

namespace cppuhelper { namespace detail {

rtl::OUString expandMacros(rtl::OUString const & text) {
    rtl::OUString t(text);
    rtl_bootstrap_expandMacros_from_handle(
        cppu::get_unorc().getHandle(), &t.pData);
    return t;
}

} }

namespace
{

class ImplNames
{
private:
    Sequence<OUString> m_aNames;
public:
    ImplNames() : m_aNames(2)
    {
        m_aNames[0] = SERVICE_NAME_A;
        m_aNames[1] = SERVICE_NAME_B;
    }
    const Sequence<OUString>& getNames() const { return m_aNames; }
};

class theImplNames : public rtl::Static<ImplNames, theImplNames> {};

inline OUString s_impl_name()
{
    return OUString(IMPL_NAME);
}

inline Sequence< OUString > const & s_get_service_names()
{
    return theImplNames::get().getNames();
}

typedef ::cppu::WeakComponentImplHelper2<
    util::XMacroExpander, lang::XServiceInfo > t_uno_impl;

struct mutex_holder
{
    Mutex m_mutex;
};

class Bootstrap_MacroExpander : public mutex_holder, public t_uno_impl
{
protected:
    virtual void SAL_CALL disposing() override;

public:
    inline Bootstrap_MacroExpander()
        : t_uno_impl( m_mutex )
        {}
    virtual ~Bootstrap_MacroExpander();

    // XMacroExpander impl
    virtual OUString SAL_CALL expandMacros( OUString const & exp )
        throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override;
    // XServiceInfo impl
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException, std::exception) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException, std::exception) override;
};


void Bootstrap_MacroExpander::disposing()
{}

Bootstrap_MacroExpander::~Bootstrap_MacroExpander()
{}

// XServiceInfo impl

OUString Bootstrap_MacroExpander::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return s_impl_name();
}

sal_Bool Bootstrap_MacroExpander::supportsService( OUString const & serviceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, serviceName);
}

Sequence< OUString > Bootstrap_MacroExpander::getSupportedServiceNames()
    throw (RuntimeException, std::exception)
{
    return s_get_service_names();
}

// XMacroExpander impl

OUString Bootstrap_MacroExpander::expandMacros( OUString const & exp )
    throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    return cppuhelper::detail::expandMacros( exp );
}


Reference< XInterface > SAL_CALL service_create(
    SAL_UNUSED_PARAMETER Reference< XComponentContext > const & )
{
    return static_cast< ::cppu::OWeakObject * >( new Bootstrap_MacroExpander );
}

}

namespace cppuhelper { namespace detail {

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

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
