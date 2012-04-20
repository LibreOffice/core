/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "sal/config.h"

#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/module.hxx"
#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "paths.hxx"

namespace {

namespace css = com::sun::star;

}

rtl::OUString cppu::get_this_libpath() {
    static rtl::OUString s_uri;
    if (s_uri.isEmpty()) {
        rtl::OUString uri;
        osl::Module::getUrlFromAddress(
            reinterpret_cast< oslGenericFunction >(get_this_libpath), uri);
        sal_Int32 i = uri.lastIndexOf('/');
        if (i == -1) {
            throw css::uno::DeploymentException(
                "URI " + uri + " is expected to contain a slash",
                css::uno::Reference< css::uno::XInterface >());
        }
        uri = uri.copy(0, i);
        osl::MutexGuard guard(osl::Mutex::getGlobalMutex());
        if (s_uri.isEmpty()) {
            s_uri = uri;
        }
    }
    return s_uri;
}

rtl::OUString cppu::getUnoIniUri() {
#if defined ANDROID
    // Wouldn't it be lovely to avoid this fugly hard-coding.
    // The problem is that the 'create_bootstrap_macro_expander_factory()'
    // required for bootstrapping services, calls cppu::get_unorc directly
    // instead of re-using the BoostrapHandle from:
    //     defaultBootstrap_InitialComponentContext
    // and since rtlBootstrapHandle is not ref-counted doing anything
    // clean here is hardish.
    rtl::OUString uri("file:///assets/program");
#else
    rtl::OUString uri(get_this_libpath());
#endif
    return uri + "/" SAL_CONFIGFILE("uno");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
